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

#if HAVE_NAUTILUS_EXTENSION_H
	#include <nautilus-extension.h>
#else
	#include <libnautilus-extension/nautilus-property-page.h>
	#include <libnautilus-extension/nautilus-property-page-provider.h>
#endif

#include "../common/properties.h"

static GList *gtkhash_properties_get_pages(G_GNUC_UNUSED NautilusPropertyPageProvider *provider,
	GList *files)
{
	// Only display page for a single file
	if (!files || files->next)
		return NULL;

	// Only display page for regular files
	if (nautilus_file_info_get_file_type(files->data) != G_FILE_TYPE_REGULAR)
		return NULL;

	char *uri = nautilus_file_info_get_uri(files->data);

	struct page_s *page = gtkhash_properties_new_page(uri);
	if (!page)
		return NULL;

	NautilusPropertyPage *ppage = nautilus_property_page_new("GtkHash::properties",
		gtk_label_new(_("Checksums")), page->box);

	GList *pages = g_list_append(NULL, ppage);

	return pages;
}

static void gtkhash_properties_pp_iface_init(NautilusPropertyPageProviderIface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_pages = gtkhash_properties_get_pages;
}

G_MODULE_EXPORT void nautilus_module_initialize(GTypeModule *module)
{
	gtkhash_properties_register_type(module, NAUTILUS_TYPE_PROPERTY_PAGE_PROVIDER,
		(GInterfaceInitFunc)gtkhash_properties_pp_iface_init);
}

G_MODULE_EXPORT void nautilus_module_shutdown(void)
{
}

G_MODULE_EXPORT void nautilus_module_list_types(const GType **types, int *num_types)
{
	gtkhash_properties_module_list_types(types, num_types);
}
