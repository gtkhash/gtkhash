/*
 *   Copyright (C) 2007-2016 Tristan Heaven <tristan@tristanheaven.net>
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

enum {
	COL_STATUS    = 0,
	COL_ICON_NAME = 1,
	COL_PNAME     = 2,
	COL_CHECK     = 3,
	COL_DIGESTS  // 4+
};

struct list_s list;

struct {
	int hash_cols[HASH_FUNCS_N];
	bool show_status;
} list_priv = {
	.show_status = false,
};

void list_init(void)
{
	list.rows = 0;

	int cols = COL_DIGESTS;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		list_priv.hash_cols[i] = cols;
		cols = gtk_tree_view_insert_column_with_attributes(gui.treeview, -1,
			hash.funcs[i].name, gtk_cell_renderer_text_new(),
			"text", cols, NULL);
	}

	GType types[cols];
	types[COL_STATUS]    = G_TYPE_CHAR;
	types[COL_ICON_NAME] = G_TYPE_STRING;
	types[COL_PNAME]     = G_TYPE_STRING;
	types[COL_CHECK]     = G_TYPE_STRING;

	for (int i = COL_DIGESTS; i < cols; i++) {
		types[i] = G_TYPE_STRING;
		GtkTreeViewColumn *col = gtk_tree_view_get_column(gui.treeview, i);
		gtk_tree_view_column_set_resizable(col, true);
		gtk_tree_view_column_set_min_width(col, 10);
	}

	gui.liststore = gtk_list_store_newv(cols, types);
	gui.treemodel = GTK_TREE_MODEL(gui.liststore);

	gtk_tree_view_set_model(gui.treeview, gui.treemodel);

#if (GTK_MAJOR_VERSION < 3)
	gtk_tree_selection_set_mode(gui.treeselection, GTK_SELECTION_MULTIPLE);
#endif

	const GtkTargetEntry targets[] = {
		{ (char *)"text/uri-list", 0, 0 }
	};
	gtk_drag_dest_set(GTK_WIDGET(gui.treeview), GTK_DEST_DEFAULT_ALL, targets,
		G_N_ELEMENTS(targets), GDK_ACTION_COPY);
}

void list_update(void)
{
	bool enabled = false;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;
		if (hash.funcs[i].enabled)
			enabled = true;

		GtkTreeViewColumn *col = gtk_tree_view_get_column(gui.treeview,
			list_priv.hash_cols[i]);
		gtk_tree_view_column_set_visible(col, hash.funcs[i].enabled);
	}

	GtkTreeViewColumn *col = gtk_tree_view_get_column(gui.treeview, COL_STATUS);
	gtk_tree_view_column_set_visible(col, list_priv.show_status);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.toolbutton_clear), list.rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_clear), list.rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), list.rows && enabled);
}

void list_append_row(const char * const uri, const char * const check)
{
	g_assert(uri);

	GFile *file = g_file_new_for_uri(uri);
	char *pname = g_file_get_parse_name(file);
	g_object_unref(file);

	gtk_list_store_insert_with_values(gui.liststore, NULL, list.rows + 1,
		COL_PNAME, pname,
		COL_CHECK, check ? check : "",
		-1);
	list.rows++;
	g_free(pname);

	if (check)
		list_priv.show_status = true;
}

void list_remove_selection(void)
{
	if (gtk_tree_selection_count_selected_rows(gui.treeselection) ==
		(int)list.rows)
	{
		list_clear();
		return;
	}

	GList *rows = gtk_tree_selection_get_selected_rows(gui.treeselection,
		&gui.treemodel);

	for (GList *i = rows; i != NULL; i = i->next) {
		GtkTreePath *path = i->data;
		GtkTreeRowReference *ref = gtk_tree_row_reference_new(gui.treemodel,
			i->data);
		i->data = ref;
		gtk_tree_path_free(path);
	}

	for (GList *i = rows; i != NULL; i = i->next) {
		GtkTreeRowReference *ref = i->data;
		GtkTreePath *path = gtk_tree_row_reference_get_path(ref);
		GtkTreeIter iter;

		if (gtk_tree_model_get_iter(gui.treemodel, &iter, path)) {
			gtk_list_store_remove(gui.liststore, &iter);
			list.rows--;
		}

		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}

	g_list_free(rows);

	if (!list.rows) {
		gtk_widget_set_sensitive(GTK_WIDGET(gui.toolbutton_clear), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_clear), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), false);
	}
}

char *list_get_uri(const unsigned int row)
{
	g_assert(row <= list.rows);

	GtkTreeIter iter;

	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		g_assert_not_reached();

	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(gui.treemodel, &iter, COL_PNAME, &value);
	GFile *file = g_file_parse_name(g_value_get_string(&value));
	g_value_unset(&value);

	char *uri = g_file_get_uri(file);
	g_object_unref(file);

	return uri;
}

static void list_scroll_to_next_row(GtkTreeIter iter)
{
	if (!gtk_tree_model_iter_next(gui.treemodel, &iter))
		return;

	GtkTreePath *path = gtk_tree_model_get_path(gui.treemodel, &iter);
	gtk_tree_view_scroll_to_cell(gui.treeview, path, NULL, false, 0, 0);
	gtk_tree_path_free(path);
}

void list_set_digest(const unsigned int row, const enum hash_func_e id,
	const char * const digest)
{
	g_assert(row <= list.rows);
	g_assert(HASH_FUNC_IS_VALID(id));

	GtkTreeIter iter;
	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		g_assert_not_reached();

	list_scroll_to_next_row(iter);

	gtk_list_store_set(gui.liststore, &iter,
		list_priv.hash_cols[id], digest,
		-1);
}

char *list_get_digest(const unsigned int row, const enum hash_func_e id)
{
	g_assert(row <= list.rows);
	g_assert(HASH_FUNC_IS_VALID(id));

	GtkTreeIter iter;

	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		return NULL;

	char *digest;
	GValue value = G_VALUE_INIT;

	gtk_tree_model_get_value(gui.treemodel, &iter, list_priv.hash_cols[id],
		&value);
	digest = g_strdup(g_value_get_string(&value));
	g_value_unset(&value);

	return digest;
}

char *list_get_selected_digest(const enum hash_func_e id)
{
	GList *rows = gtk_tree_selection_get_selected_rows(gui.treeselection,
		&gui.treemodel);

	// Should only have one row selected
	g_assert(rows);
	g_assert(!rows->next);

	GtkTreePath *path = rows->data;
	unsigned int row = *gtk_tree_path_get_indices(path);

	gtk_tree_path_free(path);
	g_list_free(rows);

	return list_get_digest(row, id);
}

void list_check_digests(const unsigned int row)
{
	GtkTreeIter iter;
	if (!gtk_tree_model_iter_nth_child(gui.treemodel, &iter, NULL, row))
		return;

	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(gui.treemodel, &iter, COL_CHECK, &value);
	const char *check = g_value_get_string(&value);

	if (!*check) {
		g_value_unset(&value);
		gtk_list_store_set(gui.liststore, &iter, COL_ICON_NAME, "", -1);
		return;
	}

	const enum digest_format_e format = gui_get_digest_format();
	bool match = false;

	for (int i = 0; (i < HASH_FUNCS_N) && !match; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		GValue digest = G_VALUE_INIT;
		gtk_tree_model_get_value(gui.treemodel, &iter, list_priv.hash_cols[i],
			&digest);

		switch (format) {
			case DIGEST_FORMAT_HEX_LOWER:
			case DIGEST_FORMAT_HEX_UPPER:
				if (g_ascii_strcasecmp(g_value_get_string(&digest), check) == 0)
					match = true;
				break;
			case DIGEST_FORMAT_BASE64:
				if (strcmp(g_value_get_string(&digest), check) == 0)
					match = true;
				break;

			default:
				g_assert_not_reached();
		}

		g_value_unset(&digest);
	}

	g_value_unset(&value);

	// XXX: maybe use ✓ and ✖ instead
	gtk_list_store_set(gui.liststore, &iter, COL_ICON_NAME,
		match ? "gtk-yes" : "gtk-no", -1);
}

void list_clear_digests(void)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(gui.treemodel, &iter))
		return;

	int cols_n = 0;

	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (hash.funcs[i].supported)
			cols_n++;

	gint cols[cols_n];
	GValue vals[HASH_FUNCS_N] = { G_VALUE_INIT };

	for (int i = 0; i < cols_n; i++) {
		cols[i] = COL_DIGESTS + i;
		g_value_init(&vals[i], G_TYPE_STRING);
	}

	do {
		gtk_list_store_set_valuesv(gui.liststore, &iter, cols, vals, cols_n);
	} while (gtk_tree_model_iter_next(gui.treemodel, &iter));
}

void list_clear(void)
{
	gtk_list_store_clear(gui.liststore);
	list.rows = 0;
	list_priv.show_status = false;

	list_update();
}
