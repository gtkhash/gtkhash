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
#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "main.h"
#include "hash.h"
#include "gui.h"
#include "list.h"
#include "prefs.h"

static struct {
	char *check;
	char *datadir;
	char *text;
	char **files;
	gboolean version;
} opts = {
	.check = NULL,
	.datadir = NULL,
	.text = NULL,
	.files = NULL,
	.version = false,
};

static void free_opts(void)
{
	if (opts.check) {
		g_free(opts.check);
		opts.check = NULL;
	}

	if (opts.datadir) {
		g_free(opts.datadir);
		opts.datadir = NULL;
	}

	if (opts.text) {
		g_free(opts.text);
		opts.text = NULL;
	}

	if (opts.files) {
		g_strfreev(opts.files);
		opts.files = NULL;
	}
}

static char *filename_arg_to_uri(const char *arg)
{
	GFile *file = g_file_new_for_commandline_arg(arg);
	char *uri = g_file_get_uri(file);
	g_object_unref(file);

	return uri;
}

static void read_opts_preinit(int *argc, char ***argv)
{
	GOptionEntry entries[] = {
		{
			"check", 'c', 0, G_OPTION_ARG_STRING, &opts.check,
			C_("gtkhash --help",
				"Check against the specified digest or checksum"),
			C_("gtkhash --help", "DIGEST")
		},
		{
			"datadir", 'd', 0, G_OPTION_ARG_FILENAME, &opts.datadir,
			C_("gtkhash --help",
				"Read program data from the specified directory"),
			C_("gtkhash --help", "DIRECTORY")
		},
		{
			"text", 't', 0, G_OPTION_ARG_STRING, &opts.text,
			C_("gtkhash --help", "Hash the specified text"),
			C_("gtkhash --help", "TEXT")
		},
		{
			"version", 'v', 0, G_OPTION_ARG_NONE, &opts.version,
			C_("gtkhash --help", "Show version information"), NULL
		},
		{
			G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &opts.files,
			NULL, C_("gtkhash --help", "[FILE|URI...]")
		},
		{ NULL, 0, 0, 0, NULL, NULL, NULL }
	};

	GOptionContext *context = g_option_context_new(NULL);
	GError *error = NULL;

	atexit(free_opts);

	g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(false));
	g_option_context_parse(context, argc, argv, &error);
	g_option_context_free(context);

	if (error) {
		g_warning("%s", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	if (opts.version) {
		printf("%s\n", PACKAGE_STRING);
		exit(EXIT_SUCCESS);
	}
}

static void read_opts_postinit(void)
{
	if (opts.check && *opts.check)
		gui_add_check(opts.check);

	unsigned int files_added = 0;
	if (opts.files) {

		GSList *uris = NULL;

		for (int i = 0; opts.files[i]; i++)
			uris = g_slist_prepend(uris, filename_arg_to_uri(opts.files[i]));

		uris = g_slist_reverse(uris);

		files_added = gui_add_uris(uris, GUI_VIEW_INVALID);

		g_slist_free_full(uris, g_free);

		if (files_added)
			gui_start_hash();
	}

	if (!files_added && opts.text) {
		gui_add_text(opts.text);
	}

	free_opts();
}

int main(int argc, char **argv)
{
#if ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	read_opts_preinit(&argc, &argv);

	hash_init();
	atexit(hash_deinit);

	gui_init(opts.datadir ? opts.datadir : DATADIR);
	atexit(gui_deinit);

	list_init();

	prefs_init();
	atexit(prefs_deinit);

	read_opts_postinit();

	gui_run();

	return EXIT_SUCCESS;
}
