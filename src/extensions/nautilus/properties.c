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
#include <gio/gio.h>
#include <nautilus-extension.h>

#include "../common/module.h"
#include "properties.h"
#include "properties-hash.h"
#include "properties-prefs.h"

static void gtkhash_properties_model_start(struct model_s *model)
{
	if (model->state != MODEL_STATE_NEW)
		return;

	model->state = MODEL_STATE_BUSY;
	g_object_ref(model); // unrefed in hash finish/stop
	gtkhash_properties_hash_start(model);
}

static gpointer gtkhash_properties_model_get_item(GListModel *list, guint position)
{
	struct model_s *model = (struct model_s *)list;

	gtkhash_properties_model_start(model);

	if (position >= model->items->len)
		return NULL;

	return g_object_ref(g_ptr_array_index(model->items, position));
}

static GType gtkhash_properties_model_get_item_type(G_GNUC_UNUSED GListModel *list)
{
	return NAUTILUS_TYPE_PROPERTIES_ITEM;
}

static guint gtkhash_properties_model_get_n_items(GListModel *list)
{
	struct model_s *model = (struct model_s *)list;

	gtkhash_properties_model_start(model);

	return model->items->len;
}

static void gtkhash_properties_model_iface_init(GListModelInterface *iface,
	G_GNUC_UNUSED void *data)
{
	/* A minimal implementation of GListModel allowing _model_start() to be deferred until the user
	   selects "Checksums" in file properties. The delay isn't guaranteed by the Nautilus API. */

	iface->get_item = gtkhash_properties_model_get_item;
	iface->get_item_type = gtkhash_properties_model_get_item_type;
	iface->get_n_items = gtkhash_properties_model_get_n_items;
}

void gtkhash_properties_model_set_progress(struct model_s *model, const char *progress)
{
	const guint removed = model->items->len;

	g_ptr_array_set_size(model->items, 0);
	g_ptr_array_add(model->items, nautilus_properties_item_new(progress, ""));

	g_list_model_items_changed(G_LIST_MODEL(model), 0, removed, 1);
}

void gtkhash_properties_model_add_digest(struct model_s *model, const char *name,
	const char *digest)
{
	if (model->state == MODEL_STATE_BUSY) {
		model->state = MODEL_STATE_DONE;

		const guint removed = model->items->len;
		if (removed > 0) {
			g_ptr_array_set_size(model->items, 0);
			g_list_model_items_changed(G_LIST_MODEL(model), 0, removed, 0);
		}
	}

	g_ptr_array_add(model->items, nautilus_properties_item_new(name, digest ? digest : ""));
	g_list_model_items_changed(G_LIST_MODEL(model), model->items->len - 1, 0, 1);
}

static void gtkhash_properties_model_deinit(struct model_s *model, G_GNUC_UNUSED GObject *object)
{
	gtkhash_properties_hash_deinit(model);
	g_ptr_array_unref(model->items);
	g_free(model->uri);
}

static struct model_s *gtkhash_properties_model_new(char *uri)
{
	struct model_s *model = g_object_new(gtkhash_module_get_type(), NULL);

	g_object_weak_ref(G_OBJECT(model), (GWeakNotify)gtkhash_properties_model_deinit, model);

	model->uri = uri;
	model->items = g_ptr_array_new_with_free_func(g_object_unref);

	gtkhash_properties_hash_init(model);
	gtkhash_properties_prefs_load(model);

	// Placeholder for progress string
	gtkhash_properties_model_set_progress(model, "");

	return model;
}

static void gtkhash_properties_model_unref(struct model_s *model, G_GNUC_UNUSED GObject *pm)
{
	if (model->state == MODEL_STATE_BUSY)
		gtkhash_properties_hash_stop(model);

	g_object_unref(model);
}

static GList *gtkhash_properties_get_models(G_GNUC_UNUSED NautilusPropertiesModelProvider *provider,
	GList *files)
{
	// Only display for a single file
	if (!files || files->next)
		return NULL;

	// Only display for regular files
	if (nautilus_file_info_get_file_type(files->data) != G_FILE_TYPE_REGULAR)
		return NULL;

	char *uri = nautilus_file_info_get_uri(files->data);

	struct model_s *model = gtkhash_properties_model_new(uri);
	NautilusPropertiesModel *pm = nautilus_properties_model_new(_("Checksums"), G_LIST_MODEL(model));

	// unrefed when the file properties dialog is closed
	g_object_weak_ref(G_OBJECT(pm), (GWeakNotify)gtkhash_properties_model_unref, model);

	return g_list_append(NULL, pm);
}

static void gtkhash_properties_pm_iface_init(NautilusPropertiesModelProviderInterface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_models = gtkhash_properties_get_models;
}

void gtkhash_properties_register(GTypeModule *module)
{
	const GType type = gtkhash_module_register_type(module,
		NAUTILUS_TYPE_PROPERTIES_MODEL_PROVIDER,
		(GInterfaceInitFunc)gtkhash_properties_pm_iface_init,
		sizeof(struct model_s));

	const GInterfaceInfo interface_info = {
		(GInterfaceInitFunc)gtkhash_properties_model_iface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, type, G_TYPE_LIST_MODEL, &interface_info);
}
