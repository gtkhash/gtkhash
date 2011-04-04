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
#include <gtk/gtk.h>

#include "callbacks.h"
#include "main.h"
#include "gui.h"
#include "hash.h"
#include "prefs.h"
#include "list.h"
#include "hash/hash-string.h"

static bool on_window_destroy(void)
{
	gtk_main_quit();
	return true;
}

static void on_window_size_request(void)
{
	if (gui_is_maximised())
		return;

	int width, height;
	gtk_window_get_size(gui.window, &width, &height);
	prefs.width = width;
	prefs.height = height;
}

static void on_menuitem_file_activate(void)
{
	if (gui.busy)
		return;

	bool sensitive = false;

	switch (gui_get_view()) {
		case VIEW_FILE:
			for (int i = 0; i < HASH_FUNCS_N; i++) {
				if (hash.funcs[i].enabled &&
					*gtk_entry_get_text(gui.hash_widgets[i].entry_file))
				{
					sensitive = true;
					break;
				}
			}
			break;
		case VIEW_TEXT:
			sensitive = true;
			break;
		case VIEW_FILE_LIST:
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
			g_assert_not_reached();
	}

	gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_save_as), sensitive);
}

static void on_menuitem_save_as_activate(void)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(
		gtk_file_chooser_dialog_new(_("Save Digests"), NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL));
	gtk_file_chooser_set_do_overwrite_confirmation(chooser, true);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(chooser);
		GString *string = g_string_sized_new(1024);

		for (int i = 0; i < HASH_FUNCS_N; i++) {
			if (!hash.funcs[i].enabled)
				continue;

			switch (gui_get_view()) {
				case VIEW_FILE: {
					const char *digest = gtk_entry_get_text(
						gui.hash_widgets[i].entry_file);
					if (digest && *digest)
						g_string_append_printf(string, "# %s\n",
							hash.funcs[i].name);
					else
						continue;
					char *path = gtk_file_chooser_get_filename(
						GTK_FILE_CHOOSER(gui.filechooserbutton));
					char *basename = g_path_get_basename(path);
					g_free(path);
						g_string_append_printf(string, "%s  %s\n",
						gtk_entry_get_text(gui.hash_widgets[i].entry_file),
							basename);
					g_free(basename);
					break;
				}
				case VIEW_TEXT:
					g_string_append_printf(string, "# %s\n", hash.funcs[i].name);
					g_string_append_printf(string, "%s  \"%s\"\n",
						gtk_entry_get_text(gui.hash_widgets[i].entry_text),
						gtk_entry_get_text(gui.entry));
					break;
				case VIEW_FILE_LIST: {
					int prev = -1;
					for (unsigned int row = 0; row < list_count_rows(); row++)
					{
						char *digest = list_get_digest(row, i);
						if (digest && *digest) {
							if (i != prev)
								g_string_append_printf(string, "# %s\n",
									hash.funcs[i].name);
							prev = i;
						} else {
							if (digest)
								g_free(digest);
							prev = i;
							continue;
						}
						char *uri = list_get_uri(row);
						char *basename = g_filename_display_basename(uri);
						g_string_append_printf(string, "%s  %s\n",
							digest, basename);
						g_free(basename);
						g_free(uri);
						g_free(digest);
					}
					break;
				}
				default:
					g_assert_not_reached();
			}
		}

		char *data = g_string_free(string, false);
		g_file_set_contents(filename, data, -1, NULL);

		g_free(data);
		g_free(filename);
	}

	gtk_widget_destroy(GTK_WIDGET(chooser));
}

static void on_menuitem_quit_activate(void)
{
	gtk_main_quit();
}

static void on_menuitem_edit_activate(void)
{
	GtkWidget *widget = gtk_window_get_focus(gui.window);
	bool selection, editable, clipboard;

	if (GTK_IS_ENTRY(widget)) {
		selection = gtk_editable_get_selection_bounds(
			GTK_EDITABLE(widget), NULL, NULL);
		editable = gtk_editable_get_editable(GTK_EDITABLE(widget));
		clipboard = gtk_clipboard_wait_is_text_available(
			gtk_clipboard_get(GDK_NONE));

		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_cut), selection && editable);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_copy), selection);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_paste), editable && clipboard);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_delete), selection && editable);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_select_all), true);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_cut), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_copy), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_paste), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_delete), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gui.menuitem_select_all), false);
	}
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
	gui_update();
}

static void on_menuitem_about_activate(void)
{
	const char *license = {
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 2 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License along\n"
		"with this program; if not, see <http://www.gnu.org/licenses/>.\n"
	};

	const char *authors[] = {
		"Tristan Heaven <tristanheaven@gmail.com>",
		NULL
	};

	gtk_show_about_dialog(
			gui.window,
			"authors", authors,
			"comments", _("A GTK+ utility for computing message digests or checksums."),
			"license", license,
			"program-name", PACKAGE_NAME,
#if ENABLE_NLS
			"translator-credits", _("translator-credits"),
#endif
			"version", VERSION,
			"website", "http://gtkhash.sourceforge.net/",
			NULL);
}

static void on_filechooserbutton_selection_changed(void)
{
	char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(gui.filechooserbutton));

	gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), uri ? true : false);

	gui_clear_digests();
}

static void on_entry_changed(void)
{
	g_signal_emit_by_name(gui.button_hash, "clicked");
}

static void on_toolbutton_add_clicked(void)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(
		gtk_file_chooser_dialog_new(_("Select Files"), NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL));
	gtk_file_chooser_set_select_multiple(chooser, true);
	gtk_file_chooser_set_local_only(chooser, false);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		GSList *uris = gtk_file_chooser_get_uris(chooser);
//		bool has_selected = false;

		for (unsigned int i = 0; i < g_slist_length(uris); i++) {
			char *uri = g_slist_nth(uris, i)->data;
			list_append_row(uri);
			g_free(uri);
//			has_selected = true;
		}

//		gtk_widget_set_sensitive(GTK_WIDGET(gui.button_hash), has_selected);
		g_slist_free(uris);
	}

	gtk_widget_destroy(GTK_WIDGET(chooser));
}

static void on_toolbutton_remove_clicked(void)
{
	list_remove_selection();
}

static void on_toolbutton_clear_clicked(void)
{
	list_clear();
}

static void on_button_hash_clicked(void)
{
	if (gui_get_view() == VIEW_FILE) {
		// XXX: Workaround for when user clicks Cancel in FileChooserDialog and
		// XXX: uri is changed without emitting the "selection-changed" signal
		on_filechooserbutton_selection_changed();
		if (!gtk_widget_get_sensitive(GTK_WIDGET(gui.button_hash)))
			return;
	}

	gui_set_busy(true);
	gui_clear_digests();

	switch (gui_get_view()) {
		case VIEW_FILE: {
			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
				gui.filechooserbutton));
			hash_file_start(uri);
			break;
		}
		case VIEW_TEXT: {
			const char *str = gtk_entry_get_text(gui.entry);
			gtkhash_hash_string(hash.funcs, str);
			gui_set_busy(false);
			break;
		}
		case VIEW_FILE_LIST:
			hash_file_list_start();
			break;
		default:
			g_assert_not_reached();
	}
}

static void on_button_stop_clicked(void)
{
	hash_file_stop();
}

static bool on_dialog_delete_event(void)
{
	gtk_widget_hide(GTK_WIDGET(gui.dialog));
	return true;
}

void callbacks_init(void)
{
	const struct {
		GObject *obj;
		const char *sig;
		GCallback cb;
	} callbacks[] = {
		{ G_OBJECT(gui.window),                  "destroy",           G_CALLBACK(on_window_destroy) },
		{ G_OBJECT(gui.window),                  "destroy-event",     G_CALLBACK(on_window_destroy) },
		{ G_OBJECT(gui.window),                  "delete-event",      G_CALLBACK(on_window_destroy) },
		{ G_OBJECT(gui.window),                  "size-request",      on_window_size_request },
		{ G_OBJECT(gui.menuitem_file),           "activate",          on_menuitem_file_activate },
		{ G_OBJECT(gui.menuitem_save_as),        "activate",          on_menuitem_save_as_activate },
		{ G_OBJECT(gui.menuitem_quit),           "activate",          on_menuitem_quit_activate },
		{ G_OBJECT(gui.menuitem_edit),           "activate",          on_menuitem_edit_activate },
		{ G_OBJECT(gui.menuitem_cut),            "activate",          on_menuitem_cut_activate },
		{ G_OBJECT(gui.menuitem_copy),           "activate",          on_menuitem_copy_activate },
		{ G_OBJECT(gui.menuitem_paste),          "activate",          on_menuitem_paste_activate },
		{ G_OBJECT(gui.menuitem_delete),         "activate",          on_menuitem_delete_activate },
		{ G_OBJECT(gui.menuitem_select_all),     "activate",          on_menuitem_select_all_activate },
		{ G_OBJECT(gui.menuitem_prefs),          "activate",          on_menuitem_prefs_activate },
		{ G_OBJECT(gui.radiomenuitem_file),      "toggled",           on_radiomenuitem_toggled },
		{ G_OBJECT(gui.radiomenuitem_text),      "toggled",           on_radiomenuitem_toggled },
		{ G_OBJECT(gui.radiomenuitem_file_list), "toggled",           on_radiomenuitem_toggled },
		{ G_OBJECT(gui.menuitem_about),          "activate",          on_menuitem_about_activate },
//		file-set isn't emitted when file is deleted
//		{ G_OBJECT(gui.filechooserbutton),       "file-set",          on_filechooserbutton_file_set },
		{ G_OBJECT(gui.filechooserbutton),       "selection-changed", on_filechooserbutton_selection_changed },
		{ G_OBJECT(gui.entry),                   "changed",           on_entry_changed },
		{ G_OBJECT(gui.toolbutton_add),          "clicked",           on_toolbutton_add_clicked },
		{ G_OBJECT(gui.toolbutton_remove),       "clicked",           on_toolbutton_remove_clicked },
		{ G_OBJECT(gui.toolbutton_clear),        "clicked",           on_toolbutton_clear_clicked },
		{ G_OBJECT(gui.button_hash),             "clicked",           on_button_hash_clicked },
		{ G_OBJECT(gui.button_stop),             "clicked",           on_button_stop_clicked },
		{ G_OBJECT(gui.dialog),                  "delete-event",      G_CALLBACK(on_dialog_delete_event) },
		{ G_OBJECT(gui.dialog_button_close),     "clicked",           G_CALLBACK(on_dialog_delete_event) }
	};

	for (unsigned int i = 0; i < G_N_ELEMENTS(callbacks); i++)
		g_signal_connect(callbacks[i].obj, callbacks[i].sig, callbacks[i].cb,
			NULL);
}
