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
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-list.h"
#include "properties-hash.h"
#include "../../hash/hash-func.h"
#include "../../hash/digest-format.h"

enum {
	COL_ID,
	COL_ENABLED,
	COL_HASH_FUNC,
	COL_DIGEST
};

void gtkhash_properties_list_update_enabled(struct page_s *page, char *path_str)
{
	GtkTreeModel *filter_model = GTK_TREE_MODEL(page->treemodelfilter);

	GtkTreeIter filter_iter;
	if (!gtk_tree_model_get_iter_from_string(filter_model, &filter_iter, path_str))
		return;

	GtkTreeIter iter;
	gtk_tree_model_filter_convert_iter_to_child_iter(page->treemodelfilter, &iter, &filter_iter);

	int id;
	gboolean enabled;
	gtk_tree_model_get(GTK_TREE_MODEL(page->liststore), &iter,
		COL_ID, &id,
		COL_ENABLED, &enabled,
		-1);
	enabled = !enabled;

	gtk_list_store_set(page->liststore, &iter,
		COL_ENABLED, enabled,
		COL_DIGEST, "",
		-1);

	page->funcs[id].enabled = enabled;
}

void gtkhash_properties_list_reset_enabled(struct page_s *page)
{
	GtkTreeModel *model = GTK_TREE_MODEL(page->liststore);

	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		int id;
		gboolean enabled;
		gtk_tree_model_get(model, &iter,
			COL_ID, &id,
			COL_ENABLED, &enabled,
			-1);
		page->funcs[id].enabled = enabled;
	} while (gtk_tree_model_iter_next(model, &iter));
}

void gtkhash_properties_list_update_hash_func_names(struct page_s *page)
{
	GtkTreeModel *model = GTK_TREE_MODEL(page->liststore);

	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	const bool hmac_enabled = gtk_toggle_button_get_active(page->togglebutton_hmac);

	do {
		int id;
		gtk_tree_model_get(model, &iter, COL_ID, &id, -1);

		if (!page->funcs[id].hmac_supported)
			continue;

		gtk_list_store_set(page->liststore, &iter,
			COL_HASH_FUNC, hmac_enabled ? page->funcs[id].hmac_name : page->funcs[id].name,
			COL_DIGEST, "",
			-1);
	} while (gtk_tree_model_iter_next(model, &iter));
}

void gtkhash_properties_list_clear_digests(struct page_s *page)
{
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(page->liststore), &iter))
		return;

	do {
		gtk_list_store_set(page->liststore, &iter, COL_DIGEST, "", -1);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(page->liststore), &iter));
}

void gtkhash_properties_list_check_digests(struct page_s *page)
{
	const char *check = gtk_entry_get_text(page->entry_check);
	const char *icon = NULL;
	GtkTreeModel *model = GTK_TREE_MODEL(page->liststore);

	GtkTreeIter iter;
	if (*check && gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			char *digest = NULL;
			gtk_tree_model_get(model, &iter, COL_DIGEST, &digest, -1);

			if (gtkhash_digest_format_compare(check, digest, DIGEST_FORMAT_HEX_LOWER))
				icon = "object-select-symbolic";

			g_free(digest);
		} while (!icon && gtk_tree_model_iter_next(model, &iter));
	}

	gtk_entry_set_icon_from_icon_name(page->entry_check, GTK_ENTRY_ICON_SECONDARY, icon);
}

void gtkhash_properties_list_set_digest(struct page_s *page, const enum hash_func_e id,
	const char *digest)
{
	GtkTreeModel *model = GTK_TREE_MODEL(page->liststore);

	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(model, &iter))
		g_assert_not_reached();

	do {
		int row_id;
		gtk_tree_model_get(model, &iter, COL_ID, &row_id, -1);

		if (row_id == id) {
			gtk_list_store_set(page->liststore, &iter, COL_DIGEST, digest, -1);
			return;
		}
	} while (gtk_tree_model_iter_next(model, &iter));

	g_assert_not_reached();
}

char *gtkhash_properties_list_get_selected_digest(struct page_s *page)
{
	GtkTreeModel *filter_model;
	GtkTreeIter filter_iter;
	if (!gtk_tree_selection_get_selected(page->treeselection, &filter_model, &filter_iter))
		return NULL;

	char *digest = NULL;
	gtk_tree_model_get(filter_model, &filter_iter, COL_DIGEST, &digest, -1);

	if (digest && *digest)
		return digest;
	else {
		g_free(digest);
		return NULL;
	}
}

bool gtkhash_properties_list_hash_selected(struct page_s *page)
{
	GtkTreeModel *filter_model;
	GtkTreeIter filter_iter;
	if (!gtk_tree_selection_get_selected(page->treeselection, &filter_model, &filter_iter))
		return false;

	gboolean enabled;
	gtk_tree_model_get(filter_model, &filter_iter, COL_ENABLED, &enabled, -1);
	if (!enabled)
		return false;

	int id;
	gtk_tree_model_get(filter_model, &filter_iter, COL_ID, &id, -1);

	gtkhash_properties_list_set_digest(page, id, "");

	if (gtk_toggle_button_get_active(page->togglebutton_hmac)) {
		const uint8_t *hmac_key = (uint8_t *)gtk_entry_get_text(page->entry_hmac);
		GtkEntryBuffer *buffer = gtk_entry_get_buffer(page->entry_hmac);
		const size_t key_size = gtk_entry_buffer_get_bytes(buffer);
		gtkhash_properties_hash_start(page, &page->funcs[id], hmac_key, key_size);
	} else
		gtkhash_properties_hash_start(page, &page->funcs[id], NULL, 0);

	return true;
}

void gtkhash_properties_list_refilter(struct page_s *page)
{
	gtkhash_properties_list_sort(page);

	gtk_tree_model_filter_refilter(page->treemodelfilter);

	gtk_tree_view_column_set_visible(page->treeviewcolumn_enabled,
		gtk_check_menu_item_get_active(page->menuitem_show_funcs));
}

static gboolean gtkhash_properties_list_filter(GtkTreeModel *model, GtkTreeIter *iter,
	struct page_s *page)
{
	gboolean enabled;
	gtk_tree_model_get(model, iter, COL_ENABLED, &enabled, -1);

	if (!enabled && !gtk_check_menu_item_get_active(page->menuitem_show_funcs))
		return false;

	return true;
}

void gtkhash_properties_list_sort(struct page_s *page)
{
	// Enable for immediate sorting
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(page->liststore),
		COL_ENABLED, GTK_SORT_ASCENDING);

	// Disable sorting to keep the order stable when COL_ENABLED is toggled
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(page->liststore),
		GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
}

static int gtkhash_properties_list_sort_func(GtkTreeModel *model,
	GtkTreeIter *iter_a, GtkTreeIter *iter_b, G_GNUC_UNUSED void *data)
{
	// Sort by COL_ENABLED
	gboolean enabled_a, enabled_b;
	gtk_tree_model_get(model, iter_a, COL_ENABLED, &enabled_a, -1);
	gtk_tree_model_get(model, iter_b, COL_ENABLED, &enabled_b, -1);
	if (enabled_a != enabled_b)
		return enabled_a ? -1 : 1;

	// Sort by COL_ID
	int id_a, id_b;
	gtk_tree_model_get(model, iter_a, COL_ID, &id_a, -1);
	gtk_tree_model_get(model, iter_b, COL_ID, &id_b, -1);
	return (id_a > id_b) - (id_a < id_b);
}

void gtkhash_properties_list_init(struct page_s *page)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!page->funcs[i].supported)
			continue;

		gtk_list_store_insert_with_values(page->liststore, NULL, i,
			COL_ID, i,
			COL_ENABLED, page->funcs[i].enabled,
			COL_HASH_FUNC, page->funcs[i].name,
			COL_DIGEST, "",
			-1);
	}

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(page->liststore), COL_ENABLED,
		gtkhash_properties_list_sort_func, NULL, NULL);

	gtk_tree_model_filter_set_visible_func(page->treemodelfilter,
		(GtkTreeModelFilterVisibleFunc)gtkhash_properties_list_filter, page, NULL);

	gtkhash_properties_list_refilter(page);
}
