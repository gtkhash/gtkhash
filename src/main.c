/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
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

#include "main.h"
#include "hash.h"
#include "gui.h"
#include "list.h"
#include "prefs.h"

static struct {
	bool version;
	char **files;
} opts = {
	.version = false,
	.files = NULL
};

static GOptionEntry entries[] = {
	{
		"version", 'v', 0, G_OPTION_ARG_NONE, &opts.version,
		"Display version information", NULL
	},
	{
		G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &opts.files,
		NULL, NULL
	},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }
};

static char *filename_arg_to_uri(const char *arg)
{
	GFile *file = g_file_new_for_commandline_arg(arg);
	char *uri = g_file_get_uri(file);
	g_object_unref(file);

	return uri;
}

static void parse_opts(int *argc, char ***argv)
{
	GOptionContext *context = g_option_context_new(_("[FILE|URI...]"));

#if ENABLE_NLS
	g_option_context_add_main_entries(context, entries, PACKAGE);
#else
	g_option_context_add_main_entries(context, entries, NULL);
#endif

	g_option_context_add_group(context, gtk_get_option_group(false));

	if (!g_option_context_parse(context, argc, argv, NULL)) {
		printf(_("Unknown option, try %s --help\n"), g_get_prgname());
		exit(EXIT_FAILURE);
	}

	if (opts.version) {
		printf("%s\n", PACKAGE_STRING);
		exit(EXIT_SUCCESS);
	}

	if (opts.files) {
		if (!opts.files[1]) {
			char *uri = filename_arg_to_uri(opts.files[0]);
			gui_chooser_set_uri(uri);
			g_free(uri);
		} else {
			for (int i = 0; opts.files[i]; i++) {
				char *uri = filename_arg_to_uri(opts.files[i]);
				list_append_row(uri);
				g_free(uri);
			}
		}
	}

	g_option_context_free(context);
}

int main(int argc, char *argv[])
{
#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
#endif

	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(&argc, &argv);
	hash_init();
	gui_init();
	list_init();

	prefs_load();

	parse_opts(&argc, &argv);

	gui_run();

	prefs_save();

	gdk_threads_leave();

	return EXIT_SUCCESS;
}
