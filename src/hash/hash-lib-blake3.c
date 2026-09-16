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
#include <blake3.h>

#include "hash-lib.h"
#include "hash-func.h"

HASH_LIB_DECL(blake3)

#define LIB_DATA ((struct hash_lib_blake3_s *)func->lib_data)

struct hash_lib_blake3_s {
	blake3_hasher hasher;
};

bool gtkhash_hash_lib_blake3_is_supported(const enum hash_func_e id)
{
	switch (id) {
		case HASH_FUNC_BLAKE3:
			return true;

		default:
			return false;
	}
}

void gtkhash_hash_lib_blake3_start(struct hash_func_s *func)
{
	func->lib_data = g_new(struct hash_lib_blake3_s, 1);

	blake3_hasher_init(&LIB_DATA->hasher);
}

void gtkhash_hash_lib_blake3_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
#ifdef BLAKE3_USE_TBB
	blake3_hasher_update_tbb(&LIB_DATA->hasher, buffer, size);
#else
	blake3_hasher_update(&LIB_DATA->hasher, buffer, size);
#endif
}

void gtkhash_hash_lib_blake3_stop(struct hash_func_s *func)
{
	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_blake3_finish(struct hash_func_s *func, size_t *size)
{
	uint8_t *digest = g_malloc(BLAKE3_OUT_LEN);

	blake3_hasher_finalize(&LIB_DATA->hasher, digest, BLAKE3_OUT_LEN);
	g_free(LIB_DATA);

	*size = BLAKE3_OUT_LEN;
	return digest;
}
