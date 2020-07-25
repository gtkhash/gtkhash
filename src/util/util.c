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

#include <stdlib.h>
#include <glib.h>

#include "util.h"
#include "../main.h"

static char *gtkhash_format_time_left(const unsigned int s)
{
	if (s > 60) {
		const unsigned int m = s / 60;
		return g_strdup_printf(g_dngettext(GETTEXT_PACKAGE,
			"%u minute left",
			"%u minutes left", m), m);
	} else {
		return g_strdup_printf(g_dngettext(GETTEXT_PACKAGE,
			"%u second left",
			"%u seconds left", s), s);
	}
}

char *gtkhash_format_progress(const goffset file_size,
	const goffset total_read, const double elapsed)
{
	g_assert(file_size > 0);
	g_assert(total_read > 0);
	g_assert(elapsed > 0);

	const unsigned int s = elapsed / total_read * (file_size - total_read);
	char *time_left_str = gtkhash_format_time_left(s);
	char *total_read_str = g_format_size(total_read);
	char *file_size_str = g_format_size(file_size);
	char *speed_str = g_format_size(total_read / elapsed);
	char *text;

	text = g_strdup_printf(_("%s of %s - %s (%s/sec)"),
		total_read_str, file_size_str, time_left_str, speed_str);

	g_free(speed_str);
	g_free(file_size_str);
	g_free(total_read_str);
	g_free(time_left_str);

	return text;
}
