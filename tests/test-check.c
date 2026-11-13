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

#include <string.h>
#include <glib.h>

#include "test.h"
#include "check.h"
#include "hash.h"

#define DIGEST_HEX_7   "0aA0aA0"
#define DIGEST_HEX_8   "0aA0aA0a"
#define DIGEST_HEX_9   "0aA0aA0aA"
#define DIGEST_HEX_16  DIGEST_HEX_8  DIGEST_HEX_8
#define DIGEST_HEX_32  DIGEST_HEX_16 DIGEST_HEX_16
#define DIGEST_HEX_40  DIGEST_HEX_32 DIGEST_HEX_8
#define DIGEST_HEX_56  DIGEST_HEX_40 DIGEST_HEX_16
#define DIGEST_HEX_64  DIGEST_HEX_32 DIGEST_HEX_32
#define DIGEST_HEX_96  DIGEST_HEX_64 DIGEST_HEX_32
#define DIGEST_HEX_128 DIGEST_HEX_64 DIGEST_HEX_64

#define FILENAME     " ; a ) \\ b / ✅ \t"
#define FILENAME_ESC " ; a ) \\\\ b / ✅ \t"

static const struct test_data_s {
	const char *name;           // test name
	const char *line;           // test line
	enum check_format_e format; // expected format, or CHECK_FORMAT_INVALID
	enum hash_func_e id;        // expected FUNCTION, or HASH_FUNC_INVALID
	const char *filename;       // expected FILENAME
	const char *digest;         // expected DIGEST
} test_data[] = {
	//
	// CHECK_FORMAT_BSD: GNU coreutils `cksum --tag` output
	//
	{ "cksum-tag-blake2b",     "BLAKE2b (" FILENAME ") = " DIGEST_HEX_128,    CHECK_FORMAT_BSD, HASH_FUNC_BLAKE2B,  FILENAME, DIGEST_HEX_128 },
	// Not implemented
	{ "cksum-tag-blake2b-256", "BLAKE2b-256 (" FILENAME ") = " DIGEST_HEX_64, CHECK_FORMAT_BSD, HASH_FUNC_INVALID,  FILENAME, DIGEST_HEX_64 },
	{ "cksum-tag-md5",         "MD5 (" FILENAME ") = " DIGEST_HEX_32,         CHECK_FORMAT_BSD, HASH_FUNC_MD5,      FILENAME, DIGEST_HEX_32 },
	{ "cksum-tag-sha1",        "SHA1 (" FILENAME ") = " DIGEST_HEX_40,        CHECK_FORMAT_BSD, HASH_FUNC_SHA1,     FILENAME, DIGEST_HEX_40 },
	{ "cksum-tag-sha224",      "SHA224 (" FILENAME ") = " DIGEST_HEX_56,      CHECK_FORMAT_BSD, HASH_FUNC_SHA224,   FILENAME, DIGEST_HEX_56 },
	{ "cksum-tag-sha256",      "SHA256 (" FILENAME ") = " DIGEST_HEX_64,      CHECK_FORMAT_BSD, HASH_FUNC_SHA256,   FILENAME, DIGEST_HEX_64 },
	{ "cksum-tag-sha384",      "SHA384 (" FILENAME ") = " DIGEST_HEX_96,      CHECK_FORMAT_BSD, HASH_FUNC_SHA384,   FILENAME, DIGEST_HEX_96 },
	{ "cksum-tag-sha512",      "SHA512 (" FILENAME ") = " DIGEST_HEX_128,     CHECK_FORMAT_BSD, HASH_FUNC_SHA512,   FILENAME, DIGEST_HEX_128 },
	{ "cksum-tag-sha3-224",    "SHA3-224 (" FILENAME ") = " DIGEST_HEX_56,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_224, FILENAME, DIGEST_HEX_56 },
	{ "cksum-tag-sha3-256",    "SHA3-256 (" FILENAME ") = " DIGEST_HEX_64,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_256, FILENAME, DIGEST_HEX_64 },
	{ "cksum-tag-sha3-384",    "SHA3-384 (" FILENAME ") = " DIGEST_HEX_96,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_384, FILENAME, DIGEST_HEX_96 },
	{ "cksum-tag-sha3-512",    "SHA3-512 (" FILENAME ") = " DIGEST_HEX_128,   CHECK_FORMAT_BSD, HASH_FUNC_SHA3_512, FILENAME, DIGEST_HEX_128 },
	{ "cksum-tag-sm3",         "SM3 (" FILENAME ") = " DIGEST_HEX_64,         CHECK_FORMAT_BSD, HASH_FUNC_SM3,      FILENAME, DIGEST_HEX_64 },

	//
	// CHECK_FORMAT_BSD: `openssl dgst` output
	//
	{ "openssl-blake2b-512", "BLAKE2B-512(" FILENAME ")= " DIGEST_HEX_128, CHECK_FORMAT_BSD, HASH_FUNC_BLAKE2B,   FILENAME, DIGEST_HEX_128 },
	{ "openssl-blake2s-256", "BLAKE2S-256(" FILENAME ")= " DIGEST_HEX_64,  CHECK_FORMAT_BSD, HASH_FUNC_BLAKE2S,   FILENAME, DIGEST_HEX_64 },
	{ "openssl-md4",         "MD4(" FILENAME ")= " DIGEST_HEX_32,          CHECK_FORMAT_BSD, HASH_FUNC_MD4,       FILENAME, DIGEST_HEX_32 },
	{ "openssl-md5",         "MD5(" FILENAME ")= " DIGEST_HEX_32,          CHECK_FORMAT_BSD, HASH_FUNC_MD5,       FILENAME, DIGEST_HEX_32 },
	{ "openssl-mdc2",        "MDC2(" FILENAME ")= " DIGEST_HEX_32,         CHECK_FORMAT_BSD, HASH_FUNC_MDC2,      FILENAME, DIGEST_HEX_32 },
	{ "openssl-ripemd160",   "RIPEMD-160(" FILENAME ")= " DIGEST_HEX_40,   CHECK_FORMAT_BSD, HASH_FUNC_RIPEMD160, FILENAME, DIGEST_HEX_40 },
	{ "openssl-sha1",        "SHA1(" FILENAME ")= " DIGEST_HEX_40,         CHECK_FORMAT_BSD, HASH_FUNC_SHA1,      FILENAME, DIGEST_HEX_40 },
	{ "openssl-sha2-224",    "SHA2-224(" FILENAME ")= " DIGEST_HEX_56,     CHECK_FORMAT_BSD, HASH_FUNC_SHA224,    FILENAME, DIGEST_HEX_56 },
	{ "openssl-sha2-256",    "SHA2-256(" FILENAME ")= " DIGEST_HEX_64,     CHECK_FORMAT_BSD, HASH_FUNC_SHA256,    FILENAME, DIGEST_HEX_64 },
	{ "openssl-sha2-384",    "SHA2-384(" FILENAME ")= " DIGEST_HEX_96,     CHECK_FORMAT_BSD, HASH_FUNC_SHA384,    FILENAME, DIGEST_HEX_96 },
	{ "openssl-sha2-512",    "SHA2-512(" FILENAME ")= " DIGEST_HEX_128,    CHECK_FORMAT_BSD, HASH_FUNC_SHA512,    FILENAME, DIGEST_HEX_128 },
	{ "openssl-sha3-224",    "SHA3-224(" FILENAME ")= " DIGEST_HEX_56,     CHECK_FORMAT_BSD, HASH_FUNC_SHA3_224,  FILENAME, DIGEST_HEX_56 },
	{ "openssl-sha3-256",    "SHA3-256(" FILENAME ")= " DIGEST_HEX_64,     CHECK_FORMAT_BSD, HASH_FUNC_SHA3_256,  FILENAME, DIGEST_HEX_64 },
	{ "openssl-sha3-384",    "SHA3-384(" FILENAME ")= " DIGEST_HEX_96,     CHECK_FORMAT_BSD, HASH_FUNC_SHA3_384,  FILENAME, DIGEST_HEX_96 },
	{ "openssl-sha3-512",    "SHA3-512(" FILENAME ")= " DIGEST_HEX_128,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_512,  FILENAME, DIGEST_HEX_128 },
	{ "openssl-sm3",         "SM3(" FILENAME ")= " DIGEST_HEX_64,          CHECK_FORMAT_BSD, HASH_FUNC_SM3,       FILENAME, DIGEST_HEX_64 },
	{ "openssl-whirlpool",   "WHIRLPOOL(" FILENAME ")= " DIGEST_HEX_128,   CHECK_FORMAT_BSD, HASH_FUNC_WHIRLPOOL, FILENAME, DIGEST_HEX_128 },

	//
	// CHECK_FORMAT_BSD: `rhash --bsd` output
	//
	{ "rhash-bsd-blake2b",    "BLAKE2b (" FILENAME ") = " DIGEST_HEX_128,    CHECK_FORMAT_BSD, HASH_FUNC_BLAKE2B,     FILENAME, DIGEST_HEX_128 },
	{ "rhash-bsd-blake2s",    "BLAKE2s (" FILENAME ") = " DIGEST_HEX_64,     CHECK_FORMAT_BSD, HASH_FUNC_BLAKE2S,     FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-blake3",     "BLAKE3 (" FILENAME ") = " DIGEST_HEX_64,      CHECK_FORMAT_BSD, HASH_FUNC_BLAKE3,      FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-crc32",      "CRC32 (" FILENAME ") = " DIGEST_HEX_8,        CHECK_FORMAT_BSD, HASH_FUNC_CRC32,       FILENAME, DIGEST_HEX_8 },
	{ "rhash-bsd-gost94",     "GOST94 (" FILENAME ") = " DIGEST_HEX_64,      CHECK_FORMAT_BSD, HASH_FUNC_GOST94,      FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-md4",        "MD4   (" FILENAME ") = " DIGEST_HEX_32,       CHECK_FORMAT_BSD, HASH_FUNC_MD4,         FILENAME, DIGEST_HEX_32 },
	{ "rhash-bsd-md5",        "MD5   (" FILENAME ") = " DIGEST_HEX_32,       CHECK_FORMAT_BSD, HASH_FUNC_MD5,         FILENAME, DIGEST_HEX_32 },
	{ "rhash-bsd-rmd160",     "RMD160 (" FILENAME ") = " DIGEST_HEX_40,      CHECK_FORMAT_BSD, HASH_FUNC_RIPEMD160,   FILENAME, DIGEST_HEX_40 },
	{ "rhash-bsd-sha1",       "SHA1  (" FILENAME ") = " DIGEST_HEX_40,       CHECK_FORMAT_BSD, HASH_FUNC_SHA1,        FILENAME, DIGEST_HEX_40 },
	{ "rhash-bsd-sha224",     "SHA224 (" FILENAME ") = " DIGEST_HEX_56,      CHECK_FORMAT_BSD, HASH_FUNC_SHA224,      FILENAME, DIGEST_HEX_56 },
	{ "rhash-bsd-sha256",     "SHA256 (" FILENAME ") = " DIGEST_HEX_64,      CHECK_FORMAT_BSD, HASH_FUNC_SHA256,      FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-sha384",     "SHA384 (" FILENAME ") = " DIGEST_HEX_96,      CHECK_FORMAT_BSD, HASH_FUNC_SHA384,      FILENAME, DIGEST_HEX_96 },
	{ "rhash-bsd-sha512",     "SHA512 (" FILENAME ") = " DIGEST_HEX_128,     CHECK_FORMAT_BSD, HASH_FUNC_SHA512,      FILENAME, DIGEST_HEX_128 },
	{ "rhash-bsd-sha3-224",   "SHA3-224 (" FILENAME ") = " DIGEST_HEX_56,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_224,    FILENAME, DIGEST_HEX_56 },
	{ "rhash-bsd-sha3-256",   "SHA3-256 (" FILENAME ") = " DIGEST_HEX_64,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_256,    FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-sha3-384",   "SHA3-384 (" FILENAME ") = " DIGEST_HEX_96,    CHECK_FORMAT_BSD, HASH_FUNC_SHA3_384,    FILENAME, DIGEST_HEX_96 },
	{ "rhash-bsd-sha3-512",   "SHA3-512 (" FILENAME ") = " DIGEST_HEX_128,   CHECK_FORMAT_BSD, HASH_FUNC_SHA3_512,    FILENAME, DIGEST_HEX_128 },
	{ "rhash-bsd-gost12-256", "GOST12-256 (" FILENAME ") = " DIGEST_HEX_64,  CHECK_FORMAT_BSD, HASH_FUNC_STREEBOG256, FILENAME, DIGEST_HEX_64 },
	{ "rhash-bsd-gost12-512", "GOST12-512 (" FILENAME ") = " DIGEST_HEX_128, CHECK_FORMAT_BSD, HASH_FUNC_STREEBOG512, FILENAME, DIGEST_HEX_128 },
	{ "rhash-bsd-whirlpool",  "WHIRLPOOL (" FILENAME ") = " DIGEST_HEX_128,  CHECK_FORMAT_BSD, HASH_FUNC_WHIRLPOOL,   FILENAME, DIGEST_HEX_128 },

	//
	// CHECK_FORMAT_BSD: Accepted variants
	//
	{ "bsd-lower-func-name",   "md5 (" FILENAME ") = " DIGEST_HEX_32,        CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME,      DIGEST_HEX_32 },
	{ "bsd-leading-space",    " MD5 (" FILENAME ") = " DIGEST_HEX_32,        CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME,      DIGEST_HEX_32 },
	{ "bsd-leading-tab",     "\tMD5 (" FILENAME ") = " DIGEST_HEX_32,        CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME,      DIGEST_HEX_32 },
	{ "bsd-escaped",         "\\MD5 (" FILENAME_ESC ") = " DIGEST_HEX_32,    CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME,      DIGEST_HEX_32 },
	{ "bsd-escaped-lf",      "\\MD5 (" FILENAME_ESC "\\n) = " DIGEST_HEX_32, CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME "\n", DIGEST_HEX_32 },
	{ "bsd-dos-line-ending",   "MD5 (" FILENAME ") = " DIGEST_HEX_32 "\r",   CHECK_FORMAT_BSD, HASH_FUNC_MD5, FILENAME,      DIGEST_HEX_32 },

	//
	// CHECK_FORMAT_GNU: `md5sum` output
	//
	{ "md5sum-text",   DIGEST_HEX_32 "  " FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_32 },
	{ "md5sum-binary", DIGEST_HEX_32 " *" FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_32 },

	//
	// CHECK_FORMAT_GNU: `sha1sum` output
	//
	{ "sha1sum-text",   DIGEST_HEX_40 "  " FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_40 },
	{ "sha1sum-binary", DIGEST_HEX_40 " *" FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_40 },

	//
	// CHECK_FORMAT_GNU: `sha256sum` output
	//
	{ "sha256sum-text",   DIGEST_HEX_64 "  " FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_64 },
	{ "sha256sum-binary", DIGEST_HEX_64 " *" FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_64 },

	//
	// CHECK_FORMAT_GNU: `sha512sum` output
	//
	{ "sha512sum-text",   DIGEST_HEX_128 "  " FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_128 },
	{ "sha512sum-binary", DIGEST_HEX_128 " *" FILENAME, CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_128 },

	//
	// CHECK_FORMAT_GNU: Accepted variants
	//
	{ "gnu-leading-space",    " " DIGEST_HEX_64 "  "  FILENAME,           CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME,      DIGEST_HEX_64 },
	{ "gnu-leading-tab",     "\t" DIGEST_HEX_64 "  "  FILENAME,           CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME,      DIGEST_HEX_64 },
	{ "gnu-escaped",         "\\" DIGEST_HEX_64 "  "  FILENAME_ESC,       CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME,      DIGEST_HEX_64 },
	{ "gnu-escaped-lf",      "\\" DIGEST_HEX_64 "  "  FILENAME_ESC "\\n", CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME "\n", DIGEST_HEX_64 },
	{ "gnu-dos-line-ending",      DIGEST_HEX_64 "  "  FILENAME "\r",      CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME,      DIGEST_HEX_64 },
	{ "gnu-tab-separator",        DIGEST_HEX_64 "\t " FILENAME,           CHECK_FORMAT_GNU, HASH_FUNC_INVALID, FILENAME,      DIGEST_HEX_64 },
	{ "gnu-min-line",             DIGEST_HEX_8  "  "  "_",                CHECK_FORMAT_GNU, HASH_FUNC_INVALID, "_",           DIGEST_HEX_8 },
	{ "gnu-or-sfv",               DIGEST_HEX_32 "  "  DIGEST_HEX_8,       CHECK_FORMAT_GNU, HASH_FUNC_INVALID, DIGEST_HEX_8,  DIGEST_HEX_32 },

	//
	// CHECK_FORMAT_SFV: `cksfv` output
	//
	{ "cksfv", FILENAME " " DIGEST_HEX_8, CHECK_FORMAT_SFV, HASH_FUNC_INVALID, FILENAME, DIGEST_HEX_8 },

	//
	// CHECK_FORMAT_SFV: Accepted variants
	//
	{ "sfv-trailing-space",     FILENAME " "  DIGEST_HEX_8 " ",  CHECK_FORMAT_SFV, HASH_FUNC_INVALID,      FILENAME, DIGEST_HEX_8 },
	{ "sfv-trailing-tab",       FILENAME " "  DIGEST_HEX_8 "\t", CHECK_FORMAT_SFV, HASH_FUNC_INVALID,      FILENAME, DIGEST_HEX_8 },
	{ "sfv-dos-line-ending",    FILENAME " "  DIGEST_HEX_8 "\r", CHECK_FORMAT_SFV, HASH_FUNC_INVALID,      FILENAME, DIGEST_HEX_8 },
	{ "sfv-tab-separator",      FILENAME "\t" DIGEST_HEX_8,      CHECK_FORMAT_SFV, HASH_FUNC_INVALID,      FILENAME, DIGEST_HEX_8 },
	{ "sfv-no-unescape",   "\\" FILENAME " "  DIGEST_HEX_8,      CHECK_FORMAT_SFV, HASH_FUNC_INVALID, "\\" FILENAME, DIGEST_HEX_8 },
	{ "sfv-min-line",           "_"      " "  DIGEST_HEX_8,      CHECK_FORMAT_SFV, HASH_FUNC_INVALID,      "_",      DIGEST_HEX_8 },

	//
	// CHECK_FORMAT_COMMENT
	//
	{ "empty-line",  "",          CHECK_FORMAT_COMMENT, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-comment", "# comment", CHECK_FORMAT_COMMENT, HASH_FUNC_INVALID, NULL, NULL },
	{ "sfv-comment", "; comment", CHECK_FORMAT_COMMENT, HASH_FUNC_INVALID, NULL, NULL },

	//
	// CHECK_FORMAT_INVALID
	//
	{ "plain-text",         "hello world",                               CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "invalid-utf8",       DIGEST_HEX_16 "  \xe9",                      CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "bsd-trailing-chars", "MD5 (" FILENAME ") = " DIGEST_HEX_32 "+++", CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "bsd-short-digest",   "MD5 (" FILENAME ") = " DIGEST_HEX_7,        CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "bsd-stdin-cksum",    "MD5 (-) = " DIGEST_HEX_32,                  CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "bsd-stdin-openssl",  "MD5(stdin)= " DIGEST_HEX_32,                CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "bsd-stdin-rhash",    "MD5   ((stdin)) = " DIGEST_HEX_32,          CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-short-digest",   DIGEST_HEX_7 "  " FILENAME,                  CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-stdin-cksum",    DIGEST_HEX_32 "  -",                         CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-stdin-openssl",  DIGEST_HEX_32 " *stdin",                     CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-stdin-rhash",    DIGEST_HEX_32 "  (stdin)",                   CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "gnu-esc-invalid",    "\\" DIGEST_HEX_64 "  " FILENAME "\\",       CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "sfv-trailing-chars", FILENAME " " DIGEST_HEX_8 "+++",             CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "sfv-short-digest",   FILENAME " " DIGEST_HEX_7,                   CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
	{ "sfv-long-digest",    FILENAME " " DIGEST_HEX_9,                   CHECK_FORMAT_INVALID, HASH_FUNC_INVALID, NULL, NULL },
};

static void test_check_parse_line(const void *data)
{
	const struct test_data_s *t = data;

	enum hash_func_e id = HASH_FUNC_INVALID;
	char *filename = NULL;
	char *digest = NULL;

	const enum check_format_e format = check_line_parse(t->line, strlen(t->line),
		&id, &filename, &digest);

	g_assert_cmpint(format, ==, t->format);

	if (!CHECK_FORMAT_IS_VALID(t->format)) {
		g_assert_cmpint(id, ==, HASH_FUNC_INVALID);
		g_assert_null(filename);
		g_assert_null(digest);
		return;
	}

	g_assert_cmpint(id, ==, t->id);
	g_assert_cmpstr(filename, ==, t->filename);
	g_assert_cmpstr(digest, ==, t->digest);

	if (t->id != HASH_FUNC_INVALID)
		g_assert_cmpint(strlen(digest), ==, hash.funcs[t->id].digest_size * 2);

	g_free(filename);
	g_free(digest);
}

void test_check_add_tests(void)
{
	for (size_t i = 0; i < G_N_ELEMENTS(test_data); i++) {
		char *path = g_strdup_printf("/check/parse/%s", test_data[i].name);
		g_test_add_data_func(path, &test_data[i], test_check_parse_line);
		g_free(path);
	}
}
