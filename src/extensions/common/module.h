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

#ifndef GTKHASH_EXTENSIONS_COMMON_MODULE_H
#define GTKHASH_EXTENSIONS_COMMON_MODULE_H

#include <glib.h>
#include <glib-object.h>

#if ENABLE_NLS
	#include <glib/gi18n-lib.h>
#else
	#define _(STR) (STR)
#endif

GType gtkhash_module_get_type(void);
GType gtkhash_module_register_type(GTypeModule *module, const GType provider_type,
	GInterfaceInitFunc interface_init, const size_t instance_size);
void gtkhash_module_list_types(const GType **types, int *num_types);

#endif
