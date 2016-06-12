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
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

#include "digest.h"

// Returns a newly-allocated string containing a digest in hex
static char *gtkhash_digest_get_hex(struct digest_s *digest, bool upper)
{
	char *ret = g_malloc0((digest->size * 2) + 1);
	const char *format_str = upper ? "%.2X" : "%.2x";

	for (size_t i = 0; i < digest->size; i++)
		snprintf(&ret[i * 2], 3, format_str, digest->bin[i]);

	return ret;
}

// Returns a newly-allocated string containing a digest in base64
static char *gtkhash_digest_get_base64(struct digest_s *digest)
{
	return g_base64_encode(digest->bin, digest->size);
}

// Returns a newly-allocated digest_s structure
struct digest_s *gtkhash_digest_new(void)
{
	struct digest_s *digest = g_new(struct digest_s, 1);

	digest->bin = NULL;
	digest->size = 0;

	for (int i = 0; i < DIGEST_FORMATS_N; i++)
		digest->data[i] = NULL;

	return digest;
}

// Sets all stored digest representations using the given binary value
void gtkhash_digest_set_data(struct digest_s *digest, uint8_t *bin,
	size_t size)
{
	g_assert(digest);
	g_assert(bin);
	g_assert(size);

	gtkhash_digest_free_data(digest);

	digest->bin = bin;
	digest->size = size;

	digest->data[DIGEST_FORMAT_HEX_LOWER] = gtkhash_digest_get_hex(digest, false);
	digest->data[DIGEST_FORMAT_HEX_UPPER] = gtkhash_digest_get_hex(digest, true);
	digest->data[DIGEST_FORMAT_BASE64] = gtkhash_digest_get_base64(digest);
}

// Returns a digest string represented in the requested format
const char *gtkhash_digest_get_data(struct digest_s *digest,
	const enum digest_format_e format)
{
	g_assert(digest);
	g_assert(DIGEST_FORMAT_IS_VALID(format));

	return digest->data[format];
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

	for (int i = 0; i < DIGEST_FORMATS_N; i++) {
		if (digest->data[i]) {
			g_free(digest->data[i]);
			digest->data[i] = NULL;
		}
	}
}

// Frees a digest_s structure
void gtkhash_digest_free(struct digest_s *digest)
{
	g_assert(digest);

	gtkhash_digest_free_data(digest);
	g_free(digest);
}
