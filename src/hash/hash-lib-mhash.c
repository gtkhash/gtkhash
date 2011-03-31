/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
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
 *   along with GtkHash. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <mhash.h>

#include "hash-lib-mhash.h"
#include "hash-lib.h"
#include "hash-func.h"

#define LIB_DATA ((struct hash_lib_s *)func->priv.lib_data)

struct hash_lib_s {
	hashid type;
	MHASH thread;
};

void gtkhash_hash_lib_mhash_start(struct hash_func_s *func)
{
	func->priv.lib_data = g_new(struct hash_lib_s, 1);

	switch (func->id) {
		case HASH_FUNC_MD2:
			LIB_DATA->type = MHASH_MD2;
			break;
		case HASH_FUNC_MD4:
			LIB_DATA->type = MHASH_MD4;
			break;
		case HASH_FUNC_MD5:
			LIB_DATA->type = MHASH_MD5;
			break;
		case HASH_FUNC_SHA1:
			LIB_DATA->type = MHASH_SHA1;
			break;
		case HASH_FUNC_SHA224:
			LIB_DATA->type = MHASH_SHA224;
			break;
		case HASH_FUNC_SHA256:
			LIB_DATA->type = MHASH_SHA256;
			break;
		case HASH_FUNC_SHA384:
			LIB_DATA->type = MHASH_SHA384;
			break;
		case HASH_FUNC_SHA512:
			LIB_DATA->type = MHASH_SHA512;
			break;
		case HASH_FUNC_RIPEMD128:
			LIB_DATA->type = MHASH_RIPEMD128;
			break;
		case HASH_FUNC_RIPEMD160:
			LIB_DATA->type = MHASH_RIPEMD160;
			break;
		case HASH_FUNC_RIPEMD256:
			LIB_DATA->type = MHASH_RIPEMD256;
			break;
		case HASH_FUNC_RIPEMD320:
			LIB_DATA->type = MHASH_RIPEMD320;
			break;
		case HASH_FUNC_HAVAL128:
			LIB_DATA->type = MHASH_HAVAL128;
			break;
		case HASH_FUNC_HAVAL160:
			LIB_DATA->type = MHASH_HAVAL160;
			break;
		case HASH_FUNC_HAVAL192:
			LIB_DATA->type = MHASH_HAVAL192;
			break;
		case HASH_FUNC_HAVAL224:
			LIB_DATA->type = MHASH_HAVAL224;
			break;
		case HASH_FUNC_HAVAL256:
			LIB_DATA->type = MHASH_HAVAL256;
			break;
		case HASH_FUNC_TIGER128:
			LIB_DATA->type = MHASH_TIGER128;
			break;
		case HASH_FUNC_TIGER160:
			LIB_DATA->type = MHASH_TIGER160;
			break;
		case HASH_FUNC_TIGER192:
			LIB_DATA->type = MHASH_TIGER192;
			break;
		case HASH_FUNC_GOST:
			LIB_DATA->type = MHASH_GOST;
			break;
		case HASH_FUNC_WHIRLPOOL:
			LIB_DATA->type = MHASH_WHIRLPOOL;
			break;
		case HASH_FUNC_SNEFRU128:
			LIB_DATA->type = MHASH_SNEFRU128;
			break;
		case HASH_FUNC_SNEFRU256:
			LIB_DATA->type = MHASH_SNEFRU256;
			break;
		case HASH_FUNC_CRC32:
			LIB_DATA->type = MHASH_CRC32;
			break;
		case HASH_FUNC_CRC32B:
			LIB_DATA->type = MHASH_CRC32B;
			break;
		case HASH_FUNC_ADLER32:
			LIB_DATA->type = MHASH_ADLER32;
			break;
		default:
			g_assert_not_reached();
	}

	LIB_DATA->thread = mhash_init(LIB_DATA->type);
	g_assert(LIB_DATA->thread != MHASH_FAILED);
}

void gtkhash_hash_lib_mhash_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	mhash(LIB_DATA->thread, buffer, size);
}

void gtkhash_hash_lib_mhash_stop(struct hash_func_s *func)
{
	mhash_deinit(LIB_DATA->thread, NULL);
	g_free(LIB_DATA);
}

char *gtkhash_hash_lib_mhash_finish(struct hash_func_s *func)
{
	uint8_t *bin = mhash_end_m(LIB_DATA->thread, g_malloc);
	GString *digest = g_string_sized_new(128);

	for (unsigned int i = 0; i < mhash_get_block_size(LIB_DATA->type); i++)
		g_string_append_printf(digest, "%.2x", bin[i]);

	g_free(bin);
	g_free(LIB_DATA);

	return g_string_free(digest, false);
}
