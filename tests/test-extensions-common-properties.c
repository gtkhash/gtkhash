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
#include <stdlib.h>
#include <gtk/gtk.h>

#include "hash/hash-func.h"

#include "extensions/common/properties.c"
#include "extensions/common/properties-hash.c"
#include "extensions/common/properties-list.c"
#include "extensions/common/properties-prefs.c"

static void delay(void)
{
	for (int i = 0; i < 10; i++) {
		while (gtk_events_pending())
			gtk_main_iteration();
		g_usleep(g_test_slow() ? (G_USEC_PER_SEC / 250) : 1);
	}
}

struct fixture_s {
	GtkWidget *window;
	struct page_s *page;
};

static void fx_setup(struct fixture_s *fx, G_GNUC_UNUSED const void *data)
{
	const char *filename = g_test_get_filename(G_TEST_BUILT, "1.bytes", NULL);
	char *uri = g_filename_to_uri(filename, NULL, NULL);
	g_assert_nonnull(uri);

	fx->page = gtkhash_properties_new_page(uri);
	g_assert_nonnull(fx->page);

	fx->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_ref_sink(fx->window);
	gtk_widget_set_sensitive(fx->window, false);
	gtk_container_add(GTK_CONTAINER(fx->window), fx->page->box);
	gtk_widget_show_all(fx->window);

	g_assert_true(fx->page->busy);

	while (fx->page->busy)
		gtk_main_iteration_do(true);
}

static void fx_teardown(struct fixture_s *fx, G_GNUC_UNUSED const void *data)
{
	g_assert_false(fx->page->busy);

	gtk_widget_destroy(GTK_WIDGET(fx->window));
	g_clear_object(&fx->window);
	delay();
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
	g_assert_true(fx->page->funcs[id].enabled);

	g_assert_null(gtk_entry_get_icon_name(fx->page->entry_check, GTK_ENTRY_ICON_SECONDARY));

	gtk_entry_set_text(fx->page->entry_check, t->digest);
	delay();

	const char *icon = gtk_entry_get_icon_name(fx->page->entry_check, GTK_ENTRY_ICON_SECONDARY);
	g_assert_cmpstr(icon, ==, "object-select-symbolic");
}

static void add_tests(void)
{
	for (size_t i = 0; i < G_N_ELEMENTS(test_data); i++) {
		char *path = g_strdup_printf("/extensions/common/check-digest/%s", test_data[i].name);
		g_test_add(path, struct fixture_s, &test_data[i], fx_setup, test_check_digest, fx_teardown);
		g_free(path);
	}
}

int main(int argc, char **argv)
{
	gtk_test_init(&argc, &argv);

	// Workaround GTK bug - leak:gtk_css_value_alloc
	g_object_set(gtk_settings_get_default(), "gtk-enable-animations", false, NULL);

	add_tests();

	return g_test_run();
}
