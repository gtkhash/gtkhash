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

#include <glib.h>
#include <gmodule.h>
#include <nautilus-extension.h>

#include "../common/module.h"
#include "properties.h"

G_MODULE_EXPORT void nautilus_module_initialize(GTypeModule *module)
{
	gtkhash_properties_register(module);
}

G_MODULE_EXPORT void nautilus_module_shutdown(void)
{
}

G_MODULE_EXPORT void nautilus_module_list_types(const GType **types, int *num_types)
{
	gtkhash_module_list_types(types, num_types);
}
