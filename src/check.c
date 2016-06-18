/*
 *   Copyright (C) 2007-2016 Tristan Heaven <tristan@tristanheaven.net>
 *
 *   This file is part of GtkHash.
 *
 *   GtkHash is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GtkHash is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GtkHash. If not, see <https://gnu.org/licenses/gpl-2.0.txt>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "check.h"
#include "main.h"
#include "hash.h"
#include "gui.h"
#include "list.h"

void check_file_save(const char * const filename)
{
	GString *string = g_string_sized_new(1024);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		switch (gui.view) {
			case GUI_VIEW_FILE: {
				const bool hmac_active = gtk_toggle_button_get_active(
					gui.togglebutton_hmac_file);
				const char *digest = gtk_entry_get_text(
					gui.hash_widgets[i].entry_file);
				if (digest && *digest) {
					g_string_append_printf(string,
						(hmac_active && hash.funcs[i].hmac_supported) ?
						"# HMAC-%s\n" : "# %s\n", hash.funcs[i].name);
				} else
					continue;
				char *path = gtk_file_chooser_get_filename(
					GTK_FILE_CHOOSER(gui.filechooserbutton));
				char *basename = g_path_get_basename(path);
				g_free(path);
				g_string_append_printf(string, "%s  %s\n",
				gtk_entry_get_text(gui.hash_widgets[i].entry_file),
					basename);
				g_free(basename);
				break;
			}
			case GUI_VIEW_TEXT: {
				const bool hmac_active = gtk_toggle_button_get_active(
					gui.togglebutton_hmac_text);
				g_string_append_printf(string,
					(hmac_active && hash.funcs[i].hmac_supported) ?
					"# HMAC-%s\n" : "# %s\n", hash.funcs[i].name);
				g_string_append_printf(string, "%s  \"%s\"\n",
					gtk_entry_get_text(gui.hash_widgets[i].entry_text),
					gtk_entry_get_text(gui.entry_text));
				break;
			}
			case GUI_VIEW_FILE_LIST: {
				int prev = -1;
				for (unsigned int row = 0; row < list.rows; row++)
				{
					char *digest = list_get_digest(row, i);
					if (digest && *digest) {
						if (i != prev)
							g_string_append_printf(string, "# %s\n",
								hash.funcs[i].name);
						prev = i;
					} else {
						if (digest)
							g_free(digest);
						prev = i;
						continue;
					}
					char *uri = list_get_uri(row);
					char *basename = g_filename_display_basename(uri);
					g_string_append_printf(string, "%s  %s\n",
						digest, basename);
					g_free(basename);
					g_free(uri);
					g_free(digest);
				}
				break;
			}
			default:
				g_assert_not_reached();
		}
	}

	char *data = g_string_free(string, false);
	g_file_set_contents(filename, data, -1, NULL);

	g_free(data);
}
