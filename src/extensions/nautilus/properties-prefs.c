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
#include <glib.h>

#include "properties.h"
#include "properties-prefs.h"
#include "../../hash/hash-func.h"

#define PREFS_SCHEMA "org.gtkhash.plugin"
#define PREFS_KEY_HASH_FUNCS "hash-functions"

static void gtkhash_properties_prefs_default(struct model_s *model)
{
	gtkhash_hash_func_enable_defaults(model->funcs);
}

void gtkhash_properties_prefs_load(struct model_s *model)
{
	GSettingsSchema *schema = g_settings_schema_source_lookup(
		g_settings_schema_source_get_default(), PREFS_SCHEMA, true);
	if (!schema) {
		g_warning("GSettings schema \"" PREFS_SCHEMA "\" not found");
		gtkhash_properties_prefs_default(model);
		return;
	}
	g_settings_schema_unref(schema);

	GSettings *settings = g_settings_new(PREFS_SCHEMA);
	char **strv = g_settings_get_strv(settings, PREFS_KEY_HASH_FUNCS);

	for (int i = 0; strv[i]; i++) {
		const enum hash_func_e id = gtkhash_hash_func_get_id_from_name(strv[i]);
		if (id != HASH_FUNC_INVALID && model->funcs[id].supported)
			model->funcs[id].enabled = true;
	}

	g_strfreev(strv);
	g_object_unref(settings);

	if (!gtkhash_hash_func_count_enabled(model->funcs))
		gtkhash_properties_prefs_default(model);
}
