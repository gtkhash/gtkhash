/*
 *   Copyright (C) 2007-2011 Tristan Heaven <tristanheaven@gmail.com>
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
 *   along with GtkHash. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <gtk/gtk.h>

#include "hash.h"
#include "main.h"
#include "gui.h"
#include "list.h"
#include "hash/hash-func.h"
#include "hash/hash-string.h"
#include "hash/hash-file.h"

static struct {
	struct hash_file_s file_data;
	GSList *uris;
} hash_priv = {
	.uris = NULL,
};

void gtkhash_hash_string_finish_cb(const enum hash_func_e id,
	const char *digest)
{
	gtk_entry_set_text(gui.hash_widgets[id].entry_text, digest);
}

void gtkhash_hash_file_report_cb(G_GNUC_UNUSED void *data, goffset file_size,
	goffset total_read, G_GNUC_UNUSED GTimer *timer)
{
	gtk_progress_bar_set_fraction(gui.progressbar,
		(double)total_read /
		(double)file_size);
}

void gtkhash_hash_file_finish_cb(G_GNUC_UNUSED void *data)
{
	const bool stop = gtkhash_hash_file_is_cancelled(&hash_priv.file_data);

	switch (gui_get_view()) {
		case GUI_VIEW_FILE: {
			if (stop)
				break;

			for (int i = 0; i < HASH_FUNCS_N; i++) {
				const char *digest = gtkhash_hash_func_get_digest(&hash.funcs[i],
					gui_get_digest_format());
				gtk_entry_set_text(gui.hash_widgets[i].entry_file, digest);
			}

			break;
		}
		case GUI_VIEW_FILE_LIST: {
			g_assert(hash_priv.uris);
			g_assert(hash_priv.uris->data);

			if (stop)
				break;

			for (int i = 0; i < HASH_FUNCS_N; i++) {
				const char *digest = gtkhash_hash_func_get_digest(&hash.funcs[i],
					gui_get_digest_format());
				list_set_digest(hash_priv.uris->data, i, digest);
			}

			g_free(hash_priv.uris->data);
			hash_priv.uris = g_slist_delete_link(hash_priv.uris, hash_priv.uris);
			if (hash_priv.uris) {
				hash_file_start(hash_priv.uris->data);
				return;
			}

			break;
		}
		default:
			g_assert_not_reached();
	}

	gui_set_state(GUI_STATE_IDLE);
	gui_check_digests();
}

void hash_file_start(const char *uri)
{
	if (gui_get_view() != GUI_VIEW_FILE_LIST)
		gtkhash_hash_file_clear_digests(&hash_priv.file_data);

	GFile *file = g_file_new_for_uri(uri);
	char *pname = g_file_get_parse_name(file);
	gtk_progress_bar_set_text(gui.progressbar, pname);
	g_free(pname);
	g_object_unref(file);

	gtkhash_hash_file_set_uri(&hash_priv.file_data, uri);
	gtkhash_hash_file_set_state(&hash_priv.file_data, HASH_FILE_STATE_START);
	gtkhash_hash_file_add_source(&hash_priv.file_data);
}

void hash_file_list_start(void)
{
	g_assert(!hash_priv.uris);

	gtkhash_hash_file_clear_digests(&hash_priv.file_data);

	hash_priv.uris = list_get_all_uris();
	g_assert(hash_priv.uris);

	hash_file_start(hash_priv.uris->data);
}

void hash_file_stop(void)
{
	gtkhash_hash_file_cancel(&hash_priv.file_data);

	while (gtkhash_hash_file_get_state(&hash_priv.file_data)
		!= HASH_FILE_STATE_IDLE)
	{
		gtk_main_iteration_do(false);
	}

	if (hash_priv.uris) {
		g_slist_free_full(hash_priv.uris, g_free);
		hash_priv.uris = NULL;
	}
}

void hash_init(void)
{
	gtkhash_hash_func_init_all(hash.funcs);
	gtkhash_hash_file_init(&hash_priv.file_data, hash.funcs, NULL);
}

void hash_deinit(void)
{
	gtkhash_hash_file_deinit(&hash_priv.file_data);
	gtkhash_hash_func_deinit_all(hash.funcs);

	if (hash_priv.uris) {
		g_slist_free_full(hash_priv.uris, g_free);
		hash_priv.uris = NULL;
	}
}
