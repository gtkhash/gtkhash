/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
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
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "properties.h"
#include "properties-prefs.h"
#include "properties-hash.h"
#include "../hash/hash-func.h"

#define KEY_HASH_FUNCTIONS "/apps/gtkhash/hash_functions"
#define KEY_SHOW_DISABLED_HASH_FUNCTIONS "/apps/gtkhash/show_disabled_hash_functions"

static void gtkhash_properties_prefs_load_hash_functions(struct page_s *page,
	GConfClient *client)
{
	GSList *list = gconf_client_get_list(client, KEY_HASH_FUNCTIONS,
		GCONF_VALUE_STRING, NULL);
	if (!list)
		return;

	GSList *element = list;
	do {
		enum hash_func_e id = gtkhash_hash_func_get_id_from_name(element->data);
		if (id != HASH_FUNC_INVALID)
			page->hash_file.funcs[id].enabled = true;
		g_free(element->data);
	} while ((element = element->next));

	g_slist_free(list);
}

static void gtkhash_properties_prefs_load_show_disabled_hash_functions(struct page_s *page,
	GConfClient *client)
{
	// Default to enabled
	bool active = true;

	GError *error = NULL;
	if (!gconf_client_get_bool(client, KEY_SHOW_DISABLED_HASH_FUNCTIONS, &error)) {
		if (error)
			g_error_free(error);
		else
			active = false;
	}

	gtk_check_menu_item_set_active(page->menuitem_show_funcs, active);
}

void gtkhash_properties_prefs_load(struct page_s *page)
{
	GConfClient *client = gconf_client_get_default();

	gtkhash_properties_prefs_load_hash_functions(page, client);
	gtkhash_properties_prefs_load_show_disabled_hash_functions(page, client);

	g_object_unref(client);
}

static void gtkhash_properties_prefs_save_hash_functions(struct page_s *page,
	GConfClient *client)
{
	GSList *list = NULL;

	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (page->hash_file.funcs[i].enabled)
			list = g_slist_prepend(list, (void *)page->hash_file.funcs[i].name);

	if (list) {
		gconf_client_set_list(client, KEY_HASH_FUNCTIONS,
			GCONF_VALUE_STRING, list, NULL);
		g_slist_free(list);
	}
}

static void gtkhash_properties_prefs_save_show_disabled_hash_functions(struct page_s *page,
	GConfClient *client)
{
	bool active = gtk_check_menu_item_get_active(page->menuitem_show_funcs);

	gconf_client_set_bool(client, KEY_SHOW_DISABLED_HASH_FUNCTIONS, active, NULL);
}

void gtkhash_properties_prefs_save(struct page_s *page)
{
	GConfClient *client = gconf_client_get_default();

	gtkhash_properties_prefs_save_hash_functions(page, client);
	gtkhash_properties_prefs_save_show_disabled_hash_functions(page, client);

	g_object_unref(client);
}
