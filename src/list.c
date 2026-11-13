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
	COL_CHECK,     // 2
	COL_DIGESTS,   // 3+
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
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer, "family", "Monospace", NULL);
		list_priv.view_cols[i] = gtk_tree_view_column_new_with_attributes(
			hash.funcs[i].name, renderer, "text", cols, NULL);
		gtk_tree_view_column_set_resizable(list_priv.view_cols[i], true);
		gtk_tree_view_column_set_min_width(list_priv.view_cols[i], 10);
		gtk_tree_view_append_column(gui.treeview, list_priv.view_cols[i]);
		cols++;
	}

	GType types[cols];
	types[COL_ICON_NAME] = G_TYPE_STRING;
	types[COL_PNAME]     = G_TYPE_STRING;
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

	gtk_list_store_insert_with_values(gui.liststore, NULL, -1,
		COL_PNAME, pname,
		COL_CHECK, check,
		-1);

	g_free(pname);
	g_object_unref(file);

	if (check)
		list_priv.show_status = true;
}

void list_remove_selection(void)
{
	const int selected_rows = gtk_tree_selection_count_selected_rows(gui.treeselection);

	if (selected_rows == list_get_rows()) {
		list_clear();
		return;
	}

	GList *paths = gtk_tree_selection_get_selected_rows(gui.treeselection, &gui.treemodel);

	// Avoid "selection-changed" signals
	gtk_tree_selection_unselect_all(gui.treeselection);

	GtkTreeIter *iters = g_new(GtkTreeIter, selected_rows);
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
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(gui.treemodel, iter, COL_PNAME, &value);
	GFile *file = g_file_parse_name(g_value_get_string(&value));
	g_value_unset(&value);

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

// Returns a static string or NULL
static const char *list_status_icon_name(const char * const check, const char * const *digests)
{
	if (!check || !*check)
		return NULL;

	const enum digest_format_e format = gui_get_digest_format();

	bool has_digest = false;
	bool match = false;

	for (int i = 0; (i < HASH_FUNCS_N) && !match; i++) {
		if (!digests[i])
			continue;
		if (*digests[i])
			has_digest = true;

		match = gtkhash_digest_format_compare(digests[i], check, format);
	}

	if (has_digest)
		return match ? "object-select-symbolic" : "window-close-symbolic";

	return NULL;
}

void list_set_digests(GtkTreeIter *iter, const char * const *digests)
{
	int cols[HASH_FUNCS_N + 1];
	GValue vals[HASH_FUNCS_N + 1] = { G_VALUE_INIT };
	int vals_n = 0;

	GValue check_value = G_VALUE_INIT;
	gtk_tree_model_get_value(gui.treemodel, iter, COL_CHECK, &check_value);

	cols[vals_n] = COL_ICON_NAME;
	g_value_init(&vals[vals_n], G_TYPE_STRING);
	const char *check = g_value_get_string(&check_value);
	g_value_set_static_string(&vals[vals_n], list_status_icon_name(check, digests));
	vals_n++;

	g_value_unset(&check_value);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!digests[i])
			continue;

		cols[vals_n] = list_priv.hash_cols[i];
		g_value_init(&vals[vals_n], G_TYPE_STRING);
		g_value_set_string(&vals[vals_n], digests[i]);
		vals_n++;
	}

	list_scroll_to_next(iter);

	gtk_list_store_set_valuesv(gui.liststore, iter, cols, vals, vals_n);

	for (int i = 0; i < vals_n; i++)
		g_value_unset(&vals[i]);
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

static void list_check_digests(GtkTreeIter *iter)
{
	GValue check_value = G_VALUE_INIT;
	GValue icon_value = G_VALUE_INIT;

	gtk_tree_model_get_value(gui.treemodel, iter, COL_CHECK, &check_value);
	gtk_tree_model_get_value(gui.treemodel, iter, COL_ICON_NAME, &icon_value);

	const char *check = g_value_get_string(&check_value);
	const char *old_icon = g_value_get_string(&icon_value);
	const char *new_icon = NULL;

	if (check && *check) {
		GValue values[HASH_FUNCS_N] = { G_VALUE_INIT };
		const char *digests[HASH_FUNCS_N] = { NULL };

		for (int i = 0; i < HASH_FUNCS_N; i++) {
			if (!hash.funcs[i].enabled)
				continue;

			gtk_tree_model_get_value(gui.treemodel, iter, list_priv.hash_cols[i], &values[i]);
			digests[i] = g_value_get_string(&values[i]);
		}

		new_icon = list_status_icon_name(check, (const char * const *)digests);

		for (int i = 0; i < HASH_FUNCS_N; i++)
			g_value_unset(&values[i]);
	}

	g_value_unset(&check_value);

	if (g_strcmp0(old_icon, new_icon) != 0)
		gtk_list_store_set(gui.liststore, iter, COL_ICON_NAME, new_icon, -1);

	g_value_unset(&icon_value);
}

static int list_suspend_sort(GtkSortType *order)
{
	int col_id;
	*order = GTK_SORT_ASCENDING;

	if (!gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(gui.liststore), &col_id, order))
		return GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID;

	// Ignore columns which are not modified by bulk updates
	switch (col_id) {
		case COL_PNAME:
		case COL_CHECK:
			return GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID;
		default:
			break;
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(gui.liststore),
		GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return col_id;
}

static void list_restore_sort(const int col_id, const GtkSortType order)
{
	if (col_id == GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID)
		return;

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(gui.liststore), col_id, order);
}

void list_check_all_digests(void)
{
	// Suspend for faster row updates
	GtkSortType order;
	const int col_id = list_suspend_sort(&order);

	GPtrArray *iters = list_get_iters();

	for (unsigned int i = 0; i < iters->len; i++)
		list_check_digests(iters->pdata[i]);

	g_ptr_array_unref(iters);

	list_restore_sort(col_id, order);
}

static void list_suspend_signal(const char * const name)
{
	const unsigned int signal = g_signal_lookup(name, GTK_TYPE_TREE_MODEL);
	g_signal_handlers_block_matched(gui.treemodel, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA,
		signal, 0, NULL, NULL, gui.treeview);
}

static void list_restore_signal(const char * const name)
{
	const unsigned int signal = g_signal_lookup(name, GTK_TYPE_TREE_MODEL);
	g_signal_handlers_unblock_matched(gui.treemodel, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA,
		signal, 0, NULL, NULL, gui.treeview);

	gtk_widget_queue_draw(GTK_WIDGET(gui.treeview));
}

void list_clear_digests(void)
{
	int cols[HASH_FUNCS_N + 1];
	GValue vals[HASH_FUNCS_N + 1] = { G_VALUE_INIT };
	int vals_n = 0;

	cols[vals_n] = COL_ICON_NAME;
	g_value_init(&vals[vals_n], G_TYPE_STRING);
	g_value_set_string(&vals[vals_n], NULL);
	vals_n++;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		cols[vals_n] = list_priv.hash_cols[i];
		g_value_init(&vals[vals_n], G_TYPE_STRING);
		g_value_set_string(&vals[vals_n], NULL);
		vals_n++;
	}

	// Suspend for faster row updates
	GtkSortType order;
	const int col_id = list_suspend_sort(&order);
	list_suspend_signal("row-changed");

	GPtrArray *iters = list_get_iters();

	for (unsigned int i = 0; i < iters->len; i++)
		gtk_list_store_set_valuesv(gui.liststore, iters->pdata[i], cols, vals, vals_n);

	for (int i = 0; i < vals_n; i++)
		g_value_unset(&vals[i]);

	g_ptr_array_unref(iters);

	list_restore_signal("row-changed");
	list_restore_sort(col_id, order);
}

void list_clear(void)
{
	// Temporarily remove treemodel so treeview doesn't have to handle any signals
	gtk_tree_view_set_model(gui.treeview, NULL);

	gtk_list_store_clear(gui.liststore);
	list_priv.show_status = false;

	gtk_tree_view_set_model(gui.treeview, gui.treemodel);

	list_update();
}
