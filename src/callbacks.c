/*
 *   Copyright (C) 2007-2019 Tristan Heaven <tristan@tristanheaven.net>
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
#include <gdk/gdk.h>

#include "callbacks.h"
#include "main.h"
#include "gui.h"
#include "hash.h"
#include "prefs.h"
#include "list.h"
#include "check.h"
#include "uri-digest.h"
#include "hash/hash-string.h"

static bool on_window_delete_event(void)
{
	gtk_widget_hide(GTK_WIDGET(gui.window));
	gtk_main_quit();

	return true;
}

static void on_menuitem_open_activate(void)
{
#if GTK_CHECK_VERSION(3,20,0)
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(gtk_file_chooser_native_new(
		_("Open Digest File"), gui.window, GTK_FILE_CHOOSER_ACTION_OPEN,
		_("_Open"), _("_Cancel")));
#else
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(
		gtk_file_chooser_dialog_new(_("Open Digest File"), gui.window,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Open"), GTK_RESPONSE_ACCEPT,
			NULL));
#endif

	gtk_file_chooser_set_select_multiple(chooser, true);
	gtk_file_chooser_set_local_only(chooser, false);

#ifndef G_OS_WIN32
	/* Note: Adding filters causes GTK+ to fallback to GtkFileChooserDialog.
	   On Windows, having a native file chooser is definitely preferable. */

	GtkFileFilter *filter = NULL;

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter,
		_("Digest/Checksum Files (*.sha1, *.md5, *.sfv, ...)"));
	check_file_add_filters(filter);
	gtk_file_chooser_add_filter(chooser, filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All Files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(chooser, filter);
#endif

#if GTK_CHECK_VERSION(3,20,0)
	if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#else
	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#endif
		GSList *files = gtk_file_chooser_get_files(chooser);
		GSList *ud_list = NULL;

		for (GSList *p = files; p; p = p->next)
			if (p->data)
				ud_list = check_file_load(ud_list, p->data);

		if (ud_list) {
			if (gui.view == GUI_VIEW_FILE_LIST)
				gui_add_ud_list(ud_list, GUI_VIEW_FILE_LIST);
			else
				gui_add_ud_list(ud_list, GUI_VIEW_INVALID);

			gui_update();

			uri_digest_list_free_full(ud_list);
		}

		g_slist_free_full(files, g_object_unref);
	}

#if GTK_CHECK_VERSION(3,20,0)
	g_object_unref(chooser);
#else
	gtk_widget_destroy(GTK_WIDGET(chooser));
#endif
}

static void on_menuitem_save_as_activate(void)
{
#if GTK_CHECK_VERSION(3,20,0)
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(gtk_file_chooser_native_new(
		_("Save Digest File"), gui.window, GTK_FILE_CHOOSER_ACTION_SAVE,
		_("_Save"), _("_Cancel")));
#else
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(
		gtk_file_chooser_dialog_new(_("Save Digest File"), gui.window,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_ACCEPT,
			NULL));
#endif

	gtk_file_chooser_set_select_multiple(chooser, false);
	gtk_file_chooser_set_local_only(chooser, true); // TODO
	gtk_file_chooser_set_do_overwrite_confirmation(chooser, true);

#if GTK_CHECK_VERSION(3,20,0)
	if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#else
	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#endif
		char *filename = gtk_file_chooser_get_filename(chooser);
		check_file_save(filename);
		g_free(filename);
	}

#if GTK_CHECK_VERSION(3,20,0)
	g_object_unref(chooser);
#else
	gtk_widget_destroy(GTK_WIDGET(chooser));
#endif
}

static void on_menuitem_quit_activate(void)
{
	gtk_widget_hide(GTK_WIDGET(gui.window));
	gtk_main_quit();
}

static void on_menuitem_edit_activate(void)
{
	GtkWidget *widget = gtk_window_get_focus(gui.window);
	bool selectable = false;
	bool editable = false;
	bool selection_ready = false;
	bool clipboard_ready = false;

	if (GTK_IS_ENTRY(widget)) {
		selectable = gtk_entry_get_text_length(GTK_ENTRY(widget));
		editable = gtk_editable_get_editable(GTK_EDITABLE(widget));
		selection_ready = gtk_editable_get_selection_bounds(
			GTK_EDITABLE(widget), NULL, NULL);
		clipboard_ready = gtk_clipboard_wait_is_text_available(
			gtk_clipboard_get(GDK_NONE));
	}

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_cut),
		selection_ready && editable);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_copy),
		selection_ready);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_paste),
		editable && clipboard_ready);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_delete),
		selection_ready && editable);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_select_all),
		selectable);
}

static void on_menuitem_cut_activate(void)
{
	GtkEditable *widget = GTK_EDITABLE(gtk_window_get_focus(gui.window));

	gtk_editable_cut_clipboard(widget);
}

static void on_menuitem_copy_activate(void)
{
	GtkEditable *widget = GTK_EDITABLE(gtk_window_get_focus(gui.window));

	gtk_editable_copy_clipboard(widget);
}

static void on_menuitem_paste_activate(void)
{
	GtkEditable *widget = GTK_EDITABLE(gtk_window_get_focus(gui.window));

	gtk_editable_paste_clipboard(widget);
}

static void on_menuitem_delete_activate(void)
{
	GtkEditable *widget = GTK_EDITABLE(gtk_window_get_focus(gui.window));

	gtk_editable_delete_selection(widget);
}

static void on_menuitem_select_all_activate(void)
{
	GtkEditable *widget = GTK_EDITABLE(gtk_window_get_focus(gui.window));

	gtk_editable_set_position(widget, -1);
	gtk_editable_select_region(widget, 0, -1);
}

static void on_menuitem_prefs_activate(void)
{
	gtk_widget_show(GTK_WIDGET(gui.dialog));
}

static void on_radiomenuitem_toggled(void)
{
	enum gui_view_e view = GUI_VIEW_INVALID;

	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_file)))
	{
		view = GUI_VIEW_FILE;
	} else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_text)))
	{
		view = GUI_VIEW_TEXT;
	} else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_file_list)))
	{
		view = GUI_VIEW_FILE_LIST;
	}

	g_assert(GUI_VIEW_IS_VALID(view));

	if (gui.view != view) {
		gui.view = view;
		gui_update();
	}
}

static void on_menuitem_about_activate(void)
{
#if (GTK_MAJOR_VERSION < 3)
	static const char * const license = {
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 2 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License along\n"
		"with this program; if not, see <https://gnu.org/licenses/gpl-2.0.txt>.\n"
	};
#endif

	static const char * const artists[] = {
		"Icon derived from GTK+ Logo "
		"https://wiki.gnome.org/Projects/GTK%2B/Logo",
		NULL
	};

	static const char * const authors[] = {
		"Tristan Heaven <tristan@tristanheaven.net>",
		NULL
	};

	gtk_show_about_dialog(
			gui.window,
			"artists", artists,
			"authors", authors,
			"comments", _("A GTK+ utility for computing message digests or checksums."),
#if (GTK_MAJOR_VERSION > 2)
			"license-type", GTK_LICENSE_GPL_2_0,
#else
			"license", license,
#endif
			"logo-icon-name", PACKAGE,
			"program-name", PACKAGE_NAME,
#if ENABLE_NLS
			"translator-credits", _("translator-credits"),
#endif
			"version", VERSION,
			"website", "https://github.com/tristanheaven/gtkhash",
			NULL);
}

static void on_filechooserbutton_selection_changed(void)
{
	bool enabled = hash_funcs_count_enabled();
	char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
		gui.filechooserbutton));

	if (enabled && uri) {
		g_free(uri);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), true);
	} else
		gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), false);

	gui_clear_digests();
}

static void on_toolbutton_add_clicked(void)
{
#if GTK_CHECK_VERSION(3,20,0)
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(gtk_file_chooser_native_new(
		_("Select Files"), gui.window, GTK_FILE_CHOOSER_ACTION_OPEN,
		_("_Open"), _("_Cancel")));
#else
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(
		gtk_file_chooser_dialog_new(_("Select Files"), gui.window,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Open"), GTK_RESPONSE_ACCEPT,
			NULL));
#endif

	gtk_file_chooser_set_select_multiple(chooser, true);
	gtk_file_chooser_set_local_only(chooser, false);

#if GTK_CHECK_VERSION(3,20,0)
	if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#else
	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
#endif
		GSList *uris = gtk_file_chooser_get_uris(chooser);
		GSList *ud_list = uri_digest_list_from_uri_list(uris);

		gui_add_ud_list(ud_list, GUI_VIEW_FILE_LIST);

		uri_digest_list_free_full(ud_list);
		g_slist_free(uris);
	}

#if GTK_CHECK_VERSION(3,20,0)
	g_object_unref(chooser);
#else
	gtk_widget_destroy(GTK_WIDGET(chooser));
#endif
}

static void on_treeselection_changed(void)
{
	const int rows = gtk_tree_selection_count_selected_rows(gui.treeselection);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.toolbutton_remove), (rows > 0));
}

static void show_menu_treeview(GdkEventButton *event)
{
	const int rows = gtk_tree_selection_count_selected_rows(gui.treeselection);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_remove),
		(rows > 0));

	bool can_copy = false;

	if (rows == 1) {
		for (int i = 0; i < HASH_FUNCS_N; i++) {
			if (!hash.funcs[i].enabled)
				continue;
			char *digest = list_get_selected_digest(i);
			if (digest && *digest) {
				can_copy = true;
				gtk_widget_set_sensitive(GTK_WIDGET(
					gui.hash_widgets[i].menuitem_treeview_copy), true);
			} else {
				gtk_widget_set_sensitive(GTK_WIDGET(
					gui.hash_widgets[i].menuitem_treeview_copy), false);
			}
			g_free(digest);
		}
	}

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_treeview_copy),
		can_copy);

#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(gui.menu_treeview, (GdkEvent *)event);
#else
	if (event) {
		gtk_menu_popup(gui.menu_treeview, NULL, NULL, NULL, NULL,
			event->button, event->time);
	} else {
		gtk_menu_popup(gui.menu_treeview, NULL, NULL, NULL, NULL,
			0, gtk_get_current_event_time());
	}
#endif
}

static void on_treeview_popup_menu(void)
{
	/* Note: Shift+F10 can trigger this, so it's possible for the pointer
	   to be outside the window */

	show_menu_treeview(NULL);
}

static bool on_treeview_button_press_event(G_GNUC_UNUSED GtkWidget *widget,
	GdkEventButton *event)
{
#if (GTK_MAJOR_VERSION > 2)
	if (gdk_event_triggers_context_menu((GdkEvent *)event))
#else
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
#endif
	{
		show_menu_treeview(event);
		// Stop processing the event now so the selection won't be changed
		return true;
	}

	return false;
}

static void on_treeview_drag_data_received(G_GNUC_UNUSED GtkWidget *widget,
	GdkDragContext *context, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y,
	GtkSelectionData *selection, G_GNUC_UNUSED guint info, guint t,
	G_GNUC_UNUSED gpointer data)
{
	char **uris = gtk_selection_data_get_uris(selection);
	if (!uris) {
		gtk_drag_finish(context, false, true, t);
		return;
	}

	GSList *ud_list = uri_digest_list_from_uri_strv(uris);

	gui_add_ud_list(ud_list, GUI_VIEW_FILE_LIST);

	uri_digest_list_free(ud_list);
	g_strfreev(uris);

	gtk_drag_finish(context, true, true, t);
}

static void on_menuitem_treeview_copy_activate(struct hash_func_s *func)
{
	char *digest = list_get_selected_digest(func->id);
	g_assert(digest);

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_NONE), digest, -1);

	g_free(digest);
}

static void on_menuitem_treeview_show_toolbar_toggled(void)
{
	const bool show_toolbar = gtk_check_menu_item_get_active(
		GTK_CHECK_MENU_ITEM(gui.menuitem_treeview_show_toolbar));

	gtk_widget_set_visible(GTK_WIDGET(gui.toolbar), show_toolbar);
}

static void on_button_hash_clicked(void)
{
	if (gui.view == GUI_VIEW_FILE) {
		// XXX: Workaround for when user clicks Cancel in FileChooserDialog and
		// XXX: uri is changed without emitting the "selection-changed" signal
		on_filechooserbutton_selection_changed();
		if (!gtk_widget_get_sensitive(GTK_WIDGET(gui.button_hash)))
			return;
	}

	gui_start_hash();
}

static void on_entry_check_icon_press(GtkEntry *entry,
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

static void on_togglebutton_hmac_file_toggled(void)
{
	g_assert(gui.view == GUI_VIEW_FILE);

	bool active = gtk_toggle_button_get_active(gui.togglebutton_hmac_file);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.entry_hmac_file), active);
	gui_clear_digests();

	gui_update_hash_func_labels(active);
}

static void on_togglebutton_hmac_text_toggled(void)
{
	g_assert(gui.view == GUI_VIEW_TEXT);

	bool active = gtk_toggle_button_get_active(gui.togglebutton_hmac_text);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.entry_hmac_text), active);

	hash_string();

	gui_update_hash_func_labels(active);
}

static void on_menuitem_show_hmac_key_toggled(GtkCheckMenuItem *item,
	GtkEntry *entry)
{
	const bool active = gtk_check_menu_item_get_active(item);

	gtk_entry_set_visibility(entry, active);
}

static void on_entry_hmac_populate_popup(GtkEntry *entry, GtkMenu *menu)
{
	GtkWidget *item;

	// Add separator
	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	// Add checkbutton
	item = gtk_check_menu_item_new_with_mnemonic(_("_Show HMAC Key"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),
		gtk_entry_get_visibility(entry));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(item, "toggled",
		G_CALLBACK(on_menuitem_show_hmac_key_toggled), entry);
}

static bool on_dialog_delete_event(void)
{
	gtk_widget_hide(GTK_WIDGET(gui.dialog));
	return true;
}

static void on_dialog_combobox_changed(void)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		gtk_entry_set_text(gui.hash_widgets[i].entry_file, "");
		gtk_entry_set_text(gui.hash_widgets[i].entry_text, "");
	}

	list_clear_digests();

	if (gui.view == GUI_VIEW_TEXT)
		hash_string();
	else
		gui_check_digests();
}

void callbacks_init(void)
{
#define CON(OBJ, SIG, CB) \
	g_signal_connect(G_OBJECT(OBJ), SIG, G_CALLBACK(CB), NULL)

	CON(gui.window,                         "delete-event",        on_window_delete_event);
	CON(gui.menuitem_open,                  "activate",            on_menuitem_open_activate);
	CON(gui.menuitem_save_as,               "activate",            on_menuitem_save_as_activate);
	CON(gui.menuitem_quit,                  "activate",            on_menuitem_quit_activate);
	CON(gui.menuitem_edit,                  "activate",            on_menuitem_edit_activate);
	CON(gui.menuitem_cut,                   "activate",            on_menuitem_cut_activate);
	CON(gui.menuitem_copy,                  "activate",            on_menuitem_copy_activate);
	CON(gui.menuitem_paste,                 "activate",            on_menuitem_paste_activate);
	CON(gui.menuitem_delete,                "activate",            on_menuitem_delete_activate);
	CON(gui.menuitem_select_all,            "activate",            on_menuitem_select_all_activate);
	CON(gui.menuitem_prefs,                 "activate",            on_menuitem_prefs_activate);
	CON(gui.radiomenuitem_file,             "toggled",             on_radiomenuitem_toggled);
	CON(gui.radiomenuitem_text,             "toggled",             on_radiomenuitem_toggled);
	CON(gui.radiomenuitem_file_list,        "toggled",             on_radiomenuitem_toggled);
	CON(gui.menuitem_about,                 "activate",            on_menuitem_about_activate);
//	file-set isn't emitted when file is deleted
//	CON(gui.filechooserbutton,              "file-set",            on_filechooserbutton_file_set);
	CON(gui.filechooserbutton,              "selection-changed",   on_filechooserbutton_selection_changed);
	CON(gui.entry_text,                     "changed",             hash_string);
	CON(gui.togglebutton_hmac_file,         "toggled",             on_togglebutton_hmac_file_toggled);
	CON(gui.togglebutton_hmac_text,         "toggled",             on_togglebutton_hmac_text_toggled);
	CON(gui.entry_hmac_file,                "populate-popup",      on_entry_hmac_populate_popup);
	CON(gui.entry_hmac_text,                "populate-popup",      on_entry_hmac_populate_popup);
	CON(gui.entry_hmac_file,                "changed",             gui_clear_digests);
	CON(gui.entry_hmac_text,                "changed",             hash_string);
	CON(gui.entry_check_file,               "changed",             gui_check_digests);
	CON(gui.entry_check_file,               "icon-press",          on_entry_check_icon_press);
	CON(gui.entry_check_text,               "changed",             gui_check_digests);
	CON(gui.entry_check_text,               "icon-press",          on_entry_check_icon_press);
	CON(gui.toolbutton_add,                 "clicked",             on_toolbutton_add_clicked);
	CON(gui.toolbutton_remove,              "clicked",             list_remove_selection);
	CON(gui.toolbutton_clear,               "clicked",             list_clear);
	CON(gui.treeselection,                  "changed",             on_treeselection_changed);
	CON(gui.treeview,                       "popup-menu",          on_treeview_popup_menu);
	CON(gui.treeview,                       "button-press-event",  on_treeview_button_press_event);
	CON(gui.treeview,                       "drag-data-received",  on_treeview_drag_data_received);
	CON(gui.menuitem_treeview_add,          "activate",            on_toolbutton_add_clicked);
	CON(gui.menuitem_treeview_remove,       "activate",            list_remove_selection);
	CON(gui.menuitem_treeview_clear,        "activate",            list_clear);
	CON(gui.menuitem_treeview_show_toolbar, "toggled",             on_menuitem_treeview_show_toolbar_toggled);
	CON(gui.button_hash,                    "clicked",             on_button_hash_clicked);
	CON(gui.button_stop,                    "clicked",             gui_stop_hash);
	CON(gui.dialog,                         "delete-event",        G_CALLBACK(on_dialog_delete_event));
	CON(gui.dialog_button_close,            "clicked",             G_CALLBACK(on_dialog_delete_event));
	CON(gui.dialog_combobox,                "changed",             on_dialog_combobox_changed);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported)
			continue;

		CON(gui.hash_widgets[i].button, "toggled", gui_update);
		g_signal_connect_swapped(gui.hash_widgets[i].menuitem_treeview_copy,
			"activate", G_CALLBACK(on_menuitem_treeview_copy_activate),
			&hash.funcs[i]);
	}

#undef CON
}
