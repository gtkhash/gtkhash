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

#include <gio/gio.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <libnemo-extension/nemo-property-page.h>
#include <libnemo-extension/nemo-property-page-provider.h>
#include <libnemo-extension/nemo-name-and-desc-provider.h>

#include "../common/module.h"
#include "../common/properties.h"

static GList *gtkhash_properties_get_pages(G_GNUC_UNUSED NemoPropertyPageProvider *provider,
	GList *files)
{
	// Only display page for a single file
	if (!files || files->next)
		return NULL;

	// Only display page for regular files
	if (nemo_file_info_get_file_type(files->data) != G_FILE_TYPE_REGULAR)
		return NULL;

	char *uri = nemo_file_info_get_uri(files->data);

	struct page_s *page = gtkhash_properties_new_page(uri);
	if (!page)
		return NULL;

	NemoPropertyPage *ppage = nemo_property_page_new("GtkHash::properties",
		gtk_label_new(_("Checksums")), page->box);

	return g_list_append(NULL, ppage);
}

static void gtkhash_properties_pp_iface_init(NemoPropertyPageProviderIface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_pages = gtkhash_properties_get_pages;
}

static GList *gtkhash_properties_get_name_desc(G_GNUC_UNUSED NemoNameAndDescProvider *provider)
{
	char *nd = g_strdup_printf("GtkHash:::%s", _("Calculate message digests or checksums"));
	return g_list_append(NULL, nd);
}

static void gtkhash_properties_nd_iface_init(NemoNameAndDescProviderIface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_name_and_desc = gtkhash_properties_get_name_desc;
}

G_MODULE_EXPORT void nemo_module_initialize(GTypeModule *module)
{
	const GType type = gtkhash_module_register_type(module, NEMO_TYPE_PROPERTY_PAGE_PROVIDER,
		(GInterfaceInitFunc)gtkhash_properties_pp_iface_init, sizeof(GObject));

	const GInterfaceInfo nd_iface_info = {
		(GInterfaceInitFunc)gtkhash_properties_nd_iface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, type, NEMO_TYPE_NAME_AND_DESC_PROVIDER, &nd_iface_info);
}

G_MODULE_EXPORT void nemo_module_shutdown(void)
{
}

G_MODULE_EXPORT void nemo_module_list_types(const GType **types, int *num_types)
{
	gtkhash_module_list_types(types, num_types);
}
