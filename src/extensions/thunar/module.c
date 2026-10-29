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

#undef GTK_DISABLE_DEPRECATED // thunarx-3 doesn't build with this
#include <thunarx/thunarx.h>

#include "../common/module.h"
#include "../common/properties.h"

static GList *gtkhash_properties_get_pages(G_GNUC_UNUSED ThunarxPropertyPageProvider *provider,
	GList *files)
{
	// Only display page for a single file
	if (!files || files->next)
		return NULL;

	GFileInfo *info = thunarx_file_info_get_file_info(files->data);
	GFileType type = g_file_info_get_file_type(info);
	g_object_unref(info);

	// Only display page for regular files
	if (type != G_FILE_TYPE_REGULAR)
		return NULL;

	char *uri = thunarx_file_info_get_uri(files->data);

	struct page_s *page = gtkhash_properties_new_page(uri);
	if (!page)
		return NULL;

	GtkWidget *ppage = thunarx_property_page_new(_("Checksums"));
	gtk_container_add(GTK_CONTAINER(ppage), page->box);

	return g_list_append(NULL, ppage);
}

static void gtkhash_properties_pp_iface_init(ThunarxPropertyPageProviderIface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_pages = gtkhash_properties_get_pages;
}

G_MODULE_EXPORT void thunar_extension_initialize(GTypeModule *module);
G_MODULE_EXPORT void thunar_extension_initialize(GTypeModule *module)
{
	gtkhash_module_register_type(module, THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
		(GInterfaceInitFunc)gtkhash_properties_pp_iface_init, sizeof(GObject));
}

G_MODULE_EXPORT void thunar_extension_shutdown(void);
G_MODULE_EXPORT void thunar_extension_shutdown(void)
{
}

G_MODULE_EXPORT void thunar_extension_list_types(const GType **types, int *num_types);
G_MODULE_EXPORT void thunar_extension_list_types(const GType **types, int *num_types)
{
	gtkhash_module_list_types(types, num_types);
}
