/*
 *   Copyright (C) 2007-2020 Tristan Heaven <tristan@tristanheaven.net>
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

#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-list.h"
#include "properties-hash.h"
#include "../hash/hash-func.h"
#include "../hash/digest-format.h"

enum {
	COL_ID,
	COL_ENABLED,
	COL_HASH_FUNC,
	COL_DIGEST
};

inline static GtkTreeModelFilter *gtkhash_properties_list_get_filter(
	struct page_s *page)
{
	return GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model(page->treeview));
}

inline static GtkTreeModel *gtkhash_properties_list_get_model(
	struct page_s *page)
{
	GtkTreeModelFilter *filter = gtkhash_properties_list_get_filter(page);
	return gtk_tree_model_filter_get_model(filter);
}

static gint gtkhash_sort_iter_compare_func (GtkTreeModel *model,
						  GtkTreeIter  *a,
						  GtkTreeIter  *b,
						  gpointer	  userdata)
{
  guint enabled1, enabled2;
  gtk_tree_model_get(model, a, COL_ENABLED, &enabled1, -1);
  gtk_tree_model_get(model, b, COL_ENABLED, &enabled2, -1);

  if (enabled1 > enabled2)
	return -1;
  else if (enabled1 < enabled2)
	return 1;
  else
  {
	gint ret = 0;
	gchar *name1 = NULL, *name2 = NULL;
	gtk_tree_model_get(model, a, COL_HASH_FUNC, &name1, -1);
	gtk_tree_model_get(model, b, COL_HASH_FUNC, &name2, -1);

	if (name1 == NULL || name2 == NULL)
	{
	  if (name1 != NULL || name2 != NULL)
		ret = (name1 == NULL) ? -1 : 1;
	}
	else
	{
	  ret = g_utf8_collate(name1,name2);
	}

	g_free(name1);
	g_free(name2);
	return ret;
  }
}

static void gtkhash_sort_list(GtkTreeModel *liststore)
{
  GtkTreeSortable *sortable = GTK_TREE_SORTABLE(liststore);

  gtk_tree_sortable_set_sort_func(sortable, COL_ENABLED, gtkhash_sort_iter_compare_func,
								  NULL, NULL);
  /* set initial sort order */
  gtk_tree_sortable_set_sort_column_id(sortable, COL_ENABLED, GTK_SORT_ASCENDING);
}

inline static GtkListStore *gtkhash_properties_list_get_store(
	struct page_s *page)
{
	return GTK_LIST_STORE(gtkhash_properties_list_get_model(page));
}

void gtkhash_properties_list_update_enabled(struct page_s *page,
	char *path_str)
{
	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkListStore *store = gtkhash_properties_list_get_store(page);
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(model, &iter, path_str);

	int id;
	gboolean enabled;
	gtk_tree_model_get(model, &iter, COL_ID, &id, COL_ENABLED, &enabled, -1);
	enabled = !enabled;
	gtk_list_store_set(store, &iter, COL_ENABLED, enabled, -1);

	if (!enabled) {
		// Clear digest for disabled func
		gtk_list_store_set(store, &iter, COL_DIGEST, "", -1);
	}

	page->funcs[id].enabled = enabled;

	gtkhash_sort_list(model);
}

void gtkhash_properties_list_reset_enabled(struct page_s *page)
{
	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	do {
		int id;
		gboolean enabled;
		gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
		gtk_tree_model_get(model, &iter, COL_ENABLED, &enabled, -1);

		page->funcs[id].enabled = enabled;
	} while (gtk_tree_model_iter_next(model, &iter));

	gtkhash_sort_list(model);
}

void gtkhash_properties_list_update_hash_func_names(struct page_s *page)
{
	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	GtkListStore *store = gtkhash_properties_list_get_store(page);
	const bool hmac = gtk_toggle_button_get_active(page->togglebutton_hmac);

	do {
		int id;
		gtk_tree_model_get(model, &iter, COL_ID, &id, -1);

		if (!page->funcs[id].hmac_supported)
			continue;

		if (hmac) {
			char *name = g_strdup_printf("HMAC-%s",
				page->funcs[id].name);
			gtk_list_store_set(store, &iter, COL_HASH_FUNC, name, -1);
			g_free(name);
		} else {
			gtk_list_store_set(store, &iter, COL_HASH_FUNC,
				page->funcs[id].name, -1);
		}

		// Digest is invalid now, so clear it
		gtk_list_store_set(store, &iter, COL_DIGEST, "", -1);
	} while (gtk_tree_model_iter_next(model, &iter));

	gtk_tree_view_columns_autosize(page->treeview);
	gtkhash_sort_list(model);
}

void gtkhash_properties_list_clear_digests(struct page_s *page)
{
	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	GtkListStore *store = gtkhash_properties_list_get_store(page);

	do {
		gtk_list_store_set(store, &iter, COL_DIGEST, "", -1);
	} while (gtk_tree_model_iter_next(model, &iter));

	gtk_tree_view_columns_autosize(page->treeview);
	gtkhash_sort_list(model);
}

void gtkhash_properties_list_check_digests(struct page_s *page)
{
	const char *check = gtk_entry_get_text(page->entry_check);
	const char *icon = NULL;

	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkTreeIter iter;

	if (*check && gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			char *digest = NULL;;
			gtk_tree_model_get(model, &iter, COL_DIGEST, &digest, -1);

			if (gtkhash_digest_format_compare(check, digest, DIGEST_FORMAT_HEX_LOWER))
				icon = "gtk-yes";

			g_free(digest);
		} while (!icon && gtk_tree_model_iter_next(model, &iter));
	}

	gtk_entry_set_icon_from_icon_name(page->entry_check,
		GTK_ENTRY_ICON_SECONDARY, icon);
}

void gtkhash_properties_list_set_digest(struct page_s *page,
	const enum hash_func_e id, const char *digest)
{
	GtkTreeModel *model = gtkhash_properties_list_get_model(page);
	GtkListStore *store = gtkhash_properties_list_get_store(page);
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		g_assert_not_reached();

	do {
		int row_id;
		gtk_tree_model_get(model, &iter, COL_ID, &row_id, -1);

		if (row_id == id) {
			gtk_list_store_set(store, &iter, COL_DIGEST, digest, -1);
			return;
		}
	} while (gtk_tree_model_iter_next(model, &iter));

	g_assert_not_reached();
}

char *gtkhash_properties_list_get_selected_digest(struct page_s *page)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(page->treeselection, &model, &iter))
		return NULL;

	char *digest = NULL;;
	gtk_tree_model_get(model, &iter, COL_DIGEST, &digest, -1);

	if (digest && *digest)
		return digest;
	else {
		g_free(digest);
		return NULL;
	}
}

bool gtkhash_properties_list_hash_selected(struct page_s *page)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(page->treeselection, &model, &iter))
		return false;

	gboolean enabled;
	gtk_tree_model_get(model, &iter, COL_ENABLED, &enabled, -1);
	if (!enabled)
		return false;

	int id;
	gtk_tree_model_get(model, &iter, COL_ID, &id, -1);

	if (gtk_toggle_button_get_active(page->togglebutton_hmac)) {
		const uint8_t *hmac_key = (uint8_t *)gtk_entry_get_text(
			page->entry_hmac);
		GtkEntryBuffer *buffer = gtk_entry_get_buffer(page->entry_hmac);
		const size_t key_size = gtk_entry_buffer_get_bytes(buffer);
		gtkhash_properties_hash_start(page, &page->funcs[id], hmac_key, key_size);
	} else
		gtkhash_properties_hash_start(page, &page->funcs[id], NULL, 0);

	return true;
}

void gtkhash_properties_list_refilter(struct page_s *page)
{
	GtkTreeModelFilter *filter = gtkhash_properties_list_get_filter(page);
	gtk_tree_model_filter_refilter(filter);

	bool active = gtk_check_menu_item_get_active(page->menuitem_show_funcs);
	GtkTreeViewColumn *col = gtk_tree_view_get_column(page->treeview, 0);
	gtk_tree_view_column_set_visible(col, active);

	gtk_tree_view_columns_autosize(page->treeview);
}

static gboolean gtkhash_properties_list_filter(GtkTreeModel *model,
	GtkTreeIter *iter, struct page_s *page)
{
	gboolean enabled;
	gtk_tree_model_get(model, iter, COL_ENABLED, &enabled, -1);

	if (!enabled && !gtk_check_menu_item_get_active(page->menuitem_show_funcs))
		return false;

	return true;
}

void gtkhash_properties_list_init(struct page_s *page)
{
	GtkListStore *store = gtkhash_properties_list_get_store(page);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!page->funcs[i].supported)
			continue;

		gtk_list_store_insert_with_values(store, NULL, i,
			COL_ID, i,
			COL_ENABLED, page->funcs[i].enabled,
			COL_HASH_FUNC, page->funcs[i].name,
			COL_DIGEST, "",
			-1);
	}

	GtkTreeModelFilter *filter = gtkhash_properties_list_get_filter(page);
	gtk_tree_model_filter_set_visible_func(filter,
		(GtkTreeModelFilterVisibleFunc)gtkhash_properties_list_filter, page,
		NULL);

	gtkhash_properties_list_refilter(page);
}

