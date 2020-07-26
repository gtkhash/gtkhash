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
#include <string.h>
#include <stdbool.h>
#include <glib.h>

#include "digest-format.h"

// Returns true if the two digests are equivalent in the specified format
bool gtkhash_digest_format_compare(char const *digest1, const char *digest2,
	enum digest_format_e format)
{
	g_assert(DIGEST_FORMAT_IS_VALID(format));

	if (!digest1 || !digest2 || !*digest1 || !*digest2)
		return false;

	switch (format) {
		case DIGEST_FORMAT_HEX_LOWER:
		case DIGEST_FORMAT_HEX_UPPER:
			return g_ascii_strcasecmp(digest1, digest2) == 0;
		case DIGEST_FORMAT_BASE64:
			return strcmp(digest1, digest2) == 0;
		default:
			g_assert_not_reached();
			return false;
	}
}
