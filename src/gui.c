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
#include <errno.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gui.h"
#include "main.h"
#include "callbacks.h"
#include "hash.h"
#include "list.h"
#include "prefs.h"
#include "uri-digest.h"
#include "hash/digest-format.h"
#include "hash/hash-func.h"

#if (GTK_MAJOR_VERSION > 2)
#define GUI_XML_RESOURCE "/org/gtkhash/gtkhash-gtk3.xml"
#else
#define GUI_XML_RESOURCE "/org/gtkhash/gtkhash-gtk2.xml"
#endif

struct gui_s gui = {
	.view = GUI_VIEW_INVALID,
};

static struct {
	enum gui_state_e state;
} gui_priv = {
	.state = GUI_STATE_INVALID,
};

static GObject *gui_get_object(GtkBuilder *builder, const char *name)
{
	g_assert(name);

	GObject *obj = gtk_builder_get_object(builder, name);
	if (!obj)
		g_error("unknown object: \"%s\"", name);

	return obj;
}

static void gui_init_objects(GtkBuilder *builder)
{
	// Window
	gui.window = GTK_WINDOW(gui_get_object(builder,
		"window"));

	// Menus
	gui.menuitem_open = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_open"));
	gui.menuitem_save_as = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_save_as"));
	gui.menuitem_quit = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_quit"));
	gui.menuitem_edit = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_edit"));
	gui.menuitem_cut = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_cut"));
	gui.menuitem_copy = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_copy"));
	gui.menuitem_paste = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_paste"));
	gui.menuitem_delete = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_delete"));
	gui.menuitem_select_all = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_select_all"));
	gui.menuitem_prefs = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_prefs"));
	gui.menuitem_about = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_about"));
	gui.radiomenuitem_file = GTK_RADIO_MENU_ITEM(gui_get_object(builder,
		"radiomenuitem_file"));
	gui.radiomenuitem_text = GTK_RADIO_MENU_ITEM(gui_get_object(builder,
		"radiomenuitem_text"));
	gui.radiomenuitem_file_list = GTK_RADIO_MENU_ITEM(gui_get_object(builder,
		"radiomenuitem_file_list"));

	// Toolbar
	gui.toolbar = GTK_TOOLBAR(gui_get_object(builder,
		"toolbar"));
	gui.toolbutton_add = GTK_TOOL_BUTTON(gui_get_object(builder,
		"toolbutton_add"));
	gui.toolbutton_remove = GTK_TOOL_BUTTON(gui_get_object(builder,
		"toolbutton_remove"));
	gui.toolbutton_clear = GTK_TOOL_BUTTON(gui_get_object(builder,
		"toolbutton_clear"));

	// Containers
	gui.vbox_single = GTK_VBOX(gui_get_object(builder,
		"vbox_single"));
	gui.vbox_list = GTK_VBOX(gui_get_object(builder,
		"vbox_list"));
	gui.hbox_input = GTK_HBOX(gui_get_object(builder,
		"hbox_input"));
	gui.hbox_output = GTK_HBOX(gui_get_object(builder,
		"hbox_output"));
	gui.vbox_outputlabels = GTK_VBOX(gui_get_object(builder,
		"vbox_outputlabels"));
	gui.vbox_digests_file = GTK_VBOX(gui_get_object(builder,
		"vbox_digests_file"));
	gui.vbox_digests_text = GTK_VBOX(gui_get_object(builder,
		"vbox_digests_text"));

	// Inputs
	gui.filechooserbutton = GTK_FILE_CHOOSER_BUTTON(gui_get_object(builder,
		"filechooserbutton"));
	gui.entry_text = GTK_ENTRY(gui_get_object(builder,
		"entry_text"));
	gui.entry_check_file = GTK_ENTRY(gui_get_object(builder,
		"entry_check_file"));
	gui.entry_check_text = GTK_ENTRY(gui_get_object(builder,
		"entry_check_text"));
	gui.togglebutton_hmac_file = GTK_TOGGLE_BUTTON(gui_get_object(builder,
		"togglebutton_hmac_file"));
	gui.togglebutton_hmac_text = GTK_TOGGLE_BUTTON(gui_get_object(builder,
		"togglebutton_hmac_text"));
	gui.entry_hmac_file = GTK_ENTRY(gui_get_object(builder,
		"entry_hmac_file"));
	gui.entry_hmac_text = GTK_ENTRY(gui_get_object(builder,
		"entry_hmac_text"));

	// Labels
	gui.label_file = GTK_LABEL(gui_get_object(builder,
		"label_file"));
	gui.label_text = GTK_LABEL(gui_get_object(builder,
		"label_text"));

	// Tree View
	gui.treeview = GTK_TREE_VIEW(gui_get_object(builder,
		"treeview"));
	gui.treeselection = GTK_TREE_SELECTION(gui_get_object(builder,
		"treeselection"));
	gui.menu_treeview = GTK_MENU(gui_get_object(builder,
		"menu_treeview"));
	g_object_ref(gui.menu_treeview);
	gui.menuitem_treeview_add = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_treeview_add"));
	gui.menuitem_treeview_remove = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_treeview_remove"));
	gui.menuitem_treeview_clear = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_treeview_clear"));
	gui.menu_treeview_copy = GTK_MENU(gui_get_object(builder,
		"menu_treeview_copy"));
	gui.menuitem_treeview_copy = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_treeview_copy"));
	gui.menuitem_treeview_show_toolbar = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_treeview_show_toolbar"));

	// Buttons
	gui.hseparator_buttons = GTK_HSEPARATOR(gui_get_object(builder,
		"hseparator_buttons"));
	gui.button_hash = GTK_BUTTON(gui_get_object(builder,
		"button_hash"));
	gui.button_stop = GTK_BUTTON(gui_get_object(builder,
		"button_stop"));

	// Progress Bar
	gui.progressbar = GTK_PROGRESS_BAR(gui_get_object(builder,
		"progressbar"));

	// Dialog
	gui.dialog = GTK_DIALOG(gui_get_object(builder,
		"dialog"));
#if (GTK_MAJOR_VERSION > 2)
	gui.dialog_grid = GTK_GRID(gui_get_object(builder,
		"dialog_grid"));
#else
	gui.dialog_table = GTK_TABLE(gui_get_object(builder,
		"dialog_table"));
#endif
	gui.dialog_combobox = GTK_COMBO_BOX(gui_get_object(builder,
		"dialog_combobox"));
	gui.dialog_button_close = GTK_BUTTON(gui_get_object(builder,
		"dialog_button_close"));
}

static void gui_init_hash_funcs(void)
{
	for (int i = 0, supported = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		// File/Text view func labels
		gui.hash_widgets[i].label = GTK_LABEL(gtk_label_new(NULL));
		gtk_container_add(GTK_CONTAINER(gui.vbox_outputlabels),
			GTK_WIDGET(gui.hash_widgets[i].label));
#if (GTK_MAJOR_VERSION > 2)
		gtk_widget_set_halign(GTK_WIDGET(gui.hash_widgets[i].label),
			GTK_ALIGN_START);
#else
		gtk_misc_set_alignment(GTK_MISC(gui.hash_widgets[i].label),
			// Left align
			0.0, 0.5);
#endif

		// File view digests
		gui.hash_widgets[i].entry_file = GTK_ENTRY(gtk_entry_new());
		gtk_container_add(GTK_CONTAINER(gui.vbox_digests_file),
			GTK_WIDGET(gui.hash_widgets[i].entry_file));
		gtk_editable_set_editable(GTK_EDITABLE(gui.hash_widgets[i].entry_file),
			false);
		gtk_entry_set_icon_activatable(gui.hash_widgets[i].entry_file,
			GTK_ENTRY_ICON_SECONDARY, false);

		// Text view digests
		gui.hash_widgets[i].entry_text = GTK_ENTRY(gtk_entry_new());
		gtk_container_add(GTK_CONTAINER(gui.vbox_digests_text),
			GTK_WIDGET(gui.hash_widgets[i].entry_text));
		gtk_editable_set_editable(GTK_EDITABLE(gui.hash_widgets[i].entry_text),
			false);
		gtk_entry_set_icon_activatable(gui.hash_widgets[i].entry_text,
			GTK_ENTRY_ICON_SECONDARY, false);

		// File list treeview popup menu
		gui.hash_widgets[i].menuitem_treeview_copy =
			GTK_MENU_ITEM(gtk_menu_item_new_with_label(hash.funcs[i].name));
		gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu_treeview_copy),
			GTK_WIDGET(gui.hash_widgets[i].menuitem_treeview_copy));

		// Dialog checkbuttons
		gui.hash_widgets[i].button = GTK_TOGGLE_BUTTON(
			gtk_check_button_new_with_label(hash.funcs[i].name));

#if (GTK_MAJOR_VERSION > 2)
		gtk_grid_attach(gui.dialog_grid,
			GTK_WIDGET(gui.hash_widgets[i].button),
			supported % 2 ? 1 : 0, // column
			supported / 2,         // row
			1,                     // width
			1);                    // height
#else
		gtk_table_attach_defaults(gui.dialog_table,
			GTK_WIDGET(gui.hash_widgets[i].button),
			// Sort checkbuttons into 2 columns
			supported % 2 ? 1 : 0,
			supported % 2 ? 2 : 1,
			supported / 2,
			supported / 2 + 1);
#endif
		// Could be enabled already by cmdline arg
		if (hash.funcs[i].enabled)
			gui_enable_hash_func(i);

		gtk_widget_show(GTK_WIDGET(gui.hash_widgets[i].button));

		supported++;
	}
}

#if (GTK_MAJOR_VERSION < 3)
static void gui_init_fonts(void)
{
	PangoFontDescription *desc = pango_font_description_from_string(
		"monospace");

	gtk_widget_modify_font(GTK_WIDGET(gui.entry_check_file), desc);
	gtk_widget_modify_font(GTK_WIDGET(gui.entry_check_text), desc);
	gtk_widget_modify_font(GTK_WIDGET(gui.treeview), desc);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		gtk_widget_modify_font(GTK_WIDGET(gui.hash_widgets[i].entry_file), desc);
		gtk_widget_modify_font(GTK_WIDGET(gui.hash_widgets[i].entry_text), desc);
	}

	pango_font_description_free(desc);
}
#endif

static GtkBuilder *gui_init_builder(void)
{
#if (GTK_MAJOR_VERSION > 2)
	return gtk_builder_new_from_resource(GUI_XML_RESOURCE);
#else
	GError *error = NULL;
	GBytes *bytes = g_resources_lookup_data(GUI_XML_RESOURCE,
		G_RESOURCE_LOOKUP_FLAGS_NONE, &error);

	if (G_UNLIKELY(error)) {
		gui_error(error->message);
		g_error_free(error);

		exit(EXIT_FAILURE);
	}

	gsize xml_len = 0;
	char *xml = g_bytes_unref_to_data(bytes, &xml_len);
	GtkBuilder *builder = gtk_builder_new();

	gtk_builder_add_from_string(builder, xml, xml_len, &error);
	g_free(xml);

	if (G_UNLIKELY(error)) {
		gui_error(error->message);
		g_error_free(error);
		g_object_unref(builder);

		exit(EXIT_FAILURE);
	}

	return builder;
#endif
}

void gui_init(void)
{
	GtkBuilder *builder = gui_init_builder();
	gui_init_objects(builder);
	g_object_unref(builder);

	gui_init_hash_funcs();

#if (GTK_MAJOR_VERSION < 3)
	gui_init_fonts();

	g_object_set(gtk_settings_get_default(), "gtk-button-images", true, NULL);
	g_object_set(gtk_settings_get_default(), "gtk-menu-images", true, NULL);
#endif

	gui_set_state(GUI_STATE_IDLE);
}

static char *gui_try_uri(const char *uri)
{
	g_assert(uri);

	char *error_str = NULL;

	GFile *file = g_file_new_for_uri(uri);
	GError *error = NULL;
	GFileInfo *info = g_file_query_info(file,
		G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_ACCESS_CAN_READ,
		G_FILE_QUERY_INFO_NONE, NULL, &error);

	if (info) {
		bool can_read = g_file_info_get_attribute_boolean(info,
			G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
		GFileType type = g_file_info_get_file_type(info);
		g_object_unref(info);
		if (!can_read)
			error_str = g_strdup(g_strerror(EACCES));
		else if (type == G_FILE_TYPE_DIRECTORY) // TODO
			error_str = g_strdup(g_strerror(EISDIR));
		else if (type != G_FILE_TYPE_REGULAR)
			error_str = g_strdup(_("Not a regular file"));
	} else {
		error_str = g_strdup(error->message);
		g_error_free(error);
	}

	g_object_unref(file);

	return error_str;
}

unsigned int gui_add_ud_list(GSList *ud_list, const enum gui_view_e view)
{
	g_assert(ud_list);

	GSList *readable = NULL;
	unsigned int readable_len = 0;
	{
		GSList *tmp = ud_list;
		do {
			char *error = NULL;
			struct uri_digest_s *ud = tmp->data;
			if ((error = gui_try_uri(ud->uri))) {
				g_message(_("Failed to add file \"%s\": %s"),
					ud->uri, error);
				g_free(error);
				continue;
			}
			readable = g_slist_prepend(readable, tmp->data);
			readable_len++;
		} while ((tmp = tmp->next));
	}
	readable = g_slist_reverse(readable);

	if (!GUI_VIEW_IS_VALID(view)) {
		if (readable_len == 1)
			gui_set_view(GUI_VIEW_FILE);
		else if (readable_len > 1)
			gui_set_view(GUI_VIEW_FILE_LIST);
	}

	if (readable_len && (gui.view == GUI_VIEW_FILE)) {
		struct uri_digest_s *ud = readable->data;
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(gui.filechooserbutton),
			ud->uri);
		if (ud->digest)
			gtk_entry_set_text(gui.entry_check_file, ud->digest);
	} else if (readable_len && (gui.view == GUI_VIEW_FILE_LIST)) {
		GSList *tmp = readable;
		do {
			struct uri_digest_s *ud = tmp->data;
			list_append_row(ud->uri, ud->digest);
		} while ((tmp = tmp->next));
		list_update();
	}

	g_slist_free(readable);

	return readable_len;
}

void gui_add_check(const char *check)
{
	g_assert(check && *check);

	gtk_entry_set_text(gui.entry_check_file, check);
	gtk_entry_set_text(gui.entry_check_text, check);
}

void gui_add_text(const char *text)
{
	g_assert(text);

	gtk_entry_set_text(gui.entry_text, text);

	gui_set_view(GUI_VIEW_TEXT);
	gui_update();

	gtk_editable_set_position(GTK_EDITABLE(gui.entry_text), -1);
}

void gui_error(const char *message)
{
	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), PACKAGE_NAME);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void gui_run(void)
{
	// Set default view
	if (!GUI_VIEW_IS_VALID(gui.view)) {
		gui_set_view(GUI_VIEW_FILE);
		gui_update();
	}

	gtk_widget_show_now(GTK_WIDGET(gui.window));

	// Connect signals to start handling events
	callbacks_init();

	gtk_main();
}

void gui_deinit(void)
{
	hash_file_stop();

	while (gui_priv.state != GUI_STATE_IDLE)
		gtk_main_iteration();

	gtk_widget_destroy(GTK_WIDGET(gui.window));
	g_object_unref(gui.menu_treeview);

	while (gtk_events_pending())
		gtk_main_iteration();
}

void gui_set_view(const enum gui_view_e view)
{
	g_assert(GUI_VIEW_IS_VALID(view));

	if (view == gui.view)
		return;

	gui.view = view;

	switch (view) {
		case GUI_VIEW_FILE:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_file), true);
			break;
		case GUI_VIEW_TEXT:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_text), true);
			break;
		case GUI_VIEW_FILE_LIST:
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
				gui.radiomenuitem_file_list), true);
			break;
		default:
			g_assert_not_reached();
	}
}

void gui_set_digest_format(const enum digest_format_e format)
{
	g_assert(DIGEST_FORMAT_IS_VALID(format));

	gtk_combo_box_set_active(gui.dialog_combobox, format);
}

enum digest_format_e gui_get_digest_format(void)
{
	enum digest_format_e format = gtk_combo_box_get_active(gui.dialog_combobox);
	g_assert(DIGEST_FORMAT_IS_VALID(format));

	return format;
}

const uint8_t *gui_get_hmac_key(size_t *key_size)
{
	g_assert(key_size);

	const uint8_t *hmac_key = NULL;
	*key_size = 0;

	switch (gui.view) {
		case GUI_VIEW_FILE:
			if (gtk_toggle_button_get_active(gui.togglebutton_hmac_file)) {
				GtkEntryBuffer *buffer = gtk_entry_get_buffer(
					gui.entry_hmac_file);

				hmac_key = (uint8_t *)gtk_entry_get_text(gui.entry_hmac_file);
				*key_size = gtk_entry_buffer_get_bytes(buffer);
			}
			break;
		case GUI_VIEW_TEXT:
			if (gtk_toggle_button_get_active(gui.togglebutton_hmac_text)) {
				GtkEntryBuffer *buffer = gtk_entry_get_buffer(
					gui.entry_hmac_text);

				hmac_key = (uint8_t *)gtk_entry_get_text(gui.entry_hmac_text);
				*key_size = gtk_entry_buffer_get_bytes(buffer);
			}
			break;
		case GUI_VIEW_FILE_LIST: // TODO
			break;
		default:
			g_assert_not_reached();
	}

	return hmac_key;
}

static void gui_menuitem_save_as_set_sensitive(void)
{
	if (gui_priv.state == GUI_STATE_BUSY) {
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_save_as), false);
		return;
	}

	bool sensitive = false;

	switch (gui.view) {
		case GUI_VIEW_FILE:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (hash.funcs[i].enabled &&
					*gtk_entry_get_text(gui.hash_widgets[i].entry_file))
				{
					sensitive = true;
					break;
				}
			}
			break;
		case GUI_VIEW_TEXT:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (hash.funcs[i].enabled) {
					sensitive = true;
					break;
				}
			}
			break;
		case GUI_VIEW_FILE_LIST:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (hash.funcs[i].enabled) {
					char *digest = list_get_digest(0, i);
					if (digest != NULL && *digest) {
						g_free(digest);
						sensitive = true;
						break;
					}
				}
			}
			break;
		default:
			sensitive = false;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_save_as), sensitive);
}

void gui_enable_hash_func(const enum hash_func_e id)
{
	g_assert(HASH_FUNC_IS_VALID(id));

	if (hash.funcs[id].supported)
		gtk_toggle_button_set_active(gui.hash_widgets[id].button, true);
}

void gui_update_hash_func_labels(const bool hmac_enabled)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		char *str = NULL;

		// FIXME: different labels for RTL?
		if (hmac_enabled && hash.funcs[i].hmac_supported)
			str = g_strdup_printf("HMAC-%s:", hash.funcs[i].name);
		else
			str = g_strdup_printf("%s:", hash.funcs[i].name);

		gtk_label_set_text(gui.hash_widgets[i].label, str);
		g_free(str);
	}
}

static void gui_update_hash_funcs(void)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		hash.funcs[i].enabled = gtk_toggle_button_get_active(
			gui.hash_widgets[i].button);

		gtk_widget_set_visible(GTK_WIDGET(gui.hash_widgets[i].label),
			hash.funcs[i].enabled);
		gtk_widget_set_visible(GTK_WIDGET(gui.hash_widgets[i].entry_file),
			hash.funcs[i].enabled);
		gtk_widget_set_visible(GTK_WIDGET(gui.hash_widgets[i].entry_text),
			hash.funcs[i].enabled);
		gtk_widget_set_visible(GTK_WIDGET(gui.hash_widgets[i].menuitem_treeview_copy),
			hash.funcs[i].enabled);
	}
}

static void gui_update_hmac(void)
{
	bool active = false;

	switch (gui.view) {
		case GUI_VIEW_FILE:
			gtk_widget_hide(GTK_WIDGET(gui.entry_hmac_text));
			gtk_widget_hide(GTK_WIDGET(gui.togglebutton_hmac_text));
			gtk_widget_show(GTK_WIDGET(gui.entry_hmac_file));
			gtk_widget_show(GTK_WIDGET(gui.togglebutton_hmac_file));

			active = gtk_toggle_button_get_active(gui.togglebutton_hmac_file);
			gtk_widget_set_sensitive(GTK_WIDGET(gui.entry_hmac_file), active);
			break;
		case GUI_VIEW_TEXT:
			gtk_widget_hide(GTK_WIDGET(gui.entry_hmac_file));
			gtk_widget_hide(GTK_WIDGET(gui.togglebutton_hmac_file));
			gtk_widget_show(GTK_WIDGET(gui.entry_hmac_text));
			gtk_widget_show(GTK_WIDGET(gui.togglebutton_hmac_text));

			active = gtk_toggle_button_get_active(gui.togglebutton_hmac_text);
			gtk_widget_set_sensitive(GTK_WIDGET(gui.entry_hmac_text), active);
			break;
		default:
			g_assert_not_reached();
	}

	gui_update_hash_func_labels(active);
}

void gui_update(void)
{
	// Must call gui_set_view() before this
	g_assert(GUI_VIEW_IS_VALID(gui.view));
	g_assert(gui_priv.state == GUI_STATE_IDLE);

	gui_update_hash_funcs();
	const unsigned int funcs_enabled = hash_funcs_count_enabled();

	if ((gui.view == GUI_VIEW_FILE) || (gui.view == GUI_VIEW_TEXT)) {
		gui_update_hmac();

		gtk_widget_hide(GTK_WIDGET(gui.toolbar));
		gtk_widget_hide(GTK_WIDGET(gui.vbox_list));
		gtk_widget_show(GTK_WIDGET(gui.vbox_single));
	}

	switch (gui.view) {
		case GUI_VIEW_FILE: {
			gtk_widget_hide(GTK_WIDGET(gui.label_text));
			gtk_widget_hide(GTK_WIDGET(gui.entry_text));
			gtk_widget_hide(GTK_WIDGET(gui.entry_check_text));

			gtk_widget_hide(GTK_WIDGET(gui.vbox_digests_text));
			gtk_widget_show(GTK_WIDGET(gui.label_file));
			gtk_widget_show(GTK_WIDGET(gui.filechooserbutton));
			gtk_widget_show(GTK_WIDGET(gui.entry_check_file));
			gtk_widget_show(GTK_WIDGET(gui.vbox_digests_file));
			gtk_widget_show(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_show(GTK_WIDGET(gui.button_hash));

			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
				gui.filechooserbutton));
			if (uri) {
				gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash),
					funcs_enabled);
				g_free(uri);
			} else
				gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), false);
			break;
		}
		case GUI_VIEW_TEXT:
			gtk_widget_hide(GTK_WIDGET(gui.label_file));
			gtk_widget_hide(GTK_WIDGET(gui.filechooserbutton));
			gtk_widget_hide(GTK_WIDGET(gui.entry_check_file));
			gtk_widget_hide(GTK_WIDGET(gui.vbox_digests_file));
			gtk_widget_hide(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_hide(GTK_WIDGET(gui.button_hash));
			gtk_widget_show(GTK_WIDGET(gui.label_text));
			gtk_widget_show(GTK_WIDGET(gui.entry_text));
			gtk_widget_show(GTK_WIDGET(gui.entry_check_text));
			gtk_widget_show(GTK_WIDGET(gui.vbox_digests_text));

			gtk_widget_grab_focus(GTK_WIDGET(gui.entry_text));

			if (funcs_enabled)
				hash_string();
			break;
		case GUI_VIEW_FILE_LIST:
			list_update();

			gtk_widget_hide(GTK_WIDGET(gui.vbox_single));
			gtk_widget_hide(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_show(GTK_WIDGET(gui.vbox_list));
			gtk_widget_show(GTK_WIDGET(gui.button_hash));

			gtk_widget_set_visible(GTK_WIDGET(gui.toolbar),
				gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
					gui.menuitem_treeview_show_toolbar)));
			break;
		default:
			g_assert_not_reached();
	}

	gui_check_digests();
	gui_menuitem_save_as_set_sensitive();
}

void gui_clear_digests(void)
{
	switch (gui.view) {
		case GUI_VIEW_FILE:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (!hash.funcs[i].supported)
					continue;
				gtk_entry_set_text(gui.hash_widgets[i].entry_file, "");
			}
			gui_check_digests();
			break;
		case GUI_VIEW_TEXT:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (!hash.funcs[i].supported)
					continue;
				gtk_entry_set_text(gui.hash_widgets[i].entry_text, "");
			}
			gui_check_digests();
			break;
		case GUI_VIEW_FILE_LIST:
			list_clear_digests();
			break;
		default:
			g_assert_not_reached();
	}

	gui_menuitem_save_as_set_sensitive();
}

void gui_check_digests(void)
{
	const char *icon_in = NULL;
	GtkEntry *entry_check = NULL;

	switch (gui.view) {
		case GUI_VIEW_TEXT:
			entry_check = gui.entry_check_text;
			break;
		case GUI_VIEW_FILE:
			entry_check = gui.entry_check_file;
			break;
		case GUI_VIEW_FILE_LIST: // TODO
			return;
		default:
			g_assert_not_reached();
	}

	const enum digest_format_e format = gui_get_digest_format();
	const char *str_in = gtk_entry_get_text(entry_check);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		GtkEntry *entry = NULL;

		switch (gui.view) {
			case GUI_VIEW_TEXT:
				entry = gui.hash_widgets[i].entry_text;
				break;
			case GUI_VIEW_FILE:
				entry = gui.hash_widgets[i].entry_file;
				break;
			default:
				g_assert_not_reached();
		}

		const char *str_out = gtk_entry_get_text(entry);
		const char *icon_out = NULL;

		switch (format) {
			case DIGEST_FORMAT_HEX_LOWER:
			case DIGEST_FORMAT_HEX_UPPER:
				if (*str_in && (g_ascii_strcasecmp(str_in, str_out) == 0)) {
					// FIXME: find a real alternative for GTK_STOCK_YES
					icon_out = "gtk-yes";
					icon_in = "gtk-yes";
				}
				break;
			case DIGEST_FORMAT_BASE64:
				if (*str_in && (strcmp(str_in, str_out) == 0)) {
					icon_out = "gtk-yes";
					icon_in = "gtk-yes";
				}
				break;
			default:
				g_assert_not_reached();
		}

		gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY,
			icon_out);
	}

	gtk_entry_set_icon_from_icon_name(entry_check, GTK_ENTRY_ICON_SECONDARY,
		icon_in);
}

void gui_set_state(const enum gui_state_e state)
{
	g_assert(GUI_STATE_IS_VALID(state));
	g_assert(state != gui_priv.state);

	if (gui.view == GUI_VIEW_TEXT)
		g_assert(state != GUI_STATE_BUSY);

	gui_priv.state = state;

	const bool busy = (state == GUI_STATE_BUSY);

	gtk_widget_set_visible(GTK_WIDGET(gui.button_hash), !busy);

	gtk_widget_set_visible(GTK_WIDGET(gui.button_stop), busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_stop), busy);

	gtk_progress_bar_set_fraction(gui.progressbar, 0.0);
	gtk_progress_bar_set_text(gui.progressbar, " ");
	gtk_widget_set_visible(GTK_WIDGET(gui.progressbar), busy);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.hbox_input), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.hbox_output), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.toolbar), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.treeview), !busy);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_open), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_text), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_file), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_file_list), !busy);

#if (GTK_MAJOR_VERSION > 2)
	gtk_widget_set_sensitive(GTK_WIDGET(gui.dialog_grid), !busy);
#else
	gtk_widget_set_sensitive(GTK_WIDGET(gui.dialog_table), !busy);
#endif

	gtk_widget_set_sensitive(GTK_WIDGET(gui.dialog_combobox), !busy);

	if (busy)
		gtk_window_set_default(gui.window, GTK_WIDGET(gui.button_stop));
	else
		gtk_window_set_default(gui.window, GTK_WIDGET(gui.button_hash));

	gui_menuitem_save_as_set_sensitive();
}

bool gui_is_maximised(void)
{
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(gui.window));

	if (!window)
		return false;

	GdkWindowState state = gdk_window_get_state(window);

	return (state & GDK_WINDOW_STATE_MAXIMIZED);
}

void gui_start_hash(void)
{
	switch (gui.view) {
		case GUI_VIEW_FILE: {
			gui_clear_digests();
			gui_set_state(GUI_STATE_BUSY);
			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
				gui.filechooserbutton));
			hash_file_start(uri);
			break;
		}
		case GUI_VIEW_FILE_LIST:
			gui_clear_digests();
			gui_set_state(GUI_STATE_BUSY);
			hash_file_list_start();
			break;
		default:
			g_assert_not_reached();
	}
}

void gui_stop_hash(void)
{
	g_assert(gui.view != GUI_VIEW_TEXT);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_stop), false);
	hash_file_stop();
}
