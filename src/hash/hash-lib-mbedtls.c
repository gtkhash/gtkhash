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
#include <mbedtls/md.h>

#include "hash-lib.h"
#include "hash-func.h"

HASH_LIB_DECL(mbedtls)

#define LIB_DATA ((struct hash_lib_mbedtls_s *)func->lib_data)

struct hash_lib_mbedtls_s {
	mbedtls_md_context_t ctx;
};

static bool gtkhash_hash_lib_mbedtls_set_type(const enum hash_func_e id,
	mbedtls_md_type_t *type)
{
	switch (id) {
		case HASH_FUNC_MD2:
			*type = MBEDTLS_MD_MD2;
			break;
		case HASH_FUNC_MD4:
			*type = MBEDTLS_MD_MD4;
			break;
		case HASH_FUNC_MD5:
			*type = MBEDTLS_MD_MD5;
			break;
		case HASH_FUNC_RIPEMD160:
			*type = MBEDTLS_MD_RIPEMD160;
			break;
		case HASH_FUNC_SHA1:
			*type = MBEDTLS_MD_SHA1;
			break;
		case HASH_FUNC_SHA224:
			*type = MBEDTLS_MD_SHA224;
			break;
		case HASH_FUNC_SHA256:
			*type = MBEDTLS_MD_SHA256;
			break;
		case HASH_FUNC_SHA384:
			*type = MBEDTLS_MD_SHA384;
			break;
		case HASH_FUNC_SHA512:
			*type = MBEDTLS_MD_SHA512;
			break;
		default:
			return false;
	}

	return true;
}

bool gtkhash_hash_lib_mbedtls_is_supported(const enum hash_func_e id)
{
	mbedtls_md_type_t type;
	if (!gtkhash_hash_lib_mbedtls_set_type(id, &type))
		return false;

	struct hash_lib_mbedtls_s data;
	mbedtls_md_init(&data.ctx);

	const mbedtls_md_info_t *info = mbedtls_md_info_from_type(type);
	if (mbedtls_md_setup(&data.ctx, info, 0) != 0) {
		mbedtls_md_free(&data.ctx);
		return false;
	}

	mbedtls_md_free(&data.ctx);

	return true;
}

void gtkhash_hash_lib_mbedtls_start(struct hash_func_s *func)
{
	func->lib_data = g_new(struct hash_lib_mbedtls_s, 1);

	mbedtls_md_type_t type;
	if (!gtkhash_hash_lib_mbedtls_set_type(func->id, &type))
		g_assert_not_reached();

	mbedtls_md_init(&LIB_DATA->ctx);

	const mbedtls_md_info_t *info = mbedtls_md_info_from_type(type);
	if (mbedtls_md_setup(&LIB_DATA->ctx, info, 0) != 0)
		g_assert_not_reached();

	if (mbedtls_md_starts(&LIB_DATA->ctx) != 0)
		g_assert_not_reached();
}

void gtkhash_hash_lib_mbedtls_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	mbedtls_md_update(&LIB_DATA->ctx, buffer, size);
}

void gtkhash_hash_lib_mbedtls_stop(struct hash_func_s *func)
{
	mbedtls_md_free(&LIB_DATA->ctx);
	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_mbedtls_finish(struct hash_func_s *func,
	size_t *size)
{
	*size = mbedtls_md_get_size(LIB_DATA->ctx.md_info);
	uint8_t *digest = g_malloc(*size);

	if (mbedtls_md_finish(&LIB_DATA->ctx, digest) != 0)
		g_assert_not_reached();

	mbedtls_md_free(&LIB_DATA->ctx);
	g_free(LIB_DATA);

	return digest;
}
