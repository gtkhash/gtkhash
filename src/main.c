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
#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "main.h"
#include "opts.h"
#include "hash.h"
#include "gui.h"
#include "list.h"
#include "prefs.h"
#include "resources.h"
#include "check.h"

int main(int argc, char **argv)
{
#if ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	hash_init();
	atexit(hash_deinit);

	opts_preinit(&argc, &argv);

	gtk_init(NULL, NULL);

	// Init gui using GResource data
	resources_register_resource();
	gui_init();
	atexit(gui_deinit);
	resources_unregister_resource();

	list_init();

	prefs_init();
	atexit(prefs_deinit);

	check_init();
	atexit(check_deinit);

	opts_postinit();

	gui_run();

	return EXIT_SUCCESS;
}
