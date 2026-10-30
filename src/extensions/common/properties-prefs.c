/*
 *   Copyright (C) 2007-2026 Tristan Heaven <tristan@tristanheaven.net>
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

#include <stdbool.h>
#include <gio/gio.h>

#include "properties.h"
#include "properties-prefs.h"
#include "../../hash/hash-func.h"

#define PREFS_SCHEMA "org.gtkhash.plugin"
#define PREFS_KEY_HASH_FUNCS "hash-functions"
#define PREFS_KEY_SHOW_DISABLED_FUNCS "show-disabled-hash-functions"
#define PREFS_BIND_FLAGS \
	(G_SETTINGS_BIND_DEFAULT | \
	 G_SETTINGS_BIND_GET_NO_CHANGES)

static void gtkhash_properties_prefs_default_hash_funcs(struct page_s *page)
{
	gtkhash_hash_func_enable_defaults(page->funcs);
}

static void gtkhash_properties_prefs_load_hash_funcs(struct page_s *page)
{
	char **names = g_settings_get_strv(page->settings, PREFS_KEY_HASH_FUNCS);
	gtkhash_hash_func_set_enabled_names(page->funcs, (const char * const *)names);
	g_strfreev(names);

	if (!gtkhash_hash_func_count_enabled(page->funcs))
		gtkhash_properties_prefs_default_hash_funcs(page);
}

static void gtkhash_properties_prefs_load_show_disabled_funcs(struct page_s *page)
{
	g_settings_bind(page->settings, PREFS_KEY_SHOW_DISABLED_FUNCS,
		page->menuitem_show_funcs, "active", PREFS_BIND_FLAGS);
}

static void gtkhash_properties_prefs_load(struct page_s *page)
{
	g_assert(page->settings);

	gtkhash_properties_prefs_load_hash_funcs(page);
	gtkhash_properties_prefs_load_show_disabled_funcs(page);
}

static void gtkhash_properties_prefs_save_hash_funcs(struct page_s *page)
{
	char **names = gtkhash_hash_func_get_enabled_names(page->funcs);
	g_settings_set_strv(page->settings, PREFS_KEY_HASH_FUNCS, (const char * const *)names);
	g_strfreev(names);
}

static void gtkhash_properties_prefs_save(struct page_s *page)
{
	g_assert(page->settings);

	gtkhash_properties_prefs_save_hash_funcs(page);
}

void gtkhash_properties_prefs_init(struct page_s *page)
{
	page->settings = NULL;

	GSettingsSchema *schema = g_settings_schema_source_lookup(
		g_settings_schema_source_get_default(), PREFS_SCHEMA, true);

	if (schema) {
		g_settings_schema_unref(schema);
		page->settings = g_settings_new(PREFS_SCHEMA);
		gtkhash_properties_prefs_load(page);
	} else {
		g_warning("GSettings schema \"" PREFS_SCHEMA "\" not found");
		gtkhash_properties_prefs_default_hash_funcs(page);
	}
}

void gtkhash_properties_prefs_deinit(struct page_s *page)
{
	if (!page->settings)
		return;

	gtkhash_properties_prefs_save(page);

	g_object_unref(page->settings);
	page->settings = NULL;
}
