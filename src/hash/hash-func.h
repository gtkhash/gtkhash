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

#ifndef GTKHASH_HASH_FUNC_H
#define GTKHASH_HASH_FUNC_H

#include <stdbool.h>

#define HASH_FUNCS_N (HASH_FUNC_ADLER32 + 1)
#define HASH_FUNC_IS_VALID(X) (((X) >= 0) && ((X) < HASH_FUNCS_N))
#define HASH_FUNC_IS_DEFAULT(X) \
	((X) == HASH_FUNC_MD5 || (X) == HASH_FUNC_SHA1 || (X) == HASH_FUNC_SHA256)

// All supported hash functions
enum hash_func_e {
	HASH_FUNC_INVALID = -1,
	HASH_FUNC_MD2,
	HASH_FUNC_MD4,
	HASH_FUNC_MD5,
	HASH_FUNC_SHA1,
	HASH_FUNC_SHA224,
	HASH_FUNC_SHA256,
	HASH_FUNC_SHA384,
	HASH_FUNC_SHA512,
	HASH_FUNC_RIPEMD128,
	HASH_FUNC_RIPEMD160,
	HASH_FUNC_RIPEMD256,
	HASH_FUNC_RIPEMD320,
	HASH_FUNC_TIGER128,
	HASH_FUNC_TIGER160,
	HASH_FUNC_TIGER192,
	HASH_FUNC_WHIRLPOOL,
	HASH_FUNC_HAVAL128_3,
	HASH_FUNC_HAVAL160_3,
	HASH_FUNC_HAVAL192_3,
	HASH_FUNC_HAVAL224_3,
	HASH_FUNC_HAVAL256_3,
	HASH_FUNC_GOST,
	HASH_FUNC_SNEFRU128,
	HASH_FUNC_SNEFRU256,
	HASH_FUNC_CRC32,
	HASH_FUNC_CRC32B,
	HASH_FUNC_ADLER32,
};

struct hash_func_s {
	enum hash_func_e id;
	bool supported, enabled;
	const char *name;
	struct {
		char *digest;
		void *lib_data;
	} priv;
};

enum hash_func_e gtkhash_hash_func_get_id_from_name(const char *name);
void gtkhash_hash_func_set_digest(struct hash_func_s *func, char *digest);
const char *gtkhash_hash_func_get_digest(struct hash_func_s *func);
void gtkhash_hash_func_init_all(struct hash_func_s *funcs);
void gtkhash_hash_func_deinit_all(struct hash_func_s *funcs);

#endif
