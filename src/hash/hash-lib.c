/*
 *   Copyright (C) 2007-2011 Tristan Heaven <tristanheaven@gmail.com>
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
#include <glib.h>

#include "hash-lib.h"
#include "hash-func.h"
#include "hash-lib-mhash.h"
#include "hash-lib-glib.h"

enum hash_lib_e {
	HASH_LIB_GLIB  = 0,
	HASH_LIB_MHASH = 1
};

static const enum hash_lib_e hash_libs[HASH_FUNCS_N] = {
	[HASH_FUNC_MD2]       = HASH_LIB_MHASH,
	[HASH_FUNC_MD4]       = HASH_LIB_MHASH,
	[HASH_FUNC_MD5]       = HASH_LIB_GLIB,
	[HASH_FUNC_SHA1]      = HASH_LIB_GLIB,
	[HASH_FUNC_SHA224]    = HASH_LIB_MHASH,
	[HASH_FUNC_SHA256]    = HASH_LIB_GLIB,
	[HASH_FUNC_SHA384]    = HASH_LIB_MHASH,
	[HASH_FUNC_SHA512]    = HASH_LIB_MHASH,
	[HASH_FUNC_RIPEMD128] = HASH_LIB_MHASH,
	[HASH_FUNC_RIPEMD160] = HASH_LIB_MHASH,
	[HASH_FUNC_RIPEMD256] = HASH_LIB_MHASH,
	[HASH_FUNC_RIPEMD320] = HASH_LIB_MHASH,
	[HASH_FUNC_HAVAL128]  = HASH_LIB_MHASH,
	[HASH_FUNC_HAVAL160]  = HASH_LIB_MHASH,
	[HASH_FUNC_HAVAL192]  = HASH_LIB_MHASH,
	[HASH_FUNC_HAVAL224]  = HASH_LIB_MHASH,
	[HASH_FUNC_HAVAL256]  = HASH_LIB_MHASH,
	[HASH_FUNC_TIGER128]  = HASH_LIB_MHASH,
	[HASH_FUNC_TIGER160]  = HASH_LIB_MHASH,
	[HASH_FUNC_TIGER192]  = HASH_LIB_MHASH,
	[HASH_FUNC_GOST]      = HASH_LIB_MHASH,
	[HASH_FUNC_WHIRLPOOL] = HASH_LIB_MHASH,
	[HASH_FUNC_SNEFRU128] = HASH_LIB_MHASH,
	[HASH_FUNC_SNEFRU256] = HASH_LIB_MHASH,
	[HASH_FUNC_CRC32]     = HASH_LIB_MHASH,
	[HASH_FUNC_CRC32B]    = HASH_LIB_MHASH,
	[HASH_FUNC_ADLER32]   = HASH_LIB_MHASH
};

void gtkhash_hash_lib_start(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->enabled);
	g_assert(!func->priv.lib_data);

	static void (* const start_funcs[])(struct hash_func_s *) = {
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_start,
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_start
	};

	start_funcs[hash_libs[func->id]](func);
}

void gtkhash_hash_lib_update(struct hash_func_s *func, const uint8_t *buffer,
	const size_t size)
{
	g_assert(func);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);
	g_assert(buffer);

	static void (* const update_funcs[])(struct hash_func_s *,
		const uint8_t *, const size_t) =
	{
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_update,
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_update
	};

	update_funcs[hash_libs[func->id]](func, buffer, size);
}

void gtkhash_hash_lib_stop(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);

	static void (* const stop_funcs[])(struct hash_func_s *) = {
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_stop,
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_stop
	};

	stop_funcs[hash_libs[func->id]](func);
	func->priv.lib_data = NULL;
}

void gtkhash_hash_lib_finish(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);

	static char * (* const finish_libs[])(struct hash_func_s *) = {
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_finish,
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_finish
	};

	char *digest = finish_libs[hash_libs[func->id]](func);
	func->priv.lib_data = NULL;

	gtkhash_hash_func_set_digest(func, digest);
}
