/*
 *   Copyright (C) 2007-2011 Tristan Heaven <tristanheaven@gmail.com>
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
 *   along with GtkHash. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

#include "gui.h"
#include "main.h"
#include "callbacks.h"
#include "hash.h"
#include "list.h"
#include "prefs.h"

static GObject *gui_get_object(GtkBuilder *builder, const char *name)
{
	g_assert(name);

	GObject *obj = gtk_builder_get_object(builder, name);
	if (!obj)
		g_error("unknown object: \"%s\"\n", name);

	return obj;
}

static void gui_get_objects(GtkBuilder *builder)
{
	// Window
	gui.window = GTK_WINDOW(gui_get_object(builder, "window"));

	// Menus
	gui.menuitem_file = GTK_MENU_ITEM(gui_get_object(builder,
		"menuitem_file"));
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
	gui.entry = GTK_ENTRY(gui_get_object(builder,
		"entry"));
	gui.filechooserbutton = GTK_FILE_CHOOSER_BUTTON(gui_get_object(builder,
		"filechooserbutton"));

	// Labels
	gui.label_text = GTK_LABEL(gui_get_object(builder, "label_text"));
	gui.label_file = GTK_LABEL(gui_get_object(builder, "label_file"));

	// Tree View
	gui.treeview = GTK_TREE_VIEW(gui_get_object(builder, "treeview"));
	gui.treeselection = GTK_TREE_SELECTION(gtk_tree_view_get_selection(
		gui.treeview));

	// Buttons
	gui.hseparator_buttons = GTK_HSEPARATOR(gui_get_object(builder,
		"hseparator_buttons"));
	gui.button_hash = GTK_BUTTON(gui_get_object(builder, "button_hash"));
	gui.button_stop = GTK_BUTTON(gui_get_object(builder, "button_stop"));

	// Progress Bar
	gui.progressbar = GTK_PROGRESS_BAR(gui_get_object(builder, "progressbar"));

	// Dialog
	gui.dialog = GTK_DIALOG(gui_get_object(builder,
		"dialog"));
	gui.dialog_table = GTK_TABLE(gui_get_object(builder,
		"dialog_table"));
	gui.dialog_button_close = GTK_BUTTON(gui_get_object(builder,
		"dialog_button_close"));
}

static void init_hash_funcs(void)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		gui.hash_widgets[i].button = GTK_TOGGLE_BUTTON(
			gtk_check_button_new_with_label(hash.funcs[i].name));
		g_signal_connect(G_OBJECT(gui.hash_widgets[i].button), "toggled",
			gui_update, NULL);
		if (!hash.funcs[i].supported)
			gtk_widget_set_sensitive(GTK_WIDGET(gui.hash_widgets[i].button),
				false);

		// Label the digest outputs
		char *label = g_strdup_printf("%s:", hash.funcs[i].name);
		gui.hash_widgets[i].label = GTK_LABEL(gtk_label_new(label));
		g_free(label);

		gui.hash_widgets[i].entry_file = GTK_ENTRY(gtk_entry_new());
		gui.hash_widgets[i].entry_text = GTK_ENTRY(gtk_entry_new());

		gtk_table_attach_defaults(gui.dialog_table,
			GTK_WIDGET(gui.hash_widgets[i].button),
			// Sort checkbuttons into 2 columns
			i % 2 ? 1 : 0,
			i % 2 ? 2 : 1,
			i / 2,
			i / 2 + 1);

		gtk_container_add(GTK_CONTAINER(gui.vbox_outputlabels),
			GTK_WIDGET(gui.hash_widgets[i].label));
		// Left align
		gtk_misc_set_alignment(GTK_MISC(gui.hash_widgets[i].label), 0.0, 0.5);

		gtk_container_add(GTK_CONTAINER(gui.vbox_digests_file),
			GTK_WIDGET(gui.hash_widgets[i].entry_file));
		gtk_editable_set_editable(GTK_EDITABLE(gui.hash_widgets[i].entry_file), false);

		gtk_container_add(GTK_CONTAINER(gui.vbox_digests_text),
			GTK_WIDGET(gui.hash_widgets[i].entry_text));
		gtk_editable_set_editable(GTK_EDITABLE(gui.hash_widgets[i].entry_text), false);

		gtk_widget_show(GTK_WIDGET(gui.hash_widgets[i].button));
	}
}

void gui_init(void)
{
	GtkBuilder *builder = gtk_builder_new();

#if ENABLE_NLS
	gtk_builder_set_translation_domain(builder, PACKAGE);
#endif

	if (!gtk_builder_add_from_file(builder, BUILDER_XML, NULL)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Failed to read %s:\n%s"),
			BUILDER_XML, g_strerror(errno));
		gtk_dialog_run(GTK_DIALOG(dialog));
		exit(EXIT_FAILURE);
	}

	gui_get_objects(builder);
	callbacks_init();
	init_hash_funcs();

	g_object_unref(builder);
}

void gui_run(void)
{
	// Show window here so it doesn't resize just after it's opened
	gtk_widget_show(GTK_WIDGET(gui.window));

	gtk_main();
}

enum gui_view_e gui_get_view(void)
{
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_file)))
	{
		return VIEW_FILE;
	} else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_text)))
	{
		return VIEW_TEXT;
	} else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
		gui.radiomenuitem_file_list)))
	{
		return VIEW_FILE_LIST;
	} else
		g_assert_not_reached();
}

void gui_update(void)
{
	bool has_enabled = false;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (hash.funcs[i].supported)
			hash.funcs[i].enabled = gtk_toggle_button_get_active(gui.hash_widgets[i].button);
		else {
			hash.funcs[i].enabled = false;
		}
		if (hash.funcs[i].enabled) {
			gtk_widget_show(GTK_WIDGET(gui.hash_widgets[i].label));
			gtk_widget_show(GTK_WIDGET(gui.hash_widgets[i].entry_file));
			gtk_widget_show(GTK_WIDGET(gui.hash_widgets[i].entry_text));
			has_enabled = true;
		} else {
			gtk_widget_hide(GTK_WIDGET(gui.hash_widgets[i].label));
			gtk_widget_hide(GTK_WIDGET(gui.hash_widgets[i].entry_file));
			gtk_widget_hide(GTK_WIDGET(gui.hash_widgets[i].entry_text));
		}
	}

	list_update();

	switch (gui_get_view()) {
		case VIEW_FILE:
			gtk_widget_hide(GTK_WIDGET(gui.toolbar));
			gtk_widget_hide(GTK_WIDGET(gui.label_text));
			gtk_widget_hide(GTK_WIDGET(gui.entry));
			gtk_widget_hide(GTK_WIDGET(gui.vbox_list));
			gtk_widget_hide(GTK_WIDGET(gui.vbox_digests_text));
			gtk_widget_show(GTK_WIDGET(gui.label_file));
			gtk_widget_show(GTK_WIDGET(gui.filechooserbutton));
			gtk_widget_show(GTK_WIDGET(gui.vbox_single));
			gtk_widget_show(GTK_WIDGET(gui.vbox_digests_file));
			gtk_widget_show(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_show(GTK_WIDGET(gui.button_hash));

			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
				gui.filechooserbutton));
			if (uri) {
				gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), has_enabled);
				g_free(uri);
			} else
				gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), false);
			break;
		case VIEW_TEXT:
			gtk_widget_hide(GTK_WIDGET(gui.toolbar));
			gtk_widget_hide(GTK_WIDGET(gui.label_file));
			gtk_widget_hide(GTK_WIDGET(gui.filechooserbutton));
			gtk_widget_hide(GTK_WIDGET(gui.vbox_list));
			gtk_widget_hide(GTK_WIDGET(gui.vbox_digests_file));
			gtk_widget_hide(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_hide(GTK_WIDGET(gui.button_hash));
			gtk_widget_show(GTK_WIDGET(gui.label_text));
			gtk_widget_show(GTK_WIDGET(gui.entry));
			gtk_widget_show(GTK_WIDGET(gui.vbox_single));
			gtk_widget_show(GTK_WIDGET(gui.vbox_digests_text));

			gtk_widget_grab_focus(GTK_WIDGET(gui.entry));

			g_signal_emit_by_name(gui.button_hash, "clicked");
			break;
		case VIEW_FILE_LIST:
			gtk_widget_hide(GTK_WIDGET(gui.vbox_single));
			gtk_widget_hide(GTK_WIDGET(gui.hseparator_buttons));
			gtk_widget_show(GTK_WIDGET(gui.toolbar));
			gtk_widget_show(GTK_WIDGET(gui.vbox_list));
			gtk_widget_show(GTK_WIDGET(gui.button_hash));

			gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash),
				list_count_rows());
			break;
		default:
			g_assert_not_reached();
	}

	gtk_window_resize(gui.window,
		CLAMP(prefs.width, 1, prefs.width),
		CLAMP(prefs.height, 1, prefs.height));
}

void gui_clear_digests(void)
{
	switch (gui_get_view()) {
		case VIEW_FILE:
			for (int i = 0; i < HASH_FUNCS_N; i++)
				gtk_entry_set_text(gui.hash_widgets[i].entry_file, "");
			break;
		case VIEW_TEXT:
			for (int i = 0; i < HASH_FUNCS_N; i++)
				gtk_entry_set_text(gui.hash_widgets[i].entry_text, "");
			break;
		case VIEW_FILE_LIST:
			list_clear_digests();
			break;
		default:
			g_assert_not_reached();
	}
}

void gui_set_busy(const bool busy)
{
	gui.busy = busy;

	if (gui_get_view() == VIEW_TEXT)
		return;

	gtk_widget_set_visible(GTK_WIDGET(gui.button_hash), !busy);
	gtk_widget_set_visible(GTK_WIDGET(gui.button_stop), busy);

	gtk_progress_bar_set_fraction(gui.progressbar, 0.0);
	gtk_widget_set_visible(GTK_WIDGET(gui.progressbar), busy);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.hbox_input), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.hbox_output), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.toolbar), !busy);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_text), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_file), !busy);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.radiomenuitem_file_list), !busy);

	gtk_widget_set_sensitive(GTK_WIDGET(gui.dialog_table), !busy);

	if (busy) {
		gtk_window_set_default(gui.window, GTK_WIDGET(gui.button_stop));
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_save_as), false);
	} else {
		gtk_window_set_default(gui.window, GTK_WIDGET(gui.button_hash));
		// User may already have menu open, so make sure save_as gets updated
		g_signal_emit_by_name(gui.menuitem_file, "activate");
	}
}

bool gui_is_maximised(void)
{
	GdkWindow *window = GTK_WIDGET(gui.window)->window;

	if (!window)
		return false;

	GdkWindowState state = gdk_window_get_state(window);

	return (state & GDK_WINDOW_STATE_MAXIMIZED);
}

void gui_chooser_set_uri(const char *uri)
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gui.radiomenuitem_file),
		true);

	gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(gui.filechooserbutton), uri);
}
