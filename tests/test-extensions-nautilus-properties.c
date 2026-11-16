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

#undef G_DISABLE_ASSERT
#undef G_DISABLE_CAST_CHECKS

#include <stdbool.h>
#include <glib-object.h>
#include <nautilus-extension.h>

#include "hash/hash-func.h"

#include "extensions/common/module.c"
#include "extensions/nautilus/module.c"
#include "extensions/nautilus/properties.c"
#include "extensions/nautilus/properties-hash.c"
#include "extensions/nautilus/properties-prefs.c"

#define TEST_TYPE_MODULE (test_module_get_type())
G_DECLARE_FINAL_TYPE(TestModule, test_module, TEST, MODULE, GTypeModule)

struct _TestModule {
	GTypeModule parent_instance;
};

G_DEFINE_TYPE(TestModule, test_module, G_TYPE_TYPE_MODULE)

static gboolean test_module_load(G_GNUC_UNUSED GTypeModule *module)
{
	return true;
}

static void test_module_init(G_GNUC_UNUSED TestModule *module)
{
}

static void test_module_class_init(TestModuleClass *klass)
{
	G_TYPE_MODULE_CLASS(klass)->load = test_module_load;
}

struct fixture_s {
	struct model_s *model;
};

static void fx_setup(struct fixture_s *fx, G_GNUC_UNUSED const void *data)
{
	const char *filename = g_test_get_filename(G_TEST_BUILT, "1.bytes", NULL);
	char *uri = g_filename_to_uri(filename, NULL, NULL);
	g_assert_nonnull(uri);

	fx->model = gtkhash_properties_model_new(uri);
	g_assert_nonnull(fx->model);

	g_assert_true(fx->model->state == MODEL_STATE_NEW);

	// Triggers hash start
	g_list_model_get_n_items(G_LIST_MODEL(fx->model));

	g_assert_true(fx->model->state == MODEL_STATE_BUSY);

	while (fx->model->state == MODEL_STATE_BUSY)
		g_main_context_iteration(NULL, true);
}

static void fx_teardown(struct fixture_s *fx, G_GNUC_UNUSED const void *data)
{
	g_assert_true(fx->model->state == MODEL_STATE_DONE);

	g_clear_object(&fx->model);
}

static const struct test_data_s {
	const char *name;
	const char *digest;
} test_data[] = {
	{ "MD5",    "93b885adfe0da089cdf634904fd59f71" },
	{ "SHA1",   "5ba93c9db0cff93f52b521d7420e43f6eda2784f" },
	{ "SHA256", "6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d" },
};

static void test_check_digest(struct fixture_s *fx, const void *data)
{
	const struct test_data_s *t = data;

	const enum hash_func_e id = gtkhash_hash_func_get_id_from_name(t->name);
	g_assert_true(HASH_FUNC_IS_VALID(id));
	g_assert_true(fx->model->funcs[id].enabled);

	NautilusPropertiesItem *item = NULL;
	const unsigned int n = g_list_model_get_n_items(G_LIST_MODEL(fx->model));

	for (unsigned int i = 0; !item && i < n; i++) {
		item = g_list_model_get_item(G_LIST_MODEL(fx->model), i);
		if (g_strcmp0(nautilus_properties_item_get_name(item), t->name) != 0)
			g_clear_object(&item);
	}

	g_assert_nonnull(item);
	g_assert_cmpstr(nautilus_properties_item_get_value(item), ==, t->digest);

	g_object_unref(item);
}

static void add_tests(void)
{
	for (size_t i = 0; i < G_N_ELEMENTS(test_data); i++) {
		char *path = g_strdup_printf("/extensions/nautilus/check-digest/%s", test_data[i].name);
		g_test_add(path, struct fixture_s, &test_data[i], fx_setup, test_check_digest, fx_teardown);
		g_free(path);
	}
}

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	GTypeModule *module = g_object_new(TEST_TYPE_MODULE, NULL);
	nautilus_module_initialize(module);

	add_tests();

	return g_test_run();
}
