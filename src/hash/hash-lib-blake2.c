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
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>

#include "blake2/blake2.h"

#include "hash-lib.h"
#include "hash-func.h"

HASH_LIB_DECL(blake2)

#define LIB_DATA ((union hash_lib_blake2_u *)func->lib_data)

union hash_lib_blake2_u {
	blake2b_state b;
	blake2s_state s;
	blake2bp_state bp;
	blake2sp_state sp;
};

bool gtkhash_hash_lib_blake2_is_supported(const enum hash_func_e id)
{
	switch (id) {
		case HASH_FUNC_BLAKE2B:
		case HASH_FUNC_BLAKE2S:
		case HASH_FUNC_BLAKE2BP:
		case HASH_FUNC_BLAKE2SP:
			return true;

		default:
			return false;
	}
}

void gtkhash_hash_lib_blake2_start(struct hash_func_s *func)
{
	func->lib_data = g_new(union hash_lib_blake2_u, 1);

	switch (func->id) {
		case HASH_FUNC_BLAKE2B:
			blake2b_init(&LIB_DATA->b, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2S:
			blake2s_init(&LIB_DATA->s, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2BP:
			blake2bp_init(&LIB_DATA->bp, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2SP:
			blake2sp_init(&LIB_DATA->sp, func->digest_size);
			break;

		default:
			g_assert_not_reached();
	}
}

void gtkhash_hash_lib_blake2_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	switch (func->id) {
		case HASH_FUNC_BLAKE2B:
			blake2b_update(&LIB_DATA->b, buffer, size);
			break;
		case HASH_FUNC_BLAKE2S:
			blake2s_update(&LIB_DATA->s, buffer, size);
			break;
		case HASH_FUNC_BLAKE2BP:
			blake2bp_update(&LIB_DATA->bp, buffer, size);
			break;
		case HASH_FUNC_BLAKE2SP:
			blake2sp_update(&LIB_DATA->sp, buffer, size);
			break;

		default:
			g_assert_not_reached();
	}
}

void gtkhash_hash_lib_blake2_stop(struct hash_func_s *func)
{
	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_blake2_finish(struct hash_func_s *func, size_t *size)
{
	uint8_t *digest = g_malloc(func->digest_size);

	switch (func->id) {
		case HASH_FUNC_BLAKE2B:
			blake2b_final(&LIB_DATA->b, digest, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2S:
			blake2s_final(&LIB_DATA->s, digest, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2BP:
			blake2bp_final(&LIB_DATA->bp, digest, func->digest_size);
			break;
		case HASH_FUNC_BLAKE2SP:
			blake2sp_final(&LIB_DATA->sp, digest, func->digest_size);
			break;

		default:
			g_assert_not_reached();
	}

	g_free(LIB_DATA);

	*size = func->digest_size;
	return digest;
}
