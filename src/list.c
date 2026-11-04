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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "list.h"
#include "main.h"
#include "hash.h"
#include "gui.h"
#include "hash/digest-format.h"

enum {
	COL_ICON_NAME, // 0
	COL_PNAME,     // 1
	COL_TOOLTIP,   // 2
	COL_CHECK,     // 3
	COL_DIGESTS,   // 4+
};

struct {
	GtkTreeViewColumn *view_cols[HASH_FUNCS_N];
	int hash_cols[HASH_FUNCS_N];
	bool show_status;
} list_priv = {
	.show_status = false,
};

void list_init(void)
{
	int cols = COL_DIGESTS;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		list_priv.hash_cols[i] = cols;
		list_priv.view_cols[i] = gtk_tree_view_column_new_with_attributes(
			hash.funcs[i].name, gtk_cell_renderer_text_new(), "text", cols, NULL);
		gtk_tree_view_column_set_resizable(list_priv.view_cols[i], true);
		gtk_tree_view_column_set_min_width(list_priv.view_cols[i], 10);
		gtk_tree_view_append_column(gui.treeview, list_priv.view_cols[i]);
		cols++;
	}

	GType types[cols];
	types[COL_ICON_NAME] = G_TYPE_STRING;
	types[COL_PNAME]     = G_TYPE_STRING;
	types[COL_TOOLTIP]   = G_TYPE_STRING;
	types[COL_CHECK]     = G_TYPE_STRING;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		types[list_priv.hash_cols[i]] = G_TYPE_STRING;
	}

	gui.liststore = gtk_list_store_newv(cols, types);
	gui.treemodel = GTK_TREE_MODEL(gui.liststore);

	gtk_tree_view_set_model(gui.treeview, gui.treemodel);

	const GtkTargetEntry targets[] = {
		{ (char *)"text/uri-list", 0, 0 }
	};
	gtk_drag_dest_set(GTK_WIDGET(gui.treeview), GTK_DEST_DEFAULT_ALL, targets,
		G_N_ELEMENTS(targets), GDK_ACTION_COPY);
}

int list_get_rows(void)
{
	return gtk_tree_model_iter_n_children(gui.treemodel, NULL);
}

void list_update(void)
{
	const bool enabled = gtkhash_hash_func_count_enabled(hash.funcs);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		GtkTreeViewColumn *col = list_priv.view_cols[i];
		gtk_tree_view_column_set_visible(col, hash.funcs[i].enabled);
	}

	gtk_tree_view_column_set_visible(gui.treeviewcolumn_status, list_priv.show_status);

	const bool has_rows = (list_get_rows() > 0);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_clear), has_rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_clear), has_rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), has_rows && enabled);
}

void list_append_row(const char * const uri, const char * const check)
{
	g_assert(uri);

	GFile *file = g_file_new_for_uri(uri);
	char *pname = g_file_get_parse_name(file);
	char *basename = g_filename_display_basename(pname);
	char *tooltip = g_markup_escape_text(basename, -1);

	gtk_list_store_insert_with_values(gui.liststore, NULL, -1,
		COL_ICON_NAME, "",
		COL_PNAME, pname,
		COL_TOOLTIP, tooltip,
		COL_CHECK, check ? check : "",
		-1);

	g_free(tooltip);
	g_free(basename);
	g_free(pname);
	g_object_unref(file);

	if (check)
		list_priv.show_status = true;
}

void list_remove_selection(void)
{
	if (gtk_tree_selection_count_selected_rows(gui.treeselection) == list_get_rows()) {
		list_clear();
		return;
	}

	GList *paths = gtk_tree_selection_get_selected_rows(gui.treeselection, &gui.treemodel);

	GtkTreeIter *iters = g_new(GtkTreeIter, g_list_length(paths));
	int iters_n = 0;

	for (GList *i = paths; i != NULL; i = i->next) {
		GtkTreePath *path = i->data;
		if (gtk_tree_model_get_iter(gui.treemodel, &iters[iters_n], path))
			iters_n++;
		gtk_tree_path_free(path);
	}

	g_list_free(paths);

	for (int i = 0; i < iters_n; i++)
		gtk_list_store_remove(gui.liststore, &iters[i]);

	g_free(iters);

	gtk_tree_selection_unselect_all(gui.treeselection);
}

GPtrArray *list_get_iters(void)
{
	GPtrArray *iters = g_ptr_array_new_with_free_func(g_free);

	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(gui.treemodel, &iter)) {
		do {
			GtkTreeIter *copy = g_new(GtkTreeIter, 1);
			*copy = iter;
			g_ptr_array_add(iters, copy);
		} while (gtk_tree_model_iter_next(gui.treemodel, &iter));
	}

	return iters;
}


static GFile *list_get_iter_file(GtkTreeIter *iter)
{
	char *pname;
	gtk_tree_model_get(gui.treemodel, iter, COL_PNAME, &pname, -1);
	GFile *file = g_file_parse_name(pname);
	g_free(pname);

	return file;
}

static GFile *list_get_file(const int row)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		return NULL;

	return list_get_iter_file(&iter);
}

char *list_get_uri(GtkTreeIter *iter)
{
	GFile *file = list_get_iter_file(iter);
	char *uri = g_file_get_uri(file);
	g_object_unref(file);

	return uri;
}

char *list_get_basename(const int row)
{
	GFile *file = list_get_file(row);
	if (!file)
		return NULL;

	char *basename = g_file_get_basename(file);
	g_object_unref(file);

	return basename;
}

static void list_scroll_to_next(const GtkTreeIter *iter)
{
	GtkTreeIter next = *iter;

	if (!gtk_tree_model_iter_next(gui.treemodel, &next))
		return;

	GtkTreePath *path = gtk_tree_model_get_path(gui.treemodel, &next);
	gtk_tree_view_scroll_to_cell(gui.treeview, path, NULL, false, 0, 0);
	gtk_tree_path_free(path);
}

void list_set_digest(GtkTreeIter *iter, const enum hash_func_e id, const char * const digest)
{
	g_assert(HASH_FUNC_IS_VALID(id));

	list_scroll_to_next(iter);

	gtk_list_store_set(gui.liststore, iter, list_priv.hash_cols[id], digest, -1);
}

char *list_get_digest(const int row, const enum hash_func_e id)
{
	g_assert(HASH_FUNC_IS_VALID(id));

	GtkTreeIter iter;

	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		return NULL;

	char *digest;
	gtk_tree_model_get(gui.treemodel, &iter, list_priv.hash_cols[id], &digest, -1);

	return digest;
}

char *list_get_selected_digest(const enum hash_func_e id)
{
	GList *rows = gtk_tree_selection_get_selected_rows(gui.treeselection, &gui.treemodel);

	// Should only have one row selected
	g_assert(rows);
	g_assert(!rows->next);

	GtkTreeIter iter;
	char *digest = NULL;

	if (gtk_tree_model_get_iter(gui.treemodel, &iter, rows->data))
		gtk_tree_model_get(gui.treemodel, &iter, list_priv.hash_cols[id], &digest, -1);

	gtk_tree_path_free(rows->data);
	g_list_free(rows);

	return digest;
}

bool list_has_digests(void)
{
	int cols[HASH_FUNCS_N];
	int cols_n = 0;

	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (hash.funcs[i].enabled)
			cols[cols_n++] = list_priv.hash_cols[i];

	GtkTreeIter iter;
	if (!cols_n || !gtk_tree_model_get_iter_first(gui.treemodel, &iter))
		return false;

	do {
		for (int i = 0; i < cols_n; i++) {
			GValue value = G_VALUE_INIT;
			gtk_tree_model_get_value(gui.treemodel, &iter, cols[i], &value);
			const char *digest = g_value_get_string(&value);
			const bool has_digest = (digest && *digest);
			g_value_unset(&value);

			if (has_digest)
				return true;
		}
	} while (gtk_tree_model_iter_next(gui.treemodel, &iter));

	return false;
}

void list_check_digests(GtkTreeIter *iter)
{
	char *check;
	gtk_tree_model_get(gui.treemodel, iter, COL_CHECK, &check, -1);

	const char *icon_name = "";
	bool has_digest = false;
	bool match = false;

	if (check && *check) {
		const enum digest_format_e format = gui_get_digest_format();

		for (int i = 0; (i < HASH_FUNCS_N) && !match; i++) {
			if (!hash.funcs[i].enabled)
				continue;

			char *digest;
			gtk_tree_model_get(gui.treemodel, iter, list_priv.hash_cols[i], &digest, -1);

			if (digest && *digest)
				has_digest = true;

			match = gtkhash_digest_format_compare(digest, check, format);

			g_free(digest);
		}

		if (has_digest)
			icon_name = match ? "object-select-symbolic" : "window-close-symbolic";
	}

	g_free(check);

	gtk_list_store_set(gui.liststore, iter, COL_ICON_NAME, icon_name, -1);
}

void list_check_all_digests(void)
{
	GPtrArray *iters = list_get_iters();

	for (unsigned int i = 0; i < iters->len; i++)
		list_check_digests(iters->pdata[i]);

	g_ptr_array_unref(iters);
}

void list_clear_digests(void)
{
	int cols[HASH_FUNCS_N + 1];
	GValue vals[HASH_FUNCS_N + 1] = { G_VALUE_INIT };
	int vals_n = 0;

	cols[vals_n] = COL_ICON_NAME;
	g_value_init(&vals[vals_n], G_TYPE_STRING);
	g_value_set_static_string(&vals[vals_n], "");
	vals_n++;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		cols[vals_n] = list_priv.hash_cols[i];
		g_value_init(&vals[vals_n], G_TYPE_STRING);
		g_value_set_static_string(&vals[vals_n], "");
		vals_n++;
	}

	GPtrArray *iters = list_get_iters();

	for (unsigned int i = 0; i < iters->len; i++)
		gtk_list_store_set_valuesv(gui.liststore, iters->pdata[i], cols, vals, vals_n);

	for (int i = 0; i < vals_n; i++)
		g_value_unset(&vals[i]);

	g_ptr_array_unref(iters);
}

void list_clear(void)
{
	gtk_list_store_clear(gui.liststore);
	list_priv.show_status = false;

	list_update();
}
