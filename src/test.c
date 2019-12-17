/*
 *   Copyright (C) 2007-2019 Tristan Heaven <tristan@tristanheaven.net>
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

#undef G_DISABLE_ASSERT
#undef G_DISABLE_CAST_CHECKS

#include <stdlib.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "check.h"
#include "gui.h"
#include "hash.h"
#include "list.h"
#include "opts.h"
#include "resources.h"
#include "hash/hash-func.h"

#ifndef G_SOURCE_FUNC
#define G_SOURCE_FUNC(f) ((GSourceFunc) (void (*)(void)) (f))
#endif

static void delay(void)
{
	if (g_test_slow())
		g_usleep(G_USEC_PER_SEC / 10);

	while (gtk_events_pending())
		gtk_main_iteration();
}

static void select_func(const enum hash_func_e id, const bool active)
{
	if (hash.funcs[id].enabled == active)
		return;

	gtk_toggle_button_set_active(gui.hash_widgets[id].button, active);
	delay();

	g_assert_true(hash.funcs[id].enabled == active);
}

static void test_hash_func_digest(const enum hash_func_e id, const char *text,
	const char *hmac, const char *digest)
{
	gtk_entry_set_text(gui.entry_check_text, digest);
	gtk_entry_set_text(gui.entry_text, text);

	if (hmac) {
		gtk_toggle_button_set_active(gui.togglebutton_hmac_text, true);
		gtk_entry_set_visibility(gui.entry_hmac_text, true);
		gtk_entry_set_text(gui.entry_hmac_text, hmac);
	} else {
		gtk_toggle_button_set_active(gui.togglebutton_hmac_text, false);
		gtk_entry_set_text(gui.entry_hmac_text, "");
	}

	delay();

	const char *output = gtk_entry_get_text(gui.hash_widgets[id].entry_text);
	g_assert_cmpstr(output, ==, digest);
}

static void test_hash_func(const struct hash_func_s *func)
{
	if (!func->supported) {
		g_test_skip("not supported");
		return;
	}

	bool tested = false;

#define t(FUNC, TEXT, DIGEST) \
	if (func->id == G_PASTE(HASH_FUNC_, FUNC)) { \
		select_func(func->id, true); \
		test_hash_func_digest(func->id, TEXT, NULL, DIGEST); \
		tested = true; \
	}

	t(ADLER32,    "", "00000001");
	t(BLAKE2B,    "", "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce");
	t(BLAKE2S,    "", "69217a3079908094e11121d042354a7c1f55b6482ca1a51e1b250dfd1ed0eef9");
	t(BLAKE2BP,   "", "b5ef811a8038f70b628fa8b294daae7492b1ebe343a80eaabbf1f6ae664dd67b9d90b0120791eab81dc96985f28849f6a305186a85501b405114bfa678df9380");
	t(BLAKE2SP,   "", "dd0e891776933f43c7d032b08a917e25741f8aa9a12c12e1cac8801500f2ca4f");
	t(CRC32,      "", "00000000");
	t(GOST,       "", "ce85b99cc46752fffee35cab9a7b0278abb4c2d2055cff685af4912c49490f8d");
	t(HAVAL128_3, "", "c68f39913f901f3ddf44c707357a7d70");
	t(HAVAL160_3, "", "d353c3ae22a25401d257643836d7231a9a95f953");
	t(HAVAL192_3, "", "e9c48d7903eaf2a91c5b350151efcb175c0fc82de2289a4e");
	t(HAVAL224_3, "", "c5aae9d47bffcaaf84a8c6e7ccacd60a0dd1932be7b1a192b9214b6d");
	t(HAVAL256_3, "", "4f6938531f0bc8991f62da7bbd6f7de3fad44562b8c6f4ebf146d5b4e46f7c17");
	t(MD2,        "", "8350e5a3e24c153df2275c9f80692773");
	t(MD4,        "", "31d6cfe0d16ae931b73c59d7e0c089c0");
	t(MD5,        "", "d41d8cd98f00b204e9800998ecf8427e");
	t(MD6_224,    "", "d2091aa2ad17f38c51ade2697f24cafc3894c617c77ffe10fdc7abcb");
	t(MD6_256,    "", "bca38b24a804aa37d821d31af00f5598230122c5bbfc4c4ad5ed40e4258f04ca");
	t(MD6_384,    "", "b0bafffceebe856c1eff7e1ba2f539693f828b532ebf60ae9c16cbc3499020401b942ac25b310b2227b2954ccacc2f1f");
	t(MD6_512,    "", "6b7f33821a2c060ecdd81aefddea2fd3c4720270e18654f4cb08ece49ccb469f8beeee7c831206bd577f9f2630d9177979203a9489e47e04df4e6deaa0f8e0c0");
	t(RIPEMD128,  "", "cdf26213a150dc3ecb610f18f6b38b46");
	t(RIPEMD160,  "", "9c1185a5c5e9fc54612808977ee8f548b2258d31");
	t(RIPEMD256,  "", "02ba4c4e5f8ecd1877fc52d64d30e37a2d9774fb1e5d026380ae0168e3c5522d");
	t(RIPEMD320,  "", "22d65d5661536cdc75c1fdf5c6de7b41b9f27325ebc61e8557177d705a0ec880151c3a32a00899b8");
	t(SHA1,       "", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	t(SHA224,     "", "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f");
	t(SHA256,     "", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	t(SHA384,     "", "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
	t(SHA512,     "", "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
	t(SHA3_224,   "", "6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7");
	t(SHA3_256,   "", "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a");
	t(SHA3_384,   "", "0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004");
	t(SHA3_512,   "", "a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26");
	t(SM3,        "", "1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b");
	t(SNEFRU128,  "", "8617f366566a011837f4fb4ba5bedea2");
	t(SNEFRU256,  "", "8617f366566a011837f4fb4ba5bedea2b892f3ed8b894023d16ae344b2be5881");
	t(TIGER192,   "", "24f0130c63ac933216166e76b1bb925ff373de2d49584e7a");
	t(WHIRLPOOL,  "", "19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3");

	t(CRC32,     "The quick brown fox jumps over the lazy dog", "414fa339");
	t(BLAKE2B,   "The quick brown fox jumps over the lazy dog", "a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e3f14cb5413f8ed77be73045b13914cdcd6a918");
	t(GOST,      "The quick brown fox jumps over the lazy dog", "77b7fa410c9ac58a25f49bca7d0468c9296529315eaca76bd1a10f376d1f4294");
	t(MD2,       "The quick brown fox jumps over the lazy dog", "03d85a0d629d2c442e987525319fc471");
	t(MD4,       "The quick brown fox jumps over the lazy dog", "1bee69a46ba811185c194762abaeae90");
	t(MD5,       "The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6");
	t(RIPEMD160, "The quick brown fox jumps over the lazy dog", "37f332f68db77bd9d7edd4969571ad671cf9dd3b");
	t(SHA1,      "The quick brown fox jumps over the lazy dog", "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");
	t(SHA224,    "The quick brown fox jumps over the lazy dog", "730e109bd7a8a32b1cb9d9a09aa2325d2430587ddbc0c38bad911525");
	t(SHA256,    "The quick brown fox jumps over the lazy dog", "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
	t(SHA384,    "The quick brown fox jumps over the lazy dog", "ca737f1014a48f4c0b6dd43cb177b0afd9e5169367544c494011e3317dbf9a509cb1e5dc1e85a941bbee3d7f2afbc9b1");
	t(SHA512,    "The quick brown fox jumps over the lazy dog", "07e547d9586f6a73f73fbac0435ed76951218fb7d0c8d788a309d785436bbb642e93a252a954f23912547d1e8a3b5ed6e1bfd7097821233fa0538f3db854fee6");
	t(SHA3_224,  "The quick brown fox jumps over the lazy dog", "d15dadceaa4d5d7bb3b48f446421d542e08ad8887305e28d58335795");
	t(SHA3_256,  "The quick brown fox jumps over the lazy dog", "69070dda01975c8c120c3aada1b282394e7f032fa9cf32f4cb2259a0897dfc04");
	t(SHA3_384,  "The quick brown fox jumps over the lazy dog", "7063465e08a93bce31cd89d2e3ca8f602498696e253592ed26f07bf7e703cf328581e1471a7ba7ab119b1a9ebdf8be41");
	t(SHA3_512,  "The quick brown fox jumps over the lazy dog", "01dedd5de4ef14642445ba5f5b97c15e47b9ad931326e4b0727cd94cefc44fff23f07bf543139939b49128caf436dc1bdee54fcb24023a08d9403f9b4bf0d450");
	t(SNEFRU128, "The quick brown fox jumps over the lazy dog", "59d9539d0dd96d635b5bdbd1395bb86c");
	t(SNEFRU256, "The quick brown fox jumps over the lazy dog", "674caa75f9d8fd2089856b95e93a4fb42fa6c8702f8980e11d97a142d76cb358");
	t(WHIRLPOOL, "The quick brown fox jumps over the lazy dog", "b97de512e91e3828b40d2b0fdce9ceb3c4a71f9bea8d88e75c4fa854df36725fd2b52eb6544edcacd6f8beddfea403cb55ae31f03ad62a5ef54e42ee82c3fb35");

	t(RIPEMD128, "message digest", "9e327b3d6e523062afc1132d7df9d1b8");
	t(RIPEMD160, "message digest", "5d0689ef49d2fae572b881b123a85ffa21595f36");
	t(RIPEMD256, "message digest", "87e971759a1ce47a514d5c914c392c9018c7c46bc14465554afcdf54a5070c0e");
	t(RIPEMD320, "message digest", "3a8e28502ed45d422f68844f9dd316e7b98533fa3f2a91d29f84d425c88d6b4eff727df66a7c0197");
	t(WHIRLPOOL, "message digest", "378c84a4126e2dc6e56dcc7458377aac838d00032230f53ce1f5700c0ffb4d3b8421557659ef55c106b4b52ac5a4aaa692ed920052838f3362e86dbd37a8903e");

	t(ADLER32,  "abc", "024d0127");
	t(CRC32,    "abc", "352441c2");
	t(SM3,      "abc", "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0");
	t(TIGER192, "abc", "f258c1e88414ab2a527ab541ffc5b8bf935f7b951c132951");

	t(SM3, "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd", "debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732");

#undef t

	if (!tested)
		g_test_incomplete("not implemented");

	select_func(func->id, false);
}

static void test_hash_func_hmac(const struct hash_func_s *func)
{
	bool tested = false;

#define t(FUNC, TEXT, HMAC, DIGEST) \
	if (func->id == G_PASTE(HASH_FUNC_, FUNC)) { \
		select_func(func->id, true); \
		test_hash_func_digest(func->id, TEXT, HMAC, DIGEST); \
		tested = true; \
	}

	// RFC 2202
	t(MD5,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"9294727a3638bb1c13f48ef8158bfc9d");
	t(MD5,
		"what do ya want for nothing?",
		"Jefe",
		"750c783e6ab0b503eaa86e310a5db738");
	t(SHA1,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"b617318655057264e28bc0b6fb378c8ef146be00");
	t(SHA1,
		"what do ya want for nothing?",
		"Jefe",
		"effcdf6ae5eb2fa2d27416d5f184df9c259a7c79");

	// RFC 2286
	t(RIPEMD128,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"fbf61f9492aa4bbf81c172e84e0734db");
	t(RIPEMD128,
		"what do ya want for nothing?",
		"Jefe",
		"875f828862b6b334b427c55f9f7ff09b");
	t(RIPEMD160,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"24cb4bd67d20fc1a5d2ed7732dcc39377f0a5668");
	t(RIPEMD160,
		"what do ya want for nothing?",
		"Jefe",
		"dda6c0213a485a9e24f4742064a7f033b43c4069");

	// RFC 4231
	t(SHA224,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"896fb1128abbdf196832107cd49df33f47b4b1169912ba4f53684b22");
	t(SHA224,
		"what do ya want for nothing?",
		"Jefe",
		"a30e01098bc6dbbf45690f3a7e9e6d0f8bbea2a39e6148008fd05e44");
	t(SHA256,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
	t(SHA256,
		"what do ya want for nothing?",
		"Jefe",
		"5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
	t(SHA384,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"afd03944d84895626b0825f4ab46907f15f9dadbe4101ec682aa034c7cebc59cfaea9ea9076ede7f4af152e8b2fa9cb6");
	t(SHA384,
		"what do ya want for nothing?",
		"Jefe",
		"af45d2e376484031617f78d2b58a6b1b9c7ef464f5a01b47e42ec3736322445e8e2240ca5e69e2c78b3239ecfab21649");
	t(SHA512,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854");
	t(SHA512,
		"what do ya want for nothing?",
		"Jefe",
		"164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf75c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737");

	// http://wolfgang-ehrhardt.de/hmac-sha3-testvectors.html
	t(SHA3_224,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"3b16546bbc7be2706a031dcafd56373d9884367641d8c59af3c860f7");
	t(SHA3_224,
		"what do ya want for nothing?",
		"Jefe",
		"7fdb8dd88bd2f60d1b798634ad386811c2cfc85bfaf5d52bbace5e66");
	t(SHA3_256,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"ba85192310dffa96e2a3a40e69774351140bb7185e1202cdcc917589f95e16bb");
	t(SHA3_256,
		"what do ya want for nothing?",
		"Jefe",
		"c7d4072e788877ae3596bbb0da73b887c9171f93095b294ae857fbe2645e1ba5");
	t(SHA3_384,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"68d2dcf7fd4ddd0a2240c8a437305f61fb7334cfb5d0226e1bc27dc10a2e723a20d370b47743130e26ac7e3d532886bd");
	t(SHA3_384,
		"what do ya want for nothing?",
		"Jefe",
		"f1101f8cbf9766fd6764d2ed61903f21ca9b18f57cf3e1a23ca13508a93243ce48c045dc007f26a21b3f5e0e9df4c20a");
	t(SHA3_512,
		"Hi There",
		"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
		"eb3fbd4b2eaab8f5c504bd3a41465aacec15770a7cabac531e482f860b5ec7ba47ccb2c6f2afce8f88d22b6dc61380f23a668fd3888bb80537c0a0b86407689e");
	t(SHA3_512,
		"what do ya want for nothing?",
		"Jefe",
		"5a4bfeab6166427c7a3647b747292b8384537cdb89afb3bf5665e4c5e709350b287baec921fd7ca0ee7a0c31d022a95e1fc92ba9d77df883960275beb4e62024");

#undef t

	if (!tested)
		g_test_incomplete("not implemented");

	select_func(func->id, false);
}

static void test_opt_help(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		g_shell_parse_argv("t --help", &argc, &argv, NULL);

		opts_preinit(&argc, &argv);

		exit(EXIT_FAILURE);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stdout("*--help*");
	g_test_trap_assert_stdout("*--version*");
}

static void test_opt_version(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		g_shell_parse_argv("t --version", &argc, &argv, NULL);

		opts_preinit(&argc, &argv);

		exit(EXIT_FAILURE);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stdout(PACKAGE_STRING "*");
}

static void test_opt_check_text(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		g_shell_parse_argv("t -c fail -t aa --check 0123abcdef", &argc, &argv, NULL);

		opts_preinit(&argc, &argv);
		opts_postinit();
		delay();

		puts(gtk_entry_get_text(gui.entry_check_text));
		exit(EXIT_SUCCESS);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stdout("0123abcdef*");
}

static void test_opt_check_file(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		char *args = g_strdup_printf("t --check-file '%s'",
			g_test_get_filename(G_TEST_BUILT, "test.md5sum", NULL));

		g_shell_parse_argv(args, &argc, &argv, NULL);
		g_free(args);

		opts_preinit(&argc, &argv);
		opts_postinit();
		delay();

		g_assert(hash.funcs[HASH_FUNC_MD5].supported);
		g_assert(hash.funcs[HASH_FUNC_MD5].enabled);
		g_assert(gui.view == GUI_VIEW_FILE_LIST);
		g_assert(list.rows == 9);

		// OK to exit before finish, with warnings
		g_test_expect_message(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "*");
		gdk_threads_add_timeout_seconds(2, G_SOURCE_FUNC(exit), NULL);

		for (;;)
			gtk_main_iteration_do(false);

		exit(EXIT_FAILURE);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stderr("*notfound.bytes*");
}

static void test_opt_function(void)
{
	if (g_test_subprocess()) {
		enum hash_func_e id = HASH_FUNC_INVALID;

		// Select first available function for the test
		for (int i = 0; i < HASH_FUNCS_N; i++) {
			if (hash.funcs[i].supported) {
				id = i;
				break;
			}
		}

		if (id == HASH_FUNC_INVALID)
			exit(EXIT_FAILURE);

		// Disable the function
		gtk_toggle_button_set_active(gui.hash_widgets[id].button, false);
		delay();

		if (hash.funcs[id].enabled)
			exit(EXIT_FAILURE);

		// Enable the function from cmdline
		char *str = g_strdup_printf("%s %s", "t --function XX -f", hash.funcs[id].name);
		gint argc;
		char **argv;
		g_shell_parse_argv(str, &argc, &argv, NULL);

		opts_preinit(&argc, &argv);

		if (hash.funcs[id].enabled)
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stderr("*Unknown*XX*");
}

static void test_opt_file(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		char *args = g_strdup_printf("t -- '%s'",
			g_test_get_filename(G_TEST_BUILT, "10M.bytes", NULL));

		g_shell_parse_argv(args, &argc, &argv, NULL);
		g_free(args);

		select_func(HASH_FUNC_MD5, true);

		opts_preinit(&argc, &argv);
		opts_postinit();
		delay();

		g_assert(gui.view == GUI_VIEW_FILE);

		while (!*gtk_entry_get_text(gui.hash_widgets[HASH_FUNC_MD5].entry_file))
			gtk_main_iteration_do(false);
		puts(gtk_entry_get_text(gui.hash_widgets[HASH_FUNC_MD5].entry_file));

		exit(EXIT_SUCCESS);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stdout("*f1c9645dbc14efddc7d8a322685f26eb*");
}

static void test_opt_file_list(void)
{
	if (g_test_subprocess()) {
		gint argc;
		char **argv;
		char *args = g_strdup_printf("t -- '%s' '%s'",
			g_test_get_filename(G_TEST_BUILT, "0.bytes", NULL),
			g_test_get_filename(G_TEST_BUILT, "0.bytes", NULL));

		g_shell_parse_argv(args, &argc, &argv, NULL);
		g_free(args);

		select_func(HASH_FUNC_MD5, true);

		opts_preinit(&argc, &argv);
		opts_postinit();
		delay();

		g_assert(gui.view == GUI_VIEW_FILE_LIST);
		g_assert(list.rows == 2);

		char *digest;
		while (!(digest = list_get_digest(1, HASH_FUNC_MD5)) || !*digest)
			gtk_main_iteration_do(false);
		puts(digest);
		g_free(digest);

		gtk_tree_selection_select_all(gui.treeselection);
		delay();
		list_remove_selection();
		delay();
		g_assert(list.rows == 0);

		exit(EXIT_SUCCESS);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_passed();
	g_test_trap_assert_stdout("*d41d8cd98f00b204e9800998ecf8427e*");
}

static void test_init(void)
{
	gui_set_view(GUI_VIEW_TEXT);
	delay();

	// Test plain hash
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		char *str = g_strdup_printf("/hash/func/%s", hash.funcs[i].name);
		g_test_add_data_func(str, &hash.funcs[i],
			(GTestDataFunc)test_hash_func);
		g_free(str);
	}

	// Test HMAC
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].supported || !hash.funcs[i].hmac_supported)
			continue;

		char *str = g_strdup_printf("/hash/func/HMAC-%s", hash.funcs[i].name);
		g_test_add_data_func(str, &hash.funcs[i],
			(GTestDataFunc)test_hash_func_hmac);
		g_free(str);
	}

	// Test cmdline options
	g_test_add_func("/opt/help", test_opt_help);
	g_test_add_func("/opt/version", test_opt_version);
	g_test_add_func("/opt/check/text", test_opt_check_text);
	g_test_add_func("/opt/check/file", test_opt_check_file);
	g_test_add_func("/opt/function", test_opt_function);
	g_test_add_func("/opt/file", test_opt_file);
	g_test_add_func("/opt/file-list", test_opt_file_list);

	g_test_set_nonfatal_assertions();
}

int main(int argc, char **argv)
{
	gtk_test_init(&argc, &argv);

	hash_init();
	atexit(hash_deinit);

	resources_register_resource();
	gui_init();
	atexit(gui_deinit);
	resources_unregister_resource();

	list_init();

	check_init();
	atexit(check_deinit);

	// Ignore user input during testing
	gtk_widget_set_sensitive(GTK_WIDGET(gui.window), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gui.dialog), false);

	if (g_test_slow()) {
		gtk_widget_show_now(GTK_WIDGET(gui.window));
		gtk_widget_show_now(GTK_WIDGET(gui.dialog));
	}

	callbacks_init();
	test_init();

	return g_test_run();
}
