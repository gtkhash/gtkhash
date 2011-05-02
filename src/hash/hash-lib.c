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
#include <string.h>
#include <stdio.h>
#include <glib.h>

#include "hash-lib.h"
#include "hash-func.h"
#include "hash-lib-gcrypt.h"
#include "hash-lib-nss.h"
#include "hash-lib-glib.h"
#include "hash-lib-linux.h"
#include "hash-lib-mhash.h"

enum hash_lib_e {
	HASH_LIB_INVALID,
	HASH_LIB_GCRYPT,
	HASH_LIB_NSS,
	HASH_LIB_GLIB,
	HASH_LIB_LINUX,
	HASH_LIB_MHASH
};

static enum hash_lib_e hash_libs[HASH_FUNCS_N];

static void gtkhash_hash_lib_init_once(void)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
#if ENABLE_LINUX_CRYPTO
		if (!hash_libs[i] && gtkhash_hash_lib_linux_is_supported(i))
			hash_libs[i] = HASH_LIB_LINUX;
#endif
#if ENABLE_GCRYPT
		if (!hash_libs[i] && gtkhash_hash_lib_gcrypt_is_supported(i))
			hash_libs[i] = HASH_LIB_GCRYPT;
#endif
#if ENABLE_NSS
		if (!hash_libs[i] && gtkhash_hash_lib_nss_is_supported(i))
			hash_libs[i] = HASH_LIB_NSS;
#endif
#if ENABLE_GLIB_CHECKSUMS
		if (!hash_libs[i] && gtkhash_hash_lib_glib_is_supported(i))
			hash_libs[i] = HASH_LIB_GLIB;
#endif
#if ENABLE_MHASH
		if (!hash_libs[i] && gtkhash_hash_lib_mhash_is_supported(i))
			hash_libs[i] = HASH_LIB_MHASH;
#endif
	}
}

bool gtkhash_hash_lib_is_supported(const enum hash_func_e id)
{
	static GOnce once = G_ONCE_INIT;
	g_once(&once, (GThreadFunc)gtkhash_hash_lib_init_once, NULL);

	return (hash_libs[id] != HASH_LIB_INVALID);
}

void gtkhash_hash_lib_start(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(!func->priv.lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	static void (* const start_funcs[])(struct hash_func_s *) = {
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_start,
#endif
#if ENABLE_NSS
		[HASH_LIB_NSS] = gtkhash_hash_lib_nss_start,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_start,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_start,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_start,
#endif
	};

	start_funcs[hash_libs[func->id]](func);
}

void gtkhash_hash_lib_update(struct hash_func_s *func, const uint8_t *buffer,
	const size_t size)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);
	g_assert(buffer);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	static void (* const update_funcs[])(struct hash_func_s *,
		const uint8_t *, const size_t) =
	{
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_update,
#endif
#if ENABLE_NSS
		[HASH_LIB_NSS] = gtkhash_hash_lib_nss_update,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_update,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_update,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_update,
#endif
	};

	update_funcs[hash_libs[func->id]](func, buffer, size);
}

void gtkhash_hash_lib_stop(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	static void (* const stop_funcs[])(struct hash_func_s *) = {
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_stop,
#endif
#if ENABLE_NSS
		[HASH_LIB_NSS] = gtkhash_hash_lib_nss_stop,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_stop,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_stop,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_stop,
#endif
	};

	stop_funcs[hash_libs[func->id]](func);
	func->priv.lib_data = NULL;
}

void gtkhash_hash_lib_finish(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->priv.lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	static char * (* const finish_libs[])(struct hash_func_s *) = {
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_finish,
#endif
#if ENABLE_NSS
		[HASH_LIB_NSS] = gtkhash_hash_lib_nss_finish,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB]  = gtkhash_hash_lib_glib_finish,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_finish,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_finish,
#endif
	};

	char *digest = finish_libs[hash_libs[func->id]](func);
	func->priv.lib_data = NULL;

	gtkhash_hash_func_set_digest(func, digest);
}

char *gtkhash_hash_lib_bin_to_hex(const uint8_t *bin, const size_t size)
{
	g_assert(bin);
	g_assert(size);

	char *digest = g_malloc0((size * 2) + 1);

	for (size_t i = 0; i < size; i++)
		snprintf(digest + (i * 2), 3, "%.2x", bin[i]);

	g_assert(strlen(digest) == (size * 2));

	return digest;
}
