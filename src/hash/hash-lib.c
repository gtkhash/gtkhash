/*
 *   Copyright (C) 2007-2017 Tristan Heaven <tristan@tristanheaven.net>
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

#include "hash-lib.h"
#include "hash-func.h"
#include "hmac.h"

#if ENABLE_BLAKE2
	HASH_LIB_DECL(blake2)
#endif
#if ENABLE_GCRYPT
	HASH_LIB_DECL(gcrypt)
#endif
#if ENABLE_GLIB_CHECKSUMS
	HASH_LIB_DECL(glib)
#endif
#if ENABLE_LIBCRYPTO
	HASH_LIB_DECL(crypto)
#endif
#if ENABLE_LINUX_CRYPTO
	HASH_LIB_DECL(linux)
#endif
#if ENABLE_MBEDTLS
	HASH_LIB_DECL(mbedtls)
#endif
#if ENABLE_MD6
	HASH_LIB_DECL(md6)
#endif
#if ENABLE_MHASH
	HASH_LIB_DECL(mhash)
#endif
#if ENABLE_NETTLE
	HASH_LIB_DECL(nettle)
#endif
#if ENABLE_ZLIB
	HASH_LIB_DECL(zlib)
#endif

enum hash_lib_e {
	HASH_LIB_INVALID = -1,
#if ENABLE_BLAKE2
	HASH_LIB_BLAKE2,
#endif
#if ENABLE_GCRYPT
	HASH_LIB_GCRYPT,
#endif
#if ENABLE_GLIB_CHECKSUMS
	HASH_LIB_GLIB,
#endif
#if ENABLE_LIBCRYPTO
	HASH_LIB_CRYPTO,
#endif
#if ENABLE_LINUX_CRYPTO
	HASH_LIB_LINUX,
#endif
#if ENABLE_MBEDTLS
	HASH_LIB_MBEDTLS,
#endif
#if ENABLE_MD6
	HASH_LIB_MD6,
#endif
#if ENABLE_MHASH
	HASH_LIB_MHASH,
#endif
#if ENABLE_NETTLE
	HASH_LIB_NETTLE,
#endif
#if ENABLE_ZLIB
	HASH_LIB_ZLIB,
#endif
};

// Currently selected lib for each hash func
static enum hash_lib_e hash_libs[HASH_FUNCS_N];

static void gtkhash_hash_lib_init_once(void)
{
	// Init all lib selections to be invalid
	for (enum hash_func_e i = 0; i < HASH_FUNCS_N; i++)
		hash_libs[i] = HASH_LIB_INVALID;

	// Note: Preferred lib selections are defined by the order used here
	for (enum hash_func_e i = 0; i < HASH_FUNCS_N; i++) {
#if ENABLE_ZLIB
		if (gtkhash_hash_lib_zlib_is_supported(i)) {
			hash_libs[i] = HASH_LIB_ZLIB;
			continue;
		}
#endif
#if ENABLE_BLAKE2
		if (gtkhash_hash_lib_blake2_is_supported(i)) {
			hash_libs[i] = HASH_LIB_BLAKE2;
			continue;
		}
#endif
#if ENABLE_GCRYPT
		if (gtkhash_hash_lib_gcrypt_is_supported(i)) {
			hash_libs[i] = HASH_LIB_GCRYPT;
			continue;
		}
#endif
#if ENABLE_LIBCRYPTO
		if (gtkhash_hash_lib_crypto_is_supported(i)) {
			hash_libs[i] = HASH_LIB_CRYPTO;
			continue;
		}
#endif
#if ENABLE_MBEDTLS
		if (gtkhash_hash_lib_mbedtls_is_supported(i)) {
			hash_libs[i] = HASH_LIB_MBEDTLS;
			continue;
		}
#endif
#if ENABLE_NETTLE
		if (gtkhash_hash_lib_nettle_is_supported(i)) {
			hash_libs[i] = HASH_LIB_NETTLE;
			continue;
		}
#endif
#if ENABLE_LINUX_CRYPTO
		if (gtkhash_hash_lib_linux_is_supported(i)) {
			hash_libs[i] = HASH_LIB_LINUX;
			continue;
		}
#endif
#if ENABLE_GLIB_CHECKSUMS
		if (gtkhash_hash_lib_glib_is_supported(i)) {
			hash_libs[i] = HASH_LIB_GLIB;
			continue;
		}
#endif
#if ENABLE_MHASH
		if (gtkhash_hash_lib_mhash_is_supported(i)) {
			hash_libs[i] = HASH_LIB_MHASH;
			continue;
		}
#endif
#if ENABLE_MD6
		if (gtkhash_hash_lib_md6_is_supported(i)) {
			hash_libs[i] = HASH_LIB_MD6;
			continue;
		}
#endif
	}
}

bool gtkhash_hash_lib_is_supported(const enum hash_func_e id)
{
	static GOnce once = G_ONCE_INIT;
	g_once(&once, (GThreadFunc)gtkhash_hash_lib_init_once, NULL);

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

	static void (* const start_funcs[])(struct hash_func_s *) = {
#if ENABLE_BLAKE2
		[HASH_LIB_BLAKE2] = gtkhash_hash_lib_blake2_start,
#endif
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_start,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB] = gtkhash_hash_lib_glib_start,
#endif
#if ENABLE_LIBCRYPTO
		[HASH_LIB_CRYPTO] = gtkhash_hash_lib_crypto_start,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_start,
#endif
#if ENABLE_MBEDTLS
		[HASH_LIB_MBEDTLS] = gtkhash_hash_lib_mbedtls_start,
#endif
#if ENABLE_MD6
		[HASH_LIB_MD6] = gtkhash_hash_lib_md6_start,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_start,
#endif
#if ENABLE_NETTLE
		[HASH_LIB_NETTLE] = gtkhash_hash_lib_nettle_start,
#endif
#if ENABLE_ZLIB
		[HASH_LIB_ZLIB] = gtkhash_hash_lib_zlib_start,
#endif
	};

	start_funcs[hash_libs[func->id]](func);

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

	static void (* const update_funcs[])(struct hash_func_s *,
		const uint8_t *, const size_t) =
	{
#if ENABLE_BLAKE2
		[HASH_LIB_BLAKE2] = gtkhash_hash_lib_blake2_update,
#endif
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_update,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB] = gtkhash_hash_lib_glib_update,
#endif
#if ENABLE_LIBCRYPTO
		[HASH_LIB_CRYPTO] = gtkhash_hash_lib_crypto_update,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_update,
#endif
#if ENABLE_MBEDTLS
		[HASH_LIB_MBEDTLS] = gtkhash_hash_lib_mbedtls_update,
#endif
#if ENABLE_MD6
		[HASH_LIB_MD6] = gtkhash_hash_lib_md6_update,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_update,
#endif
#if ENABLE_NETTLE
		[HASH_LIB_NETTLE] = gtkhash_hash_lib_nettle_update,
#endif
#if ENABLE_ZLIB
		[HASH_LIB_ZLIB] = gtkhash_hash_lib_zlib_update,
#endif
	};

	update_funcs[hash_libs[func->id]](func, buffer, size);
}

void gtkhash_hash_lib_stop(struct hash_func_s *func)
{
	g_assert(func);
	g_assert(func->supported);
	g_assert(func->enabled);
	g_assert(func->lib_data);
	g_assert(hash_libs[func->id] != HASH_LIB_INVALID);

	static void (* const stop_funcs[])(struct hash_func_s *) = {
#if ENABLE_BLAKE2
		[HASH_LIB_BLAKE2] = gtkhash_hash_lib_blake2_stop,
#endif
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_stop,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB] = gtkhash_hash_lib_glib_stop,
#endif
#if ENABLE_LIBCRYPTO
		[HASH_LIB_CRYPTO] = gtkhash_hash_lib_crypto_stop,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_stop,
#endif
#if ENABLE_MBEDTLS
		[HASH_LIB_MBEDTLS] = gtkhash_hash_lib_mbedtls_stop,
#endif
#if ENABLE_MD6
		[HASH_LIB_MD6] = gtkhash_hash_lib_md6_stop,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_stop,
#endif
#if ENABLE_NETTLE
		[HASH_LIB_NETTLE] = gtkhash_hash_lib_nettle_stop,
#endif
#if ENABLE_ZLIB
		[HASH_LIB_ZLIB] = gtkhash_hash_lib_zlib_stop,
#endif
	};

	stop_funcs[hash_libs[func->id]](func);
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

	static uint8_t *(* const finish_libs[])(struct hash_func_s *, size_t *) = {
#if ENABLE_BLAKE2
		[HASH_LIB_BLAKE2] = gtkhash_hash_lib_blake2_finish,
#endif
#if ENABLE_GCRYPT
		[HASH_LIB_GCRYPT] = gtkhash_hash_lib_gcrypt_finish,
#endif
#if ENABLE_GLIB_CHECKSUMS
		[HASH_LIB_GLIB] = gtkhash_hash_lib_glib_finish,
#endif
#if ENABLE_LIBCRYPTO
		[HASH_LIB_CRYPTO] = gtkhash_hash_lib_crypto_finish,
#endif
#if ENABLE_LINUX_CRYPTO
		[HASH_LIB_LINUX] = gtkhash_hash_lib_linux_finish,
#endif
#if ENABLE_MBEDTLS
		[HASH_LIB_MBEDTLS] = gtkhash_hash_lib_mbedtls_finish,
#endif
#if ENABLE_MD6
		[HASH_LIB_MD6] = gtkhash_hash_lib_md6_finish,
#endif
#if ENABLE_MHASH
		[HASH_LIB_MHASH] = gtkhash_hash_lib_mhash_finish,
#endif
#if ENABLE_NETTLE
		[HASH_LIB_NETTLE] = gtkhash_hash_lib_nettle_finish,
#endif
#if ENABLE_ZLIB
		[HASH_LIB_ZLIB] = gtkhash_hash_lib_zlib_finish,
#endif
	};

	size_t size = 0;
	uint8_t *digest = finish_libs[hash_libs[func->id]](func, &size);

	gtkhash_hash_func_set_digest(func, digest, size);

	if (func->hmac_data)
		gtkhash_hmac_finish(func);

	func->lib_data = NULL;
}
