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

static void load_hash_funcs(GKeyFile *keyfile)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		GError *error = NULL;
		bool active = g_key_file_get_boolean(keyfile, "hash-funcs", hash.funcs[i].name, &error);

		if (!error) {
			hash.funcs[i].enabled = active;
			gtk_toggle_button_set_active(gui.hash_widgets[i].button, active);
		} else
			g_error_free(error); // Ignore the error
	}
}

static void load_view(GKeyFile *keyfile)
{
	GError *error = NULL;
	int view = g_key_file_get_integer(keyfile, "window", "view", &error);

	if (error) {
		g_error_free(error);
		return;
	}

	switch (view) {
		case VIEW_FILE:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_file), true);
			break;
		case VIEW_TEXT:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_text), true);
			break;
		case VIEW_FILE_LIST:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_file_list), true);
			break;
		default:
			g_assert_not_reached();
	}
}

static void load_window_size(GKeyFile *keyfile)
{
	GError *error = NULL;
	int width, height;

	width = g_key_file_get_integer(keyfile, "window", "width", &error);
	if (error) {
		g_error_free(error);
		error = NULL;
	}

	height = g_key_file_get_integer(keyfile, "window", "height", &error);
	if (error) {
		g_error_free(error);
		error = NULL;
	}

	prefs.width = width;
	prefs.height = height;
}

void prefs_load(void)
{
	GKeyFile *keyfile = g_key_file_new();
	char *filename = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);

	if (g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, NULL)) {
		load_hash_funcs(keyfile);
		load_view(keyfile);
		load_window_size(keyfile);
	}

	g_free(filename);
	g_key_file_free(keyfile);

	gui_update();
}

static void save_hash_funcs(GKeyFile *keyfile)
{
	for (int i = 0; i < HASH_FUNCS_N; i++)
		g_key_file_set_boolean(keyfile, "hash-funcs", hash.funcs[i].name,
			hash.funcs[i].enabled);
}

static void save_view(GKeyFile *keyfile)
{
	g_key_file_set_integer(keyfile, "window", "view", gui_get_view());
}

static void save_window_size(GKeyFile *keyfile)
{
	g_key_file_set_integer(keyfile, "window", "width", prefs.width);
	g_key_file_set_integer(keyfile, "window", "height", prefs.height);
}

void prefs_save(void)
{
	GKeyFile *keyfile = g_key_file_new();
	char *filename = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	char *data;

	save_hash_funcs(keyfile);
	save_view(keyfile);
	save_window_size(keyfile);

	data = g_key_file_to_data(keyfile, NULL, NULL);
	g_file_set_contents(filename, data, -1, NULL);

	g_free(data);
	g_free(filename);
	g_key_file_free(keyfile);
}
