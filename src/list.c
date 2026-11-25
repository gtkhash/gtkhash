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
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "list.h"
#include "hash.h"
#include "gui.h"
#include "hash/digest-format.h"

enum {
	COL_ICON_NAME, // 0
	COL_FILE,      // 1
	COL_CHECK,     // 2
	COL_ERROR,     // 3
	COL_DIGESTS,   // 4+
};

struct {
	GtkTreeViewColumn *view_cols[HASH_FUNCS_N];
	GtkTreeModelFilter *treemodelfilter;
	enum list_filter_e filter;
	unsigned int status_id;
	int hash_cols[HASH_FUNCS_N];
} list_priv = {
	.treemodelfilter = NULL,
	.filter = LIST_FILTER_ALL,
	.status_id = 0,
};

int list_get_store_rows(void)
{
	return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(gui.liststore), NULL);
}

static int list_get_filter_rows(void)
{
	return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_priv.treemodelfilter), NULL);
}

void list_refilter(void)
{
	gtk_tree_view_set_model(gui.treeview, NULL);
	gtk_tree_model_filter_refilter(list_priv.treemodelfilter);
	gtk_tree_view_set_model(gui.treeview, GTK_TREE_MODEL(list_priv.treemodelfilter));

	list_update();
	list_update_status();
}

void list_set_filter(const enum list_filter_e filter)
{
	g_assert(LIST_FILTER_IS_VALID(filter));

	if (filter != list_priv.filter) {
		list_priv.filter = filter;
		list_refilter();
	}
}

static void list_set_filter_label(GtkToggleButton *button, const char *label)
{
	gtk_button_set_label(GTK_BUTTON(button), label);

	// Prevent button resizing when the label width is reduced
	int request_width = 0, natural_width = 0;
	gtk_widget_get_size_request(GTK_WIDGET(button), &request_width, NULL);
	gtk_widget_get_preferred_width(GTK_WIDGET(button), NULL, &natural_width);
	if (natural_width > request_width)
		gtk_widget_set_size_request(GTK_WIDGET(button), natural_width, -1);
}

static bool list_row_is_error(GtkTreeModel *model, GtkTreeIter *iter)
{
	GValue error_value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, COL_ERROR, &error_value);
	const bool error = (g_value_get_string(&error_value) != NULL);
	g_value_unset(&error_value);

	return error;
}

static bool list_row_is_pending(GtkTreeModel *model, GtkTreeIter *iter)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		GValue digest_value = G_VALUE_INIT;
		gtk_tree_model_get_value(model, iter, list_priv.hash_cols[i], &digest_value);
		const bool pending = (g_value_get_string(&digest_value) == NULL);
		g_value_unset(&digest_value);

		if (pending)
			return true;
	}

	return false;
}

static bool list_row_is_match_or_mismatch(GtkTreeModel *model, GtkTreeIter *iter, bool *match)
{
	g_assert(match);

	GValue icon_value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, COL_ICON_NAME, &icon_value);
	const char *icon = g_value_get_string(&icon_value);

	bool found = false;

	if (g_strcmp0(icon, GUI_ICON_CHECK_MATCH) == 0) {
		found = true;
		*match = true;
	} else if (g_strcmp0(icon, GUI_ICON_CHECK_MISMATCH) == 0) {
		found = true;
		*match = false;
	}

	g_value_unset(&icon_value);

	return found;
}

static bool list_row_matches_search(GtkTreeModel *model, GtkTreeIter *iter)
{
	const char *search = gtk_entry_get_text(GTK_ENTRY(gui.searchentry));
	if (!*search)
		return true;

	// Search file
	GValue file_value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, COL_FILE, &file_value);
	const char *file = g_value_get_string(&file_value);
	bool match = g_str_match_string(search, file, false);
	g_value_unset(&file_value);

	if (match)
		return true;

	// Search for a full digest match
	for (int i = 0; (i < HASH_FUNCS_N) && !match; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		GValue digest_value = G_VALUE_INIT;
		gtk_tree_model_get_value(model, iter, list_priv.hash_cols[i], &digest_value);
		const char *digest = g_value_get_string(&digest_value);
		match = gtkhash_digest_format_compare(search, digest, gui_get_digest_format());
		g_value_unset(&digest_value);
	}

	return match;
}

void list_update_status_now(void)
{
	unsigned int all = 0, pending = 0, match = 0, mismatch = 0, error = 0;

	GtkTreeIter store_iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gui.liststore), &store_iter)) {
		do {
			all++;

			if (list_row_is_error(GTK_TREE_MODEL(gui.liststore), &store_iter)) {
				error++;
				continue;
			}

			if (list_row_is_pending(GTK_TREE_MODEL(gui.liststore), &store_iter))
				pending++;

			bool result;
			if (list_row_is_match_or_mismatch(GTK_TREE_MODEL(gui.liststore), &store_iter, &result)) {
				if (result)
					match++;
				else
					mismatch++;
			}
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(gui.liststore), &store_iter));
	}

	char *label;

	label = g_strdup_printf(_("All (%u)"), all);
	list_set_filter_label(gui.togglebutton_filter_all, label);
	g_free(label);

	label = g_strdup_printf(_("Pending (%u)"), pending);
	list_set_filter_label(gui.togglebutton_filter_pending, label);
	g_free(label);

	label = g_strdup_printf(_("Match (%u)"), match);
	list_set_filter_label(gui.togglebutton_filter_match, label);
	g_free(label);

	label = g_strdup_printf(_("Mismatch (%u)"), mismatch);
	list_set_filter_label(gui.togglebutton_filter_mismatch, label);
	g_free(label);

	label = g_strdup_printf(_("Error (%u)"), error);
	list_set_filter_label(gui.togglebutton_filter_error, label);
	g_free(label);
}

static gboolean list_update_status_idle(G_GNUC_UNUSED void *data)
{
	list_priv.status_id = 0;

	if (gui.state == GUI_STATE_BUSY)
		return G_SOURCE_REMOVE;

	list_update_status_now();

	return G_SOURCE_REMOVE;
}

void list_update_status(void)
{
	if (gui.state == GUI_STATE_BUSY)
		return;

	if (list_priv.status_id == 0)
		list_priv.status_id = g_idle_add_full(G_PRIORITY_HIGH_IDLE, list_update_status_idle, NULL, NULL);
}

static gboolean list_filter_func(GtkTreeModel *model, GtkTreeIter *iter, G_GNUC_UNUSED void *data)
{
	bool visible;

	switch (list_priv.filter) {
		case LIST_FILTER_PENDING:
			visible = !list_row_is_error(model, iter) && list_row_is_pending(model, iter);
			break;
		case LIST_FILTER_MATCH:
		case LIST_FILTER_MISMATCH: {
			if (list_row_is_error(model, iter)) {
				visible = false;
				break;
			}
			bool result;
			visible = list_row_is_match_or_mismatch(model, iter, &result) &&
				((list_priv.filter == LIST_FILTER_MATCH) ? result : !result);
			break;
		}
		case LIST_FILTER_ERROR:
			visible = list_row_is_error(model, iter);
			break;
		default:
			visible = true;
	}

	if (!visible)
		return false;

	return list_row_matches_search(model, iter);
}

static void list_init_filter(void)
{
	g_assert(!list_priv.treemodelfilter);

	list_priv.treemodelfilter = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(
		GTK_TREE_MODEL(gui.liststore), NULL));
	gtk_tree_model_filter_set_visible_func(list_priv.treemodelfilter, list_filter_func, NULL, NULL);
}

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
	types[COL_FILE]      = G_TYPE_STRING;
	types[COL_CHECK]     = G_TYPE_STRING;
	types[COL_ERROR]     = G_TYPE_STRING;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		types[list_priv.hash_cols[i]] = G_TYPE_STRING;
	}

	gui.liststore = gtk_list_store_newv(cols, types);

	list_init_filter();

	gtk_tree_view_set_model(gui.treeview, GTK_TREE_MODEL(list_priv.treemodelfilter));
	gtk_tree_view_set_search_entry(gui.treeview, GTK_ENTRY(gui.searchentry));

	const GtkTargetEntry targets[] = {
		{ (char *)"text/uri-list", 0, 0 }
	};
	gtk_drag_dest_set(GTK_WIDGET(gui.treeview), GTK_DEST_DEFAULT_ALL, targets,
		G_N_ELEMENTS(targets), GDK_ACTION_COPY);

	list_update_status_now();
}

void list_deinit(void)
{
	if (list_priv.status_id) {
		g_source_remove(list_priv.status_id);
		list_priv.status_id = 0;
	}
}

void list_update(void)
{
	bool func_enabled = false;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;
		if (hash.funcs[i].enabled)
			func_enabled = true;

		gtk_tree_view_column_set_visible(list_priv.view_cols[i], hash.funcs[i].enabled);
	}

	const bool store_rows = (list_get_store_rows() > 0);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_clear), store_rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_clear), store_rows);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), func_enabled && store_rows &&
		(list_get_filter_rows() > 0));
}

void list_append_row(const char * const uri, const char * const check, const char * const icon,
	const char * const error)
{
	g_assert(uri);

	GFile *file = g_file_new_for_uri(uri);
	char *pname = g_file_get_parse_name(file);

	gtk_list_store_insert_with_values(gui.liststore, NULL, -1,
		COL_ICON_NAME, icon,
		COL_FILE, pname,
		COL_CHECK, check,
		COL_ERROR, error,
		-1);

	g_free(pname);
	g_object_unref(file);
}

static GPtrArray *list_get_selected_iters(void)
{
	GPtrArray *iters = g_ptr_array_new_with_free_func(g_free);

	GList *paths = gtk_tree_selection_get_selected_rows(gui.treeselection, NULL);

	for (GList *i = paths; i != NULL; i = i->next) {
		GtkTreeIter filter_iter;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(list_priv.treemodelfilter), &filter_iter, i->data)) {
			GtkTreeIter store_iter;
			gtk_tree_model_filter_convert_iter_to_child_iter(list_priv.treemodelfilter,
				&store_iter, &filter_iter);
			g_ptr_array_add(iters, g_memdup2(&store_iter, sizeof(store_iter)));
		}
		gtk_tree_path_free(i->data);
	}

	g_list_free(paths);

	return iters;
}

void list_remove_selection(void)
{
	GPtrArray *iters = list_get_selected_iters();

	if (!iters->len) {
		g_ptr_array_free(iters, true);
		return;
	}

	if (iters->len == (unsigned int)list_get_store_rows()) {
		g_ptr_array_free(iters, true);
		list_clear();
		return;
	}

	for (unsigned int i = 0; i < iters->len; i++)
		gtk_list_store_remove(gui.liststore, g_ptr_array_index(iters, i));

	g_ptr_array_free(iters, true);

	list_update();
}

GPtrArray *list_get_store_iters(void)
{
	GPtrArray *iters = g_ptr_array_new_with_free_func(g_free);

	GtkTreeIter store_iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gui.liststore), &store_iter)) {
		do {
			g_ptr_array_add(iters, g_memdup2(&store_iter, sizeof(store_iter)));
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(gui.liststore), &store_iter));
	}

	return iters;
}

GPtrArray *list_get_visible_iters(void)
{
	GPtrArray *iters = g_ptr_array_new_with_free_func(g_free);

	GtkTreeIter filter_iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list_priv.treemodelfilter), &filter_iter)) {
		do {
			GtkTreeIter store_iter;
			gtk_tree_model_filter_convert_iter_to_child_iter(list_priv.treemodelfilter,
				&store_iter, &filter_iter);
			g_ptr_array_add(iters, g_memdup2(&store_iter, sizeof(store_iter)));
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list_priv.treemodelfilter), &filter_iter));
	}

	return iters;
}

static GFile *list_get_iter_file(GtkTreeIter *store_iter)
{
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter, COL_FILE, &value);
	GFile *file = g_file_parse_name(g_value_get_string(&value));
	g_value_unset(&value);

	return file;
}

char *list_get_uri(GtkTreeIter *store_iter)
{
	GFile *file = list_get_iter_file(store_iter);
	char *uri = g_file_get_uri(file);
	g_object_unref(file);

	return uri;
}

char *list_get_basename(GtkTreeIter *store_iter)
{
	GFile *file = list_get_iter_file(store_iter);

	char *basename = g_file_get_basename(file);
	g_object_unref(file);

	return basename;
}

// Returns a static string or NULL
static const char *list_get_icon(const char * const check, const char * const error,
	const char * const *digests, const char * const old_icon)
{
	if (error && *error)
		return old_icon;

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
		return match ? GUI_ICON_CHECK_MATCH : GUI_ICON_CHECK_MISMATCH;

	return NULL;
}

void list_set_digests(GtkTreeIter *store_iter, const char * const *digests)
{
	int cols[HASH_FUNCS_N + 2];
	GValue vals[HASH_FUNCS_N + 2] = { G_VALUE_INIT };

	GValue check_value = G_VALUE_INIT;
	gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter, COL_CHECK, &check_value);
	const char *check = g_value_get_string(&check_value);

	cols[0] = COL_ICON_NAME;
	g_value_init(&vals[0], G_TYPE_STRING);
	g_value_set_static_string(&vals[0], list_get_icon(check, NULL, digests, NULL));

	g_value_unset(&check_value);

	cols[1] = COL_ERROR;
	g_value_init(&vals[1], G_TYPE_STRING);
	g_value_set_string(&vals[1], NULL);

	int vals_n = 2;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!digests[i])
			continue;

		cols[vals_n] = list_priv.hash_cols[i];
		g_value_init(&vals[vals_n], G_TYPE_STRING);
		g_value_set_string(&vals[vals_n], digests[i]);
		vals_n++;
	}

	gtk_list_store_set_valuesv(gui.liststore, store_iter, cols, vals, vals_n);

	for (int i = 0; i < vals_n; i++)
		g_value_unset(&vals[i]);
}

void list_set_error(GtkTreeIter *store_iter, const char * const message, const char * const icon)
{
	g_assert(message);
	g_assert(icon);

	gtk_list_store_set(gui.liststore, store_iter,
		COL_ERROR, message,
		COL_ICON_NAME, icon,
		-1);
}

char *list_get_error(GtkTreeIter *filter_iter)
{
	GtkTreeIter store_iter;
	gtk_tree_model_filter_convert_iter_to_child_iter(list_priv.treemodelfilter,
		&store_iter, filter_iter);

	char *error;
	gtk_tree_model_get(GTK_TREE_MODEL(gui.liststore), &store_iter,
		COL_ERROR, &error, -1);

	return error;
}

char *list_get_digest(GtkTreeIter *store_iter, const enum hash_func_e id)
{
	g_assert(HASH_FUNC_IS_VALID(id));

	char *digest;
	gtk_tree_model_get(GTK_TREE_MODEL(gui.liststore), store_iter,
		list_priv.hash_cols[id], &digest, -1);

	return digest;
}

char *list_get_selected_digest(const enum hash_func_e id)
{
	GList *rows = gtk_tree_selection_get_selected_rows(gui.treeselection, NULL);

	// Should only have one row selected
	g_assert(rows);
	g_assert(!rows->next);

	GtkTreeIter filter_iter;
	char *digest = NULL;

	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(list_priv.treemodelfilter), &filter_iter, rows->data)) {
		gtk_tree_model_get(GTK_TREE_MODEL(list_priv.treemodelfilter), &filter_iter,
			list_priv.hash_cols[id], &digest, -1);
	}

	gtk_tree_path_free(rows->data);
	g_list_free(rows);

	return digest;
}

static void list_check_digests(GtkTreeIter *store_iter)
{
	GValue icon_value = G_VALUE_INIT;
	GValue check_value = G_VALUE_INIT;
	GValue error_value = G_VALUE_INIT;

	gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter, COL_ICON_NAME, &icon_value);
	gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter, COL_CHECK, &check_value);
	gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter, COL_ERROR, &error_value);

	const char *old_icon = g_value_get_string(&icon_value);
	const char *check = g_value_get_string(&check_value);
	const char *error = g_value_get_string(&error_value);

	GValue values[HASH_FUNCS_N] = { G_VALUE_INIT };
	const char *digests[HASH_FUNCS_N] = { NULL };

	if (check && *check) {
		for (int i = 0; i < HASH_FUNCS_N; i++) {
			if (!hash.funcs[i].enabled)
				continue;

			gtk_tree_model_get_value(GTK_TREE_MODEL(gui.liststore), store_iter,
				list_priv.hash_cols[i], &values[i]);
			digests[i] = g_value_get_string(&values[i]);
		}
	}

	const char *new_icon = list_get_icon(check, error, (const char * const *)digests, old_icon);

	for (int i = 0; i < HASH_FUNCS_N; i++)
		g_value_unset(&values[i]);

	g_value_unset(&check_value);
	g_value_unset(&error_value);

	if (g_strcmp0(old_icon, new_icon) != 0)
		gtk_list_store_set(gui.liststore, store_iter, COL_ICON_NAME, new_icon, -1);

	g_value_unset(&icon_value);
}

static void list_suspend_signal(const char * const name)
{
	const unsigned int signal = g_signal_lookup(name, GTK_TYPE_TREE_MODEL);

	g_signal_handlers_block_matched(GTK_TREE_MODEL(gui.liststore),
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, signal, 0, NULL, NULL, list_priv.treemodelfilter);
	g_signal_handlers_block_matched(GTK_TREE_MODEL(list_priv.treemodelfilter),
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, signal, 0, NULL, NULL, gui.treeview);
}

static void list_restore_signal(const char * const name)
{
	const unsigned int signal = g_signal_lookup(name, GTK_TYPE_TREE_MODEL);

	g_signal_handlers_unblock_matched(GTK_TREE_MODEL(list_priv.treemodelfilter),
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, signal, 0, NULL, NULL, gui.treeview);
	g_signal_handlers_unblock_matched(GTK_TREE_MODEL(gui.liststore),
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA, signal, 0, NULL, NULL, list_priv.treemodelfilter);

	list_refilter();
	gtk_widget_queue_draw(GTK_WIDGET(gui.treeview));
}

void list_check_all_digests(void)
{
	list_suspend_signal("row-changed");

	GPtrArray *iters = list_get_store_iters();

	for (unsigned int i = 0; i < iters->len; i++)
		list_check_digests(g_ptr_array_index(iters, i));

	g_ptr_array_free(iters, true);

	list_restore_signal("row-changed");
}

static void list_clear_digests_iters(GPtrArray *iters)
{
	int cols[HASH_FUNCS_N + 2];
	GValue vals[HASH_FUNCS_N + 2] = { G_VALUE_INIT };

	cols[0] = COL_ICON_NAME;
	g_value_init(&vals[0], G_TYPE_STRING);
	g_value_set_string(&vals[0], NULL);

	cols[1] = COL_ERROR;
	g_value_init(&vals[1], G_TYPE_STRING);
	g_value_set_string(&vals[1], NULL);

	int vals_n = 2;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		cols[vals_n] = list_priv.hash_cols[i];
		g_value_init(&vals[vals_n], G_TYPE_STRING);
		g_value_set_string(&vals[vals_n], NULL);
		vals_n++;
	}

	list_suspend_signal("row-changed");

	for (unsigned int i = 0; i < iters->len; i++)
		gtk_list_store_set_valuesv(gui.liststore, g_ptr_array_index(iters, i), cols, vals, vals_n);

	for (int i = 0; i < vals_n; i++)
		g_value_unset(&vals[i]);

	list_restore_signal("row-changed");
}

void list_clear_digests(void)
{
	GPtrArray *iters = list_get_store_iters();

	list_clear_digests_iters(iters);

	g_ptr_array_free(iters, true);
}

void list_clear_digests_func(const enum hash_func_e id)
{
	g_assert(HASH_FUNC_IS_VALID(id));

	list_suspend_signal("row-changed");

	GPtrArray *iters = list_get_store_iters();

	for (unsigned int i = 0; i < iters->len; i++) {
		gtk_list_store_set(gui.liststore, g_ptr_array_index(iters, i),
			list_priv.hash_cols[id], NULL, -1);
		list_check_digests(g_ptr_array_index(iters, i));
	}

	g_ptr_array_free(iters, true);

	list_restore_signal("row-changed");
}

void list_clear(void)
{
	gtk_tree_view_set_model(gui.treeview, NULL);
	g_clear_object(&list_priv.treemodelfilter);

	gtk_list_store_clear(gui.liststore);

	list_init_filter();
	gtk_tree_view_set_model(gui.treeview, GTK_TREE_MODEL(list_priv.treemodelfilter));

	list_update();
}
