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

#if defined(IN_NAUTILUS_EXTENSION)
	#if HAVE_NAUTILUS_EXTENSION_H
		#include <nautilus-extension.h>
	#else
		#include <libnautilus-extension/nautilus-property-page.h>
		#include <libnautilus-extension/nautilus-property-page-provider.h>
	#endif
#elif defined(IN_CAJA_EXTENSION)
	#include <libcaja-extension/caja-property-page.h>
	#include <libcaja-extension/caja-property-page-provider.h>
#elif defined(IN_NEMO_EXTENSION)
	#include <libnemo-extension/nemo-property-page.h>
	#include <libnemo-extension/nemo-property-page-provider.h>
	#include <libnemo-extension/nemo-name-and-desc-provider.h>
#elif defined(IN_THUNAR_EXTENSION)
	#undef GTK_DISABLE_DEPRECATED // thunarx-3 doesn't build with this
	#include <thunarx/thunarx.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-list.h"
#include "properties-hash.h"
#include "properties-prefs.h"
#include "../hash/hash-func.h"
#include "../hash/hash-file.h"

#define PROPERTIES_XML_RESOURCE "/org/gtkhash/plugin/gtkhash-properties.ui"

static GType page_type;

void gtkhash_properties_on_button_hash_clicked(struct page_s *page);

static GObject *gtkhash_properties_get_object(GtkBuilder *builder,
	const char *name)
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
	gtk_widget_set_sensitive(GTK_WIDGET(page->hbox_inputs), false);

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
	bool has_enabled = false;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (page->funcs[i].enabled) {
			has_enabled = true;
			break;
		}
	}

	gtk_widget_set_sensitive(GTK_WIDGET(page->button_hash), has_enabled);
}

static void gtkhash_properties_entry_hmac_set_sensitive(struct page_s *page)
{
	bool active = gtk_toggle_button_get_active(page->togglebutton_hmac);

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
	gtk_widget_set_sensitive(GTK_WIDGET(page->hbox_inputs), true);
	gtkhash_properties_entry_hmac_set_sensitive(page);

	gtkhash_properties_list_check_digests(page);
}

static void gtkhash_properties_on_cell_toggled(struct page_s *page,
	char *path_str)
{
	bool enabled = gtkhash_properties_list_update_enabled(page, path_str);
	gtkhash_properties_list_check_digests(page);
	gtkhash_properties_button_hash_set_sensitive(page);

 	if (enabled)
		gtkhash_properties_on_button_hash_clicked(page);
}

static void gtkhash_properties_on_treeview_popup_menu(struct page_s *page)
{
	/* Note: Shift+F10 can trigger this, so it's possible for the pointer
	   to be outside the window */

#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(page->menu, NULL);
#else
	gtk_menu_popup(page->menu, NULL, NULL, NULL, NULL, 0,
		gtk_get_current_event_time());
#endif
}

static bool gtkhash_properties_on_treeview_button_press_event(
	struct page_s *page, GdkEventButton *event)
{
	if (gdk_event_triggers_context_menu((GdkEvent *)event))
#if GTK_CHECK_VERSION(3,22,0)
		gtk_menu_popup_at_pointer(page->menu, (GdkEvent *)event);
#else
		gtk_menu_popup(page->menu, NULL, NULL, NULL, NULL, event->button,
			gdk_event_get_time((GdkEvent *)event));
#endif

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

static void gtkhash_properties_on_menuitem_show_funcs_toggled(
	struct page_s *page)
{
	gtkhash_properties_list_refilter(page);
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

void gtkhash_properties_on_button_hash_clicked(struct page_s *page)
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
		"imagemenuitem_copy"));
	page->menuitem_show_funcs = GTK_CHECK_MENU_ITEM(gtkhash_properties_get_object(builder,
		"checkmenuitem_show_funcs"));

	// Check/MAC inputs
	page->hbox_inputs = GTK_WIDGET(gtkhash_properties_get_object(builder,
		"hbox_inputs"));
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

static struct page_s *gtkhash_properties_new_page(char *uri)
{
	GtkBuilder *builder = gtk_builder_new_from_resource(PROPERTIES_XML_RESOURCE);
	if (!builder)
		return NULL;

	struct page_s *page = g_new(struct page_s, 1);
	page->uri = uri;

	gtkhash_properties_hash_init(page);

	if (gtkhash_properties_hash_funcs_supported(page) == 0) {
		g_warning("no hash functions available");
		gtkhash_properties_hash_deinit(page);
		g_free(page);

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

static GList *gtkhash_properties_get_pages(
#if defined(IN_NAUTILUS_EXTENSION)
	G_GNUC_UNUSED NautilusPropertyPageProvider *provider,
#elif defined(IN_CAJA_EXTENSION)
	G_GNUC_UNUSED CajaPropertyPageProvider *provider,
#elif defined(IN_NEMO_EXTENSION)
	G_GNUC_UNUSED NemoPropertyPageProvider *provider,
#elif defined(IN_THUNAR_EXTENSION)
	G_GNUC_UNUSED ThunarxPropertyPageProvider *provider,
#endif
	GList *files)
{
	// Only display page for a single file
	if (!files || files->next)
		return NULL;

#if defined(IN_NAUTILUS_EXTENSION)
	GFileType type = nautilus_file_info_get_file_type(files->data);
	char *uri = nautilus_file_info_get_uri(files->data);
#elif defined(IN_CAJA_EXTENSION)
	GFileType type = caja_file_info_get_file_type(files->data);
	char *uri = caja_file_info_get_uri(files->data);
#elif defined(IN_NEMO_EXTENSION)
	GFileType type = nemo_file_info_get_file_type(files->data);
	char *uri = nemo_file_info_get_uri(files->data);
#elif defined(IN_THUNAR_EXTENSION)
	GFileInfo *info = thunarx_file_info_get_file_info(files->data);
	GFileType type = g_file_info_get_file_type(info);
	g_object_unref(info);

	char *uri = thunarx_file_info_get_uri(files->data);
#endif

	// Only display page for regular files
	if (type != G_FILE_TYPE_REGULAR)
		return NULL;

	struct page_s *page = gtkhash_properties_new_page(uri);
	if (!page)
		return NULL;

#if defined(IN_NAUTILUS_EXTENSION)
	NautilusPropertyPage *ppage = nautilus_property_page_new(
		"GtkHash::properties", gtk_label_new(_("Checksums")), page->box);
#elif defined(IN_CAJA_EXTENSION)
	CajaPropertyPage *ppage = caja_property_page_new(
		"GtkHash::properties", gtk_label_new(_("Checksums")), page->box);
#elif defined(IN_NEMO_EXTENSION)
	NemoPropertyPage *ppage = nemo_property_page_new(
		"GtkHash::properties", gtk_label_new(_("Checksums")), page->box);
#elif defined(IN_THUNAR_EXTENSION)
	GtkWidget *ppage = thunarx_property_page_new(_("Checksums"));
	gtk_container_add(GTK_CONTAINER(ppage), page->box);
#endif

	GList *pages = g_list_append(NULL, ppage);

	return pages;
}

static void gtkhash_properties_pp_iface_init(
#if defined(IN_NAUTILUS_EXTENSION)
	NautilusPropertyPageProviderIface *iface,
#elif defined(IN_CAJA_EXTENSION)
	CajaPropertyPageProviderIface *iface,
#elif defined(IN_NEMO_EXTENSION)
	NemoPropertyPageProviderIface *iface,
#elif defined(IN_THUNAR_EXTENSION)
	ThunarxPropertyPageProviderIface *iface,
#endif
	G_GNUC_UNUSED void *data)
{
	iface->get_pages = gtkhash_properties_get_pages;
}

#ifdef IN_NEMO_EXTENSION
static char *gtkhash_properties_nd_string = NULL;

static GList *gtkhash_properties_get_name_desc(
	G_GNUC_UNUSED NemoNameAndDescProvider *provider)
{
	return g_list_append(NULL, gtkhash_properties_nd_string);
}

static void gtkhash_properties_nd_iface_init(NemoNameAndDescProviderIface *iface,
	G_GNUC_UNUSED void *data)
{
	iface->get_name_and_desc = gtkhash_properties_get_name_desc;
}
#endif

static void gtkhash_properties_register_type(GTypeModule *module)
{
	const GTypeInfo info = {
		sizeof(GObjectClass),
		(GBaseInitFunc)NULL,
		(GBaseFinalizeFunc)NULL,
		(GClassInitFunc)NULL,
		(GClassFinalizeFunc)NULL,
		NULL,
		sizeof(GObject),
		0,
		(GInstanceInitFunc)NULL,
		(GTypeValueTable *)NULL
	};

	page_type = g_type_module_register_type(module, G_TYPE_OBJECT,
		"GtkHash", &info, 0);

	const GInterfaceInfo pp_iface_info = {
		(GInterfaceInitFunc)gtkhash_properties_pp_iface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, page_type,
#if defined(IN_NAUTILUS_EXTENSION)
		NAUTILUS_TYPE_PROPERTY_PAGE_PROVIDER,
#elif defined(IN_CAJA_EXTENSION)
		CAJA_TYPE_PROPERTY_PAGE_PROVIDER,
#elif defined(IN_NEMO_EXTENSION)
		NEMO_TYPE_PROPERTY_PAGE_PROVIDER,
#elif defined(IN_THUNAR_EXTENSION)
		THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
#endif
		&pp_iface_info);

#ifdef IN_NEMO_EXTENSION
	const GInterfaceInfo nd_iface_info = {
		(GInterfaceInitFunc)gtkhash_properties_nd_iface_init,
		(GInterfaceFinalizeFunc)NULL,
		NULL
	};

	g_type_module_add_interface(module, page_type,
		NEMO_TYPE_NAME_AND_DESC_PROVIDER, &nd_iface_info);
#endif
}

#if __GNUC__
	#define PUBLIC __attribute__((visibility("default")))
#else
	#define PUBLIC G_MODULE_EXPORT
#endif

#if defined(IN_NAUTILUS_EXTENSION)
PUBLIC void nautilus_module_initialize(GTypeModule *module)
#elif defined(IN_CAJA_EXTENSION)
PUBLIC void caja_module_initialize(GTypeModule *module)
#elif defined(IN_NEMO_EXTENSION)
PUBLIC void nemo_module_initialize(GTypeModule *module)
#elif defined(IN_THUNAR_EXTENSION)
PUBLIC void thunar_extension_initialize(GTypeModule *module);
PUBLIC void thunar_extension_initialize(GTypeModule *module)
#endif
{
	gtkhash_properties_register_type(module);

#if ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

#ifdef IN_NEMO_EXTENSION
	gtkhash_properties_nd_string = g_strdup_printf("GtkHash:::%s",
		_("Calculate message digests or checksums"));
#endif
}

#if defined(IN_NAUTILUS_EXTENSION)
PUBLIC void nautilus_module_shutdown(void)
#elif defined(IN_CAJA_EXTENSION)
PUBLIC void caja_module_shutdown(void)
#elif defined(IN_NEMO_EXTENSION)
PUBLIC void nemo_module_shutdown(void)
#elif defined(IN_THUNAR_EXTENSION)
PUBLIC void thunar_extension_shutdown(void);
PUBLIC void thunar_extension_shutdown(void)
#endif
{
#ifdef IN_NEMO_EXTENSION
	g_free(gtkhash_properties_nd_string);
	gtkhash_properties_nd_string = NULL;
#endif
}

#if defined(IN_NAUTILUS_EXTENSION)
PUBLIC void nautilus_module_list_types(const GType **types, int *num_types)
#elif defined(IN_CAJA_EXTENSION)
PUBLIC void caja_module_list_types(const GType **types, int *num_types)
#elif defined(IN_NEMO_EXTENSION)
PUBLIC void nemo_module_list_types(const GType **types, int *num_types)
#elif defined(IN_THUNAR_EXTENSION)
PUBLIC void thunar_extension_list_types(const GType **types, int *num_types);
PUBLIC void thunar_extension_list_types(const GType **types, int *num_types)
#endif
{
	static GType type_list[1];

	type_list[0] = page_type;
	*types = type_list;
	*num_types = G_N_ELEMENTS(type_list);
}

