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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <xxhash.h>

#include "hash-lib.h"
#include "hash-func.h"

HASH_LIB_DECL(xxhash)

#define LIB_DATA ((union hash_lib_xxhash_u *)func->lib_data)

union hash_lib_xxhash_u {
	XXH32_state_t *xxh32;
	XXH64_state_t *xxh64;
	XXH3_state_t *xxh3;
};

bool gtkhash_hash_lib_xxhash_is_supported(const enum hash_func_e id)
{
	switch (id) {
		case HASH_FUNC_XXH32:
		case HASH_FUNC_XXH64:
		case HASH_FUNC_XXH3:
		case HASH_FUNC_XXH128:
			return true;
		default:
			return false;
	};
}

void gtkhash_hash_lib_xxhash_start(struct hash_func_s *func)
{
	func->lib_data = g_new(union hash_lib_xxhash_u, 1);

	switch (func->id) {
		case HASH_FUNC_XXH32:
			LIB_DATA->xxh32 = XXH32_createState();
			g_assert(LIB_DATA->xxh32);
			XXH32_reset(LIB_DATA->xxh32, 0);
			break;
		case HASH_FUNC_XXH64:
			LIB_DATA->xxh64 = XXH64_createState();
			g_assert(LIB_DATA->xxh64);
			XXH64_reset(LIB_DATA->xxh64, 0);
			break;
		case HASH_FUNC_XXH3:
			LIB_DATA->xxh3 = XXH3_createState();
			g_assert(LIB_DATA->xxh3);
			XXH3_64bits_reset(LIB_DATA->xxh3);
			break;
		case HASH_FUNC_XXH128:
			LIB_DATA->xxh3 = XXH3_createState();
			g_assert(LIB_DATA->xxh3);
			XXH3_128bits_reset(LIB_DATA->xxh3);
			break;

		default:
			g_assert_not_reached();
	}
}

void gtkhash_hash_lib_xxhash_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	switch (func->id) {
		case HASH_FUNC_XXH32:
			XXH32_update(LIB_DATA->xxh32, buffer, size);
			break;
		case HASH_FUNC_XXH64:
			XXH64_update(LIB_DATA->xxh64, buffer, size);
			break;
		case HASH_FUNC_XXH3:
			XXH3_64bits_update(LIB_DATA->xxh3, buffer, size);
			break;
		case HASH_FUNC_XXH128:
			XXH3_128bits_update(LIB_DATA->xxh3, buffer, size);
			break;

		default:
			g_assert_not_reached();
	}
}

void gtkhash_hash_lib_xxhash_stop(struct hash_func_s *func)
{
	switch (func->id) {
		case HASH_FUNC_XXH32:
			XXH32_freeState(LIB_DATA->xxh32);
			break;
		case HASH_FUNC_XXH64:
			XXH64_freeState(LIB_DATA->xxh64);
			break;
		case HASH_FUNC_XXH3:
		case HASH_FUNC_XXH128:
			XXH3_freeState(LIB_DATA->xxh3);
			break;

		default:
			g_assert_not_reached();
	}

	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_xxhash_finish(struct hash_func_s *func, size_t *size)
{
	uint8_t *digest = NULL;

	union {
		XXH32_canonical_t xxh32;
		XXH64_canonical_t xxh64;
		XXH128_canonical_t xxh128;
	} out;

	switch (func->id) {
		case HASH_FUNC_XXH32:
			*size = sizeof(out.xxh32);
			XXH32_canonicalFromHash(&out.xxh32, XXH32_digest(LIB_DATA->xxh32));
			digest = g_memdup2(&out.xxh32, *size);
			XXH32_freeState(LIB_DATA->xxh32);
			break;
		case HASH_FUNC_XXH64:
			*size = sizeof(out.xxh64);
			XXH64_canonicalFromHash(&out.xxh64, XXH64_digest(LIB_DATA->xxh64));
			digest = g_memdup2(&out.xxh64, *size);
			XXH64_freeState(LIB_DATA->xxh64);
			break;
		case HASH_FUNC_XXH3:
			*size = sizeof(out.xxh64);
			XXH64_canonicalFromHash(&out.xxh64, XXH3_64bits_digest(LIB_DATA->xxh3));
			digest = g_memdup2(&out.xxh64, *size);
			XXH3_freeState(LIB_DATA->xxh3);
			break;
		case HASH_FUNC_XXH128:
			*size = sizeof(out.xxh128);
			XXH128_canonicalFromHash(&out.xxh128, XXH3_128bits_digest(LIB_DATA->xxh3));
			digest = g_memdup2(&out.xxh128, *size);
			XXH3_freeState(LIB_DATA->xxh3);
			break;

		default:
			g_assert_not_reached();
	}

	g_free(LIB_DATA);

	return digest;
}
