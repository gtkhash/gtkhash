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

#ifndef GTKHASH_HASH_DIGEST_FORMAT_H
#define GTKHASH_HASH_DIGEST_FORMAT_H

#include <stdbool.h>

#define DIGEST_FORMATS_N 3
#define DIGEST_FORMAT_IS_VALID(X) (((X) >= 0) && ((X) < DIGEST_FORMATS_N))

enum digest_format_e {
	DIGEST_FORMAT_INVALID = -1,
	DIGEST_FORMAT_HEX_LOWER,
	DIGEST_FORMAT_HEX_UPPER,
	DIGEST_FORMAT_BASE64,
};

enum digest_format_e gtkhash_digest_format_get_id_from_name(const char *name);
const char *gtkhash_digest_format_get_name(enum digest_format_e format);
bool gtkhash_digest_format_compare(const char *str1, const char *str2,
	enum digest_format_e format);

#endif
