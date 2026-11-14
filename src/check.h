/*
 *   Copyright (C) 2007-2026 Tristan Heaven <tristan@tristanheaven.net>
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

#ifndef GTKHASH_CHECK_H
#define GTKHASH_CHECK_H

#include <gtk/gtk.h>

#define CHECK_FORMATS_N 3
#define CHECK_FORMAT_IS_VALID(X) (((X) >= 0) && ((X) < CHECK_FORMATS_N))

enum check_format_e {
	CHECK_FORMAT_COMMENT = -2,
	CHECK_FORMAT_INVALID = -1,
	CHECK_FORMAT_BSD,
	CHECK_FORMAT_GNU,
	CHECK_FORMAT_SFV,
};

GSList *check_file_load(GSList *ud_list, GFile *file);
void check_file_save(const char *filename);
void check_file_add_filters(GtkFileFilter *filter);
void check_init(void);
void check_deinit(void);

#endif
