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
#include <stdbool.h>
#include <gtk/gtk.h>

#include "prefs.h"
#include "main.h"
#include "hash.h"
#include "gui.h"
#include "hash/digest-format.h"

static void default_hash_funcs(void)
{
	bool has_enabled = false;

	// Try to enable default functions
	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (HASH_FUNC_IS_DEFAULT(i) && hash.funcs[i].supported) {
			gtk_toggle_button_set_active(gui.hash_widgets[i].button, true);
			has_enabled = true;
		}

	if (has_enabled)
		return;

	// Try to enable any supported function
	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (hash.funcs[i].supported) {
			gtk_toggle_button_set_active(gui.hash_widgets[i].button, true);
			return;
		}

	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
		_("Failed to enable any supported hash functions."));
	gtk_window_set_title(GTK_WINDOW(dialog), PACKAGE_NAME);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	exit(EXIT_FAILURE);
}

static void default_window_show_toolbar(void)
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
		gui.menuitem_treeview_show_toolbar), true);
}

static void load_hash_funcs(GKeyFile *keyfile)
{
	bool has_enabled = false;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		GError *error = NULL;
		bool active = g_key_file_get_boolean(keyfile, "hash-funcs",
			hash.funcs[i].name, &error);

		if (!error) {
			if (hash.funcs[i].supported) {
				if (active)
					has_enabled = true;
				hash.funcs[i].enabled = active;
				gtk_toggle_button_set_active(gui.hash_widgets[i].button,
					active);
			} else
				hash.funcs[i].enabled = false;
		} else
			g_error_free(error);
	}

	if (!has_enabled)
		default_hash_funcs();
}

static void load_digest_format(GKeyFile *keyfile)
{
	GError *error = NULL;
	int format = g_key_file_get_integer(keyfile, "digest", "format", &error);

	if (error) {
		g_error_free(error);
		return;
	}

	if (DIGEST_FORMAT_IS_VALID(format))
		gui_set_digest_format(format);
}

static void load_window_view(GKeyFile *keyfile)
{
	GError *error = NULL;
	int view = g_key_file_get_integer(keyfile, "window", "view", &error);

	if (error) {
		g_error_free(error);
		return;
	}

	if (GUI_VIEW_IS_VALID(view))
		gui_set_view((enum gui_view_e)view);
}

static void load_window_show_toolbar(GKeyFile *keyfile)
{
	GError *error = NULL;
	bool show_toolbar = g_key_file_get_boolean(keyfile, "window",
		"show-toolbar", &error);

	if (error) {
		g_error_free(error);
		show_toolbar = true;
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
		gui.menuitem_treeview_show_toolbar), show_toolbar);
}

static void load_window_size(GKeyFile *keyfile)
{
	GError *error = NULL;

	bool max = g_key_file_get_boolean(keyfile, "window", "max", &error);
	if (error)
		g_error_free(error);
	else if (max) {
		gtk_window_maximize(gui.window);
		return;
	}

	int width = g_key_file_get_integer(keyfile, "window", "width", &error);
	if (error) {
		g_error_free(error);
		error = NULL;
		width = -1;
	}

	int height = g_key_file_get_integer(keyfile, "window", "height", &error);
	if (error) {
		g_error_free(error);
		error = NULL;
		height = -1;
	}

	if ((width > 0) && (height > 0))
		gtk_window_resize(gui.window, width, height);
}

void prefs_load(void)
{
	GKeyFile *keyfile = g_key_file_new();
	char *filename = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	bool loaded;

	if ((loaded = g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE,
		NULL)))
	{
		load_hash_funcs(keyfile);
		load_digest_format(keyfile);
		load_window_view(keyfile);
		load_window_show_toolbar(keyfile);
		load_window_size(keyfile);
	}

	g_free(filename);
	g_key_file_free(keyfile);

	if (!loaded) {
		default_hash_funcs();
		default_window_show_toolbar();
	}

	gui_update();
}

static void save_hash_funcs(GKeyFile *keyfile)
{
	for (int i = 0; i < HASH_FUNCS_N; i++)
		g_key_file_set_boolean(keyfile, "hash-funcs", hash.funcs[i].name,
			hash.funcs[i].enabled);
}

static void save_digest_format(GKeyFile *keyfile)
{
	g_key_file_set_integer(keyfile, "digest", "format", gui_get_digest_format());
}

static void save_window_view(GKeyFile *keyfile)
{
	g_key_file_set_integer(keyfile, "window", "view", gui_get_view());
}

static void save_window_show_toolbar(GKeyFile *keyfile)
{
	const bool show_toolbar = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gui.menuitem_treeview_show_toolbar));

	g_key_file_set_boolean(keyfile, "window", "show-toolbar", show_toolbar);
}

static void save_window_size(GKeyFile *keyfile)
{
	bool max = gui_is_maximised();
	g_key_file_set_boolean(keyfile, "window", "max", max);

	if (!max) {
		int width, height;
		gtk_window_get_size(gui.window, &width, &height);
		g_key_file_set_integer(keyfile, "window", "width", width);
		g_key_file_set_integer(keyfile, "window", "height", height);
	}
}

void prefs_save(void)
{
	GKeyFile *keyfile = g_key_file_new();
	char *filename = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	char *data;

	save_hash_funcs(keyfile);
	save_digest_format(keyfile);
	save_window_view(keyfile);
	save_window_show_toolbar(keyfile);
	save_window_size(keyfile);

	data = g_key_file_to_data(keyfile, NULL, NULL);
	g_file_set_contents(filename, data, -1, NULL);

	g_free(data);
	g_free(filename);
	g_key_file_free(keyfile);
}
