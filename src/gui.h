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

#ifndef GTKHASH_GUI_H
#define GTKHASH_GUI_H

#include <stdbool.h>
#include <gtk/gtk.h>

#include "hash.h"

enum gui_view_e {
	VIEW_FILE,
	VIEW_TEXT,
	VIEW_FILE_LIST
};

struct {
	bool busy;
	GtkWindow *window;
	GtkMenuItem *menuitem_file, *menuitem_save_as, *menuitem_quit;
	GtkMenuItem *menuitem_edit;
	GtkMenuItem *menuitem_cut, *menuitem_copy, *menuitem_paste;
	GtkMenuItem *menuitem_delete, *menuitem_select_all, *menuitem_prefs;
	GtkMenuItem *menuitem_about;
	GtkRadioMenuItem *radiomenuitem_file, *radiomenuitem_text, *radiomenuitem_file_list;
	GtkToolbar *toolbar;
	GtkToolButton *toolbutton_add, *toolbutton_remove, *toolbutton_clear;
	GtkVBox *vbox_single, *vbox_list;
	GtkHBox *hbox_input, *hbox_output;
	GtkVBox *vbox_outputlabels, *vbox_digests_file, *vbox_digests_text;
	GtkEntry *entry;
	GtkFileChooserButton *filechooserbutton;
	GtkLabel *label_text, *label_file;
	GtkTreeView *treeview;
	GtkTreeModel *treemodel;
	GtkListStore *liststore;
	GtkHSeparator *hseparator_buttons;
	GtkProgressBar *progressbar;
	GtkButton *button_hash, *button_stop;
	GtkDialog *dialog;
	GtkTable *dialog_table;
	GtkButton *dialog_button_close;
	struct {
		GtkToggleButton *button;
		GtkLabel *label;
		GtkEntry *entry_file, *entry_text;
	} hash_widgets[HASH_FUNCS_N];
} gui;

void gui_init(void);
void gui_run(void);
enum gui_view_e gui_get_view(void);
void gui_update(void);
void gui_clear_digests(void);
void gui_set_busy(const bool set);
bool gui_is_maximised(void);
void gui_chooser_set_uri(const char *path);

#endif
