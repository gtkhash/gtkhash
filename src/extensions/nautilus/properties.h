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

#ifndef GTKHASH_EXTENSIONS_NAUTILUS_PROPERTIES_H
#define GTKHASH_EXTENSIONS_NAUTILUS_PROPERTIES_H

#include <glib.h>
#include <gio/gio.h>

#include "../common/module.h"
#include "../../hash/hash-file.h"

enum model_state_e {
	MODEL_STATE_NEW,
	MODEL_STATE_BUSY,
	MODEL_STATE_DONE,
};

struct model_s {
	GObject parent;
	GPtrArray *items;
	char *uri;
	struct hash_file_s *hfile;
	struct hash_func_s funcs[HASH_FUNCS_N];
	enum model_state_e state;
};

void gtkhash_properties_model_set_progress(struct model_s *model, const char *progress);
void gtkhash_properties_model_add_digest(struct model_s *model, const char *name,
	const char *digest);
void gtkhash_properties_register(GTypeModule *module);

#endif
