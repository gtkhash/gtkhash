/*
 *   Copyright (C) 2007-2013 Tristan Heaven <tristanheaven@gmail.com>
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
#include <nettle/nettle-meta.h>

#include "hash-lib-nettle.h"
#include "hash-lib.h"
#include "hash-func.h"

#define LIB_DATA ((struct hash_lib_nettle_s *)func->lib_data)

struct hash_lib_nettle_s {
	const struct nettle_hash *meta;
	void *ctx;
};

static bool gtkhash_hash_lib_nettle_set_meta(const enum hash_func_e id,
	const struct nettle_hash **meta)
{
	switch (id) {
		case HASH_FUNC_MD5:
			*meta = &nettle_md5;
			break;
		default:
			return false;
	}

	return true;
}

bool gtkhash_hash_lib_nettle_is_supported(const enum hash_func_e id)
{
	const struct nettle_hash *meta;

	return gtkhash_hash_lib_nettle_set_meta(id, &meta);
}

void gtkhash_hash_lib_nettle_start(struct hash_func_s *func)
{
	func->lib_data = g_new(struct hash_lib_nettle_s, 1);

	gtkhash_hash_lib_nettle_set_meta(func->id, &LIB_DATA->meta);
	LIB_DATA->ctx = g_malloc(LIB_DATA->meta->context_size);

	LIB_DATA->meta->init(LIB_DATA->ctx);
}

void gtkhash_hash_lib_nettle_update(struct hash_func_s *func,
	const uint8_t *buffer, const size_t size)
{
	LIB_DATA->meta->update(LIB_DATA->ctx, size, buffer);
}

void gtkhash_hash_lib_nettle_stop(struct hash_func_s *func)
{
	g_free(LIB_DATA->ctx);
	g_free(LIB_DATA);
}

uint8_t *gtkhash_hash_lib_nettle_finish(struct hash_func_s *func,
	size_t *size)
{
	*size = LIB_DATA->meta->digest_size;
	uint8_t *digest = g_malloc(*size);

	LIB_DATA->meta->digest(LIB_DATA->ctx, *size, digest);

	g_free(LIB_DATA->ctx);
	g_free(LIB_DATA);

	return digest;
}
