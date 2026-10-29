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
#include <string.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-list.h"
#include "properties-hash.h"
#include "properties-prefs.h"
#include "../../hash/hash-func.h"
#include "../../hash/hash-file.h"
#include "../../util/util.h"

#define PROPERTIES_XML_RESOURCE "/org/gtkhash/plugin/properties.ui"

static GObject *gtkhash_properties_get_object(GtkBuilder *builder, const char *name)
{
	GObject *obj = gtk_builder_get_object(builder, name);
	if (!obj)
		g_warning("unknown object: \"%s\"", name);

	return obj;
}

static void gtkhash_properties_busy(struct page_s *page)
{
	page->busy = true;

	gtk_widget_set_sensitive(GTK_WIDGET(page->treeview), false);
	gtk_widget_set_sensitive(GTK_WIDGET(page->hbox_hmac), false);

	// Reset progress bar
	gtk_progress_bar_set_fraction(page->progressbar, 0.0);
	gtk_progress_bar_set_text(page->progressbar, " ");
	gtk_widget_show(GTK_WIDGET(page->progressbar));

	// Hash button
	gtk_widget_hide(GTK_WIDGET(page->button_hash));
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_hash), false);

	// Stop button
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), true);
	gtk_widget_show(GTK_WIDGET(page->button_stop));
}

static void gtkhash_properties_button_hash_set_sensitive(struct page_s *page)
{
	const bool enabled = gtkhash_hash_func_count_enabled(page->funcs);

	gtk_widget_set_sensitive(GTK_WIDGET(page->button_hash), enabled);
}

static void gtkhash_properties_entry_hmac_set_sensitive(struct page_s *page)
{
	const bool active = gtk_toggle_button_get_active(page->togglebutton_hmac);

	gtk_widget_set_sensitive(GTK_WIDGET(page->entry_hmac), active);
}

void gtkhash_properties_idle(struct page_s *page)
{
	page->busy = false;

	gtk_widget_hide(GTK_WIDGET(page->progressbar));

	// Stop button
	gtk_widget_hide(GTK_WIDGET(page->button_stop));
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), false);

	// Hash button
	gtkhash_properties_button_hash_set_sensitive(page);
	gtk_widget_show(GTK_WIDGET(page->button_hash));

	gtk_widget_set_sensitive(GTK_WIDGET(page->treeview), true);
	gtk_widget_set_sensitive(GTK_WIDGET(page->hbox_hmac), true);
	gtkhash_properties_entry_hmac_set_sensitive(page);

	gtkhash_properties_list_check_digests(page);
}

static void gtkhash_properties_on_cell_toggled(struct page_s *page,
	char *path_str)
{
	gtkhash_properties_list_update_enabled(page, path_str);
	gtkhash_properties_list_check_digests(page);
	gtkhash_properties_button_hash_set_sensitive(page);
}

static void gtkhash_properties_on_treeview_popup_menu(struct page_s *page)
{
	/* Note: Shift+F10 can trigger this, so it's possible for the pointer
	   to be outside the window */

	gtk_menu_popup_at_pointer(page->menu, NULL);
}

static bool gtkhash_properties_on_treeview_button_press_event(
	struct page_s *page, GdkEventButton *event)
{
	if (gdk_event_triggers_context_menu((GdkEvent *)event))
		gtk_menu_popup_at_pointer(page->menu, (GdkEvent *)event);

	return false;
}

static void gtkhash_properties_on_treeview_row_activated(
	struct page_s *page, GtkTreePath *path, GtkTreeViewColumn *column,
	G_GNUC_UNUSED GtkTreeView *treeview)
{
	// Ignore checkbutton column
	if (!*gtk_tree_view_column_get_title(column))
		return;

	if (!gtk_tree_selection_path_is_selected(page->treeselection, path))
		return;

	if (gtkhash_properties_list_hash_selected(page))
		gtkhash_properties_busy(page);
}

static void gtkhash_properties_on_menu_map_event(struct page_s *page)
{
	bool sensitive = false;
	char *digest = gtkhash_properties_list_get_selected_digest(page);
	if (digest) {
		sensitive = true;
		g_free(digest);
	}

	gtk_widget_set_sensitive(GTK_WIDGET(page->menuitem_copy), sensitive);
}

static void gtkhash_properties_on_menuitem_copy_activate(struct page_s *page)
{
	GtkClipboard *clipboard = gtk_clipboard_get(GDK_NONE);
	char *digest = gtkhash_properties_list_get_selected_digest(page);

	gtk_clipboard_set_text(clipboard, digest, -1);

	g_free(digest);
}

static void gtkhash_properties_on_menuitem_show_funcs_toggled(struct page_s *page)
{
	gtkhash_properties_list_refilter(page);
}

static void gtkhash_properties_on_entry_check_insert_text(GtkEditable *editable, char *new_text,
	G_GNUC_UNUSED int new_text_length, int *position, G_GNUC_UNUSED void *data)
{
	const size_t len = strlen(new_text);
	char *filtered = gtkhash_filter_digest_chars(new_text, len);

	if (strlen(filtered) == len) {
		g_free(filtered);
		return;
	}

	gtk_editable_insert_text(editable, filtered, -1, position);

	g_free(filtered);

	g_signal_stop_emission_by_name(editable, "insert-text");
}

static void gtkhash_properties_on_entry_check_changed(struct page_s *page)
{
	gtkhash_properties_list_check_digests(page);
}

static void gtkhash_properties_on_entry_check_icon_press(GtkEntry *entry,
	GtkEntryIconPosition pos, GdkEventButton *event)
{
	if (pos != GTK_ENTRY_ICON_PRIMARY)
		return;
	if (event->type != GDK_BUTTON_PRESS)
		return;
	if (event->button != 1)
		return;

	gtk_entry_set_text(entry, "");
	gtk_editable_paste_clipboard(GTK_EDITABLE(entry));
}

static void gtkhash_properties_on_entry_hmac_changed(struct page_s *page)
{
	gtkhash_properties_list_clear_digests(page);
	gtkhash_properties_list_check_digests(page);
}

static void gtkhash_properties_on_menuitem_show_hmac_key_toggled(
	GtkCheckMenuItem *item, GtkEntry *entry)
{
	const bool active = gtk_check_menu_item_get_active(item);

	gtk_entry_set_visibility(entry, active);
}

static void gtkhash_properties_on_entry_hmac_populate_popup(GtkEntry *entry,
	GtkMenu *menu)
{
	GtkWidget *item;

	// Add separator
	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_check_menu_item_new_with_mnemonic(_("_Show HMAC Key"));
	// Add checkbutton
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
		gtk_entry_get_visibility(entry));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(item, "toggled", G_CALLBACK(
		gtkhash_properties_on_menuitem_show_hmac_key_toggled), entry);
}

static void gtkhash_properties_on_togglebutton_hmac_toggled(struct page_s *page)
{
	gtkhash_properties_entry_hmac_set_sensitive(page);
	gtkhash_properties_list_update_hash_func_names(page);
	gtkhash_properties_list_check_digests(page);
}

static void gtkhash_properties_on_button_hash_clicked(struct page_s *page)
{
	gtkhash_properties_busy(page);
	gtkhash_properties_list_clear_digests(page);

	if (gtk_toggle_button_get_active(page->togglebutton_hmac)) {
		const uint8_t *hmac_key = (uint8_t *)gtk_entry_get_text(
			page->entry_hmac);
		GtkEntryBuffer *buffer = gtk_entry_get_buffer(page->entry_hmac);
		const size_t key_size = gtk_entry_buffer_get_bytes(buffer);
		gtkhash_properties_hash_start(page, NULL, hmac_key, key_size);
	} else
		gtkhash_properties_hash_start(page, NULL, NULL, 0);
}

static void gtkhash_properties_on_button_stop_clicked(struct page_s *page)
{
	gtk_widget_set_sensitive(GTK_WIDGET(page->button_stop), false);
	gtkhash_properties_hash_stop(page);
}

static void gtkhash_properties_free_page(struct page_s *page)
{
	gtkhash_properties_hash_stop(page);

	while (page->busy)
		gtk_main_iteration();

	gtkhash_properties_prefs_deinit(page);
	gtkhash_properties_hash_deinit(page);
	g_free(page->uri);
	g_object_unref(page->menu);
	g_object_unref(page->box);
	g_free(page);
}

static void gtkhash_properties_init_objects(struct page_s *page,
	GtkBuilder *builder)
{
	// Main container
	page->box = GTK_WIDGET(gtkhash_properties_get_object(builder,
		"vbox"));
	g_object_ref(page->box);

	// Progress bar
	page->progressbar = GTK_PROGRESS_BAR(gtkhash_properties_get_object(builder,
		"progressbar"));

	// Treeview
	page->treeview = GTK_TREE_VIEW(gtkhash_properties_get_object(builder,
		"treeview"));
	page->treeselection = GTK_TREE_SELECTION(gtkhash_properties_get_object(builder,
		"treeselection"));
	page->cellrendtoggle = GTK_CELL_RENDERER_TOGGLE(gtkhash_properties_get_object(builder,
		"cellrenderertoggle"));

	// Popup menu
	page->menu = GTK_MENU(gtkhash_properties_get_object(builder,
		"menu"));
	g_object_ref(page->menu);
	page->menuitem_copy = GTK_MENU_ITEM(gtkhash_properties_get_object(builder,
		"menuitem_copy"));
	page->menuitem_show_funcs = GTK_CHECK_MENU_ITEM(gtkhash_properties_get_object(builder,
		"checkmenuitem_show_funcs"));

	// Check/MAC inputs
	page->hbox_hmac = GTK_WIDGET(gtkhash_properties_get_object(builder,
		"hbox_hmac"));
	page->entry_check = GTK_ENTRY(gtkhash_properties_get_object(builder,
		"entry_check"));
	page->togglebutton_hmac = GTK_TOGGLE_BUTTON(gtkhash_properties_get_object(builder,
		"togglebutton_hmac"));
	page->entry_hmac = GTK_ENTRY(gtkhash_properties_get_object(builder,
		"entry_hmac"));

	// Buttons
	page->button_hash = GTK_BUTTON(gtkhash_properties_get_object(builder,
		"button_hash"));
	page->button_stop = GTK_BUTTON(gtkhash_properties_get_object(builder,
		"button_stop"));
}

static void gtkhash_properties_connect_signals(struct page_s *page)
{
	// Main container
	g_signal_connect_swapped(page->box, "realize",
		G_CALLBACK(gtkhash_properties_on_button_hash_clicked), page);
	g_signal_connect_swapped(page->box, "destroy",
		G_CALLBACK(gtkhash_properties_free_page), page);

	// Treeview
	g_signal_connect_swapped(page->cellrendtoggle, "toggled",
		G_CALLBACK(gtkhash_properties_on_cell_toggled), page);
	g_signal_connect_swapped(page->treeview, "popup-menu",
		G_CALLBACK(gtkhash_properties_on_treeview_popup_menu), page);
	g_signal_connect_swapped(page->treeview, "button-press-event",
		G_CALLBACK(gtkhash_properties_on_treeview_button_press_event), page);
	g_signal_connect_swapped(page->treeview, "row-activated",
		G_CALLBACK(gtkhash_properties_on_treeview_row_activated), page);

	// Popup menu
	g_signal_connect_swapped(page->menu, "map-event",
		G_CALLBACK(gtkhash_properties_on_menu_map_event), page);
	g_signal_connect_swapped(page->menuitem_copy, "activate",
		G_CALLBACK(gtkhash_properties_on_menuitem_copy_activate), page);
	g_signal_connect_swapped(page->menuitem_show_funcs, "toggled",
		G_CALLBACK(gtkhash_properties_on_menuitem_show_funcs_toggled), page);

	// Check
	g_signal_connect(page->entry_check, "insert-text",
		G_CALLBACK(gtkhash_properties_on_entry_check_insert_text), NULL);
	g_signal_connect_swapped(page->entry_check, "changed",
		G_CALLBACK(gtkhash_properties_on_entry_check_changed), page);
	g_signal_connect(page->entry_check, "icon-press",
		G_CALLBACK(gtkhash_properties_on_entry_check_icon_press), NULL);

	// HMAC
	g_signal_connect_swapped(page->togglebutton_hmac, "toggled",
		G_CALLBACK(gtkhash_properties_on_togglebutton_hmac_toggled), page);
	g_signal_connect_swapped(page->entry_hmac, "changed",
		G_CALLBACK(gtkhash_properties_on_entry_hmac_changed), page);
	g_signal_connect(page->entry_hmac, "populate-popup",
		G_CALLBACK(gtkhash_properties_on_entry_hmac_populate_popup), NULL);

	// Buttons
	g_signal_connect_swapped(page->button_hash, "clicked",
		G_CALLBACK(gtkhash_properties_on_button_hash_clicked), page);
	g_signal_connect_swapped(page->button_stop, "clicked",
		G_CALLBACK(gtkhash_properties_on_button_stop_clicked), page);
}

struct page_s *gtkhash_properties_new_page(char *uri)
{
	GtkBuilder *builder = gtk_builder_new();
	GError *error = NULL;
	if (gtk_builder_add_from_resource(builder, PROPERTIES_XML_RESOURCE, &error) == 0) {
		g_warning("failed to load resource: %s", error->message);
		g_error_free(error);
		g_object_unref(builder);
		g_free(uri);

		return NULL;
	}

	struct page_s *page = g_new(struct page_s, 1);
	page->uri = uri;

	gtkhash_properties_hash_init(page);

	if (!gtkhash_hash_func_count_supported(page->funcs)) {
		g_warning("no hash functions available");
		gtkhash_properties_hash_deinit(page);
		g_free(page->uri);
		g_free(page);
		g_object_unref(builder);

		return NULL;
	}

	gtkhash_properties_init_objects(page, builder);
	g_object_unref(builder);

	gtkhash_properties_prefs_init(page);
	gtkhash_properties_list_init(page);
	gtkhash_properties_idle(page);

	gtkhash_properties_connect_signals(page);

	return page;
}
