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
#include <string.h>
#include <glib.h>

#include "hash-lib.h"
#include "hash-func.h"
#include "hmac.h"

#if ENABLE_BLAKE2
	HASH_LIB_DECL(blake2)
#endif
#if ENABLE_BLAKE3
	HASH_LIB_DECL(blake3)
#endif
#if ENABLE_GCRYPT
	HASH_LIB_DECL(gcrypt)
#endif
	HASH_LIB_DECL(glib)
#if ENABLE_LIBCRYPTO
	HASH_LIB_DECL(libcrypto)
#endif
#if ENABLE_MBEDTLS
	HASH_LIB_DECL(mbedtls)
#endif
#if ENABLE_NETTLE
	HASH_LIB_DECL(nettle)
#endif
#if ENABLE_XXHASH
	HASH_LIB_DECL(xxhash)
#endif
#if ENABLE_ZLIB
	HASH_LIB_DECL(zlib)
#endif

#define HASH_LIB_DATA(LIB) \
	{ \
		.name = G_STRINGIFY(LIB), \
		.is_supported = gtkhash_hash_lib_ ## LIB ## _is_supported, \
		.start = gtkhash_hash_lib_ ## LIB ## _start, \
		.update = gtkhash_hash_lib_ ## LIB ## _update, \
		.stop = gtkhash_hash_lib_ ## LIB ## _stop, \
		.finish = gtkhash_hash_lib_ ## LIB ## _finish, \
	},
static const struct {
	const char * const name;
	bool (* const is_supported)(enum hash_func_e id);
	void (* const start)(struct hash_func_s *func);
	void (* const update)(struct hash_func_s *func, const uint8_t *buffer, size_t size);
	void (* const stop)(struct hash_func_s *func);
	uint8_t *(* const finish)(struct hash_func_s *func, size_t *size);
} hash_lib_data[] = {
	// Ordered by preference (matches meson.build)
#if ENABLE_BLAKE2
	HASH_LIB_DATA(blake2)
#endif
#if ENABLE_BLAKE3
	HASH_LIB_DATA(blake3)
#endif
#if ENABLE_XXHASH
	HASH_LIB_DATA(xxhash)
#endif
#if ENABLE_GCRYPT
	HASH_LIB_DATA(gcrypt)
#endif
#if ENABLE_LIBCRYPTO
	HASH_LIB_DATA(libcrypto)
#endif
#if ENABLE_MBEDTLS
	HASH_LIB_DATA(mbedtls)
#endif
#if ENABLE_NETTLE
	HASH_LIB_DATA(nettle)
#endif
#if ENABLE_ZLIB
	HASH_LIB_DATA(zlib)
#endif
	HASH_LIB_DATA(glib)
};

// Currently selected lib for each hash func
#define HASH_LIB_INVALID (-1)
static int hash_libs[HASH_FUNCS_N];

static void *gtkhash_hash_lib_init_once(G_GNUC_UNUSED void *data)
{
	// Init all lib selections to be invalid
	for (enum hash_func_e id = 0; id < HASH_FUNCS_N; id++)
		hash_libs[id] = HASH_LIB_INVALID;

	const char * const test_lib = g_getenv("GTKHASH_TEST_LIB");

	for (enum hash_func_e id = 0; id < HASH_FUNCS_N; id++) {
		for (size_t lib = 0; lib < G_N_ELEMENTS(hash_lib_data); lib++) {
			if (test_lib && strcmp(hash_lib_data[lib].name, test_lib) != 0)
				continue;
			if (hash_lib_data[lib].is_supported(id)) {
				hash_libs[id] = lib;
				break;
			}
		}
	}

	return NULL;
}

bool gtkhash_hash_lib_is_supported(const enum hash_func_e id)
{
	static GOnce once = G_ONCE_INIT;
	g_once(&once, gtkhash_hash_lib_init_once, NULL);

	return (hash_libs[id] != HASH_LIB_INVALID);
}

void gtkhash_hash_lib_start(struct hash_func_s *func, const uint8_t *hmac_key,
	const size_t key_size)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(!func->lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	hash_lib_data[hash_libs[func->id]].start(func);

	if (hmac_key && func->hmac_supported)
		gtkhash_hmac_start(func, hmac_key, key_size);
}

void gtkhash_hash_lib_update(struct hash_func_s *func, const uint8_t *buffer,
	const size_t size)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->lib_data);
	g_assert(buffer);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	hash_lib_data[hash_libs[func->id]].update(func, buffer, size);
}

void gtkhash_hash_lib_stop(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	hash_lib_data[hash_libs[func->id]].stop(func);
	func->lib_data = NULL;

	if (func->hmac_data)
		gtkhash_hmac_stop(func);
}

void gtkhash_hash_lib_finish(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	size_t size = 0;
	uint8_t *digest = hash_lib_data[hash_libs[func->id]].finish(func, &size);

	gtkhash_hash_func_set_digest(func, digest, size);

	if (func->hmac_data)
		gtkhash_hmac_finish(func);

	func->lib_data = NULL;
}
