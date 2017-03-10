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
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of GtkHash with the OpenSSL library (or
 *   with modified versions of it that use the same license as the OpenSSL
 *   library), and distribute the linked executables. You must obey the GNU
 *   General Public License in all respects for all of the code used other
 *   than OpenSSL. If you modify this file, you may extend this exception to
 *   your version of the file, but you are not obligated to do so. If you do
 *   not wish to do so, delete this exception statement from your version.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <openssl/evp.h>

#include "hash-lib.h"
#include "hash-func.h"

HASH_LIB_DECL(crypto)

#define LIB_DATA ((struct hash_lib_crypto_s *)func->lib_data)

struct hash_lib_crypto_s {
	const EVP_MD *md;
	EVP_MD_CTX *ctx;
};

static const EVP_MD *gtkhash_hash_lib_crypto_get_md(const enum hash_func_e id)
{
	switch (id) {
#ifndef OPENSSL_NO_BLAKE2
		case HASH_FUNC_BLAKE2B:
			return EVP_blake2b512();
		case HASH_FUNC_BLAKE2S:
			return EVP_blake2s256();
#endif
#ifndef OPENSSL_NO_MD2
		case HASH_FUNC_MD2:
			return EVP_md2();
#endif
#ifndef OPENSSL_NO_MD4
		case HASH_FUNC_MD4:
			return EVP_md4();
#endif
#ifndef OPENSSL_NO_MD5
		case HASH_FUNC_MD5:
			return EVP_md5();
#endif
#ifndef OPENSSL_NO_MDC2
		case HASH_FUNC_MDC2:
			return EVP_mdc2();
#endif
#ifndef OPENSSL_NO_RMD160
		case HASH_FUNC_RIPEMD160:
			return EVP_ripemd160();
#endif
		case HASH_FUNC_SHA1:
			return EVP_sha1();
		case HASH_FUNC_SHA224:
			return EVP_sha224();
		case HASH_FUNC_SHA256:
			return EVP_sha256();
		case HASH_FUNC_SHA384:
			return EVP_sha384();
		case HASH_FUNC_SHA512:
			return EVP_sha512();
#ifndef OPENSSL_NO_WHIRLPOOL
		case HASH_FUNC_WHIRLPOOL:
			return EVP_whirlpool();
#endif
		default:
			return NULL;
	}

	g_assert_not_reached();
}

bool gtkhash_hash_lib_crypto_is_supported(const enum hash_func_e id)
{
	return gtkhash_hash_lib_crypto_get_md(id) != NULL;
}

void gtkhash_hash_lib_crypto_start(struct hash_func_s *func)
{
	func->lib_data = g_new(struct hash_lib_crypto_s, 1);

	if (!(LIB_DATA->md = gtkhash_hash_lib_crypto_get_md(func->id)))
		g_assert_not_reached();

	if (!(LIB_DATA->ctx = EVP_MD_CTX_new()))
		g_assert_not_reached();

	if (EVP_DigestInit_ex(LIB_DATA->ctx, LIB_DATA->md, NULL) != 1)
		g_assert_not_reached();
}

void gtkhash_hash_lib_crypto_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	EVP_DigestUpdate(LIB_DATA->ctx, buffer, size);
}

void gtkhash_hash_lib_crypto_stop(struct hash_func_s *func)
{
	EVP_MD_CTX_free(LIB_DATA->ctx);
	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_crypto_finish(struct hash_func_s *func, size_t *size)
{
	*size = EVP_MD_size(LIB_DATA->md);
	g_assert(*size > 0);

	uint8_t *digest = g_malloc(*size);

	unsigned int len;
	if (EVP_DigestFinal_ex(LIB_DATA->ctx, digest, &len) != 1)
		g_assert_not_reached();
	g_assert(*size == len);

	EVP_MD_CTX_free(LIB_DATA->ctx);
	g_free(LIB_DATA);

	return digest;
}
