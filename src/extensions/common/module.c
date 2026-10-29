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
#include <glib-object.h>

#include "module.h"

static GType gtkhash_module_type;

GType gtkhash_module_get_type(void)
{
	return gtkhash_module_type;
}

GType gtkhash_module_register_type(GTypeModule *module, const GType provider_type,
	GInterfaceInitFunc interface_init, const size_t instance_size)
{
	const GTypeInfo info = {
		sizeof(GObjectClass),
		(GBaseInitFunc)NULL,
		(GBaseFinalizeFunc)NULL,
		(GClassInitFunc)NULL,
		(GClassFinalizeFunc)NULL,
		NULL,
		instance_size,
		0, // n_preallocs (ignored)
		(GInstanceInitFunc)NULL,
		(GTypeValueTable *)NULL
	};

	gtkhash_module_type = g_type_module_register_type(module, G_TYPE_OBJECT, PACKAGE_NAME, &info, 0);

	const GInterfaceInfo interface_info = {
		interface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, gtkhash_module_type, provider_type, &interface_info);

#if ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

	return gtkhash_module_type;
}

void gtkhash_module_list_types(const GType **types, int *num_types)
{
	static GType gtkhash_module_type_list[1];

	gtkhash_module_type_list[0] = gtkhash_module_type;
	*types = gtkhash_module_type_list;
	*num_types = G_N_ELEMENTS(gtkhash_module_type_list);
}
