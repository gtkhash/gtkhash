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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

#include "digest.h"

// Returns a newly-allocated lowercase hexadecimal digest string
static char *gtkhash_digest_get_hex_lower(struct digest_s *digest)
{
	char *ret = g_malloc((digest->size * 2) + 1);

	for (uint8_t i = 0; i < digest->size; i++)
		snprintf(&ret[i * 2], 3, "%.2x", digest->bin[i]);

	return ret;
}

// Returns a newly-allocated uppercase hexadecimal digest string
static char *gtkhash_digest_get_hex_upper(struct digest_s *digest)
{
	char *ret = g_malloc((digest->size * 2) + 1);

	for (uint8_t i = 0; i < digest->size; i++)
		snprintf(&ret[i * 2], 3, "%.2X", digest->bin[i]);

	return ret;
}

// Returns a newly-allocated digest_s structure
struct digest_s *gtkhash_digest_new(void)
{
	struct digest_s *digest = g_new(struct digest_s, 1);

	digest->bin = NULL;
	digest->size = 0;

	return digest;
}

// Sets the stored digest using the given binary value
void gtkhash_digest_set_data(struct digest_s *digest, uint8_t *bin,
	const size_t size)
{
	g_assert(digest);
	g_assert(bin);
	g_assert(size);

	gtkhash_digest_free_data(digest);

	digest->bin = bin;
	digest->size = size;
}

// Returns a newly-allocated digest string in the requested format
char *gtkhash_digest_get_data(struct digest_s *digest,
	const enum digest_format_e format)
{
	g_assert(digest);
	g_assert(DIGEST_FORMAT_IS_VALID(format));

	switch (format) {
		case DIGEST_FORMAT_HEX_LOWER:
			return gtkhash_digest_get_hex_lower(digest);
		case DIGEST_FORMAT_HEX_UPPER:
			return gtkhash_digest_get_hex_upper(digest);
		case DIGEST_FORMAT_BASE64:
			return g_base64_encode(digest->bin, digest->size);

		default:
			g_assert_not_reached();
			return NULL;
	}
}

// Resets a digest_s structure back to its original state
void gtkhash_digest_free_data(struct digest_s *digest)
{
	g_assert(digest);

	if (digest->bin) {
		g_free(digest->bin);
		digest->bin = NULL;
	}

	digest->size = 0;
}

// Frees a digest_s structure
void gtkhash_digest_free(struct digest_s *digest)
{
	g_assert(digest);

	gtkhash_digest_free_data(digest);
	g_free(digest);
}
