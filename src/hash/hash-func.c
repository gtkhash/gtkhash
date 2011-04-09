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
#include <glib.h>

#include "hash-func.h"
#include "hash-lib.h"

static const char * const hash_func_names[HASH_FUNCS_N] = {
	[HASH_FUNC_MD2]       = "MD2",
	[HASH_FUNC_MD4]       = "MD4",
	[HASH_FUNC_MD5]       = "MD5",
	[HASH_FUNC_SHA1]      = "SHA1",
	[HASH_FUNC_SHA224]    = "SHA224",
	[HASH_FUNC_SHA256]    = "SHA256",
	[HASH_FUNC_SHA384]    = "SHA384",
	[HASH_FUNC_SHA512]    = "SHA512",
	[HASH_FUNC_RIPEMD128] = "RIPEMD128",
	[HASH_FUNC_RIPEMD160] = "RIPEMD160",
	[HASH_FUNC_RIPEMD256] = "RIPEMD256",
	[HASH_FUNC_RIPEMD320] = "RIPEMD320",
	[HASH_FUNC_HAVAL128]  = "HAVAL128",
	[HASH_FUNC_HAVAL160]  = "HAVAL160",
	[HASH_FUNC_HAVAL192]  = "HAVAL192",
	[HASH_FUNC_HAVAL224]  = "HAVAL224",
	[HASH_FUNC_HAVAL256]  = "HAVAL256",
	[HASH_FUNC_TIGER128]  = "TIGER128",
	[HASH_FUNC_TIGER160]  = "TIGER160",
	[HASH_FUNC_TIGER192]  = "TIGER192",
	[HASH_FUNC_GOST]      = "GOST",
	[HASH_FUNC_WHIRLPOOL] = "WHIRLPOOL",
	[HASH_FUNC_SNEFRU128] = "SNEFRU128",
	[HASH_FUNC_SNEFRU256] = "SNEFRU256",
	[HASH_FUNC_CRC32]     = "CRC32",
	[HASH_FUNC_CRC32B]    = "CRC32B",
	[HASH_FUNC_ADLER32]   = "ADLER32"
};

enum hash_func_e gtkhash_hash_func_get_id_from_name(const char *name)
{
	g_assert(name);

	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (g_strcmp0(name, hash_func_names[i]) == 0)
			return i;

	g_warning("unknown hash func name '%s'", name);

	return HASH_FUNC_INVALID;
}

void gtkhash_hash_func_set_digest(struct hash_func_s *func, char *digest)
{
	g_assert(func);

	if (func->priv.digest)
		g_free(func->priv.digest);

	func->priv.digest = digest;
}

const char *gtkhash_hash_func_get_digest(struct hash_func_s *func)
{
	g_assert(func);

	if (func->priv.digest)
		return func->priv.digest;
	else
		return "";
}

static void gtkhash_hash_func_init(struct hash_func_s *func,
	const enum hash_func_e id)
{
	func->id = id;
	func->supported = gtkhash_hash_lib_is_supported(id);
	func->enabled = false;
	func->name = hash_func_names[id];
	func->priv.digest = NULL;
	func->priv.lib_data = NULL;
}

void gtkhash_hash_func_init_all(struct hash_func_s *funcs)
{
	g_assert(funcs);

	for (int i = 0; i < HASH_FUNCS_N; i++)
		gtkhash_hash_func_init(&funcs[i], i);
}

void gtkhash_hash_func_deinit_all(struct hash_func_s *funcs)
{
	g_assert(funcs);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		gtkhash_hash_func_set_digest(&funcs[i], NULL);
		funcs[i].name = NULL;
		funcs[i].id = HASH_FUNC_INVALID;
	}
}
