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
#include <gtk/gtk.h>

#include "hash.h"
#include "main.h"
#include "gui.h"
#include "list.h"
#include "hash/hash-func.h"
#include "hash/hash-string.h"
#include "hash/hash-file.h"
#include "util/util.h"

struct hash_s hash;

static struct {
	struct hash_file_s *hfile;
	GPtrArray *iters;
} hash_priv = {
	.hfile = NULL,
	.iters = NULL,
};

static GtkTreeIter *hash_queue_peek(void)
{
	g_assert(hash_priv.iters);
	g_assert(hash_priv.iters->len);

	return g_ptr_array_index(hash_priv.iters, 0);
}

static void hash_queue_start(void)
{
	GtkTreeIter *iter = hash_queue_peek();

	gtk_progress_bar_set_fraction(gui.progressbar, 0.0);
	gtk_progress_bar_set_text(gui.progressbar, " ");

	char *uri = list_get_uri(iter);
	hash_file_start(uri);
	g_free(uri);
}

static bool hash_queue_next(void)
{
	g_ptr_array_remove_index(hash_priv.iters, 0);

	if (hash_priv.iters->len) {
		hash_queue_start();
		return true;
	}

	g_clear_pointer(&hash_priv.iters, g_ptr_array_unref);
	return false;
}

void gtkhash_hash_string_finish_cb(const char * const *digests)
{
	for (int i = 0; i < HASH_FUNCS_N; i++)
		if (digests[i])
			gtk_entry_set_text(gui.hash_widgets[i].entry_text, digests[i]);

	gui_check_digests();
}

void gtkhash_hash_file_report_cb(G_GNUC_UNUSED void *data, const goffset file_size,
	const goffset total_read, GTimer *timer)
{
	gtk_progress_bar_set_fraction(gui.progressbar, (double)total_read / (double)file_size);

	const double elapsed = g_timer_elapsed(timer, NULL);

	char *text = gtkhash_format_progress(file_size, total_read, elapsed);
	if (text) {
		gtk_progress_bar_set_text(gui.progressbar, text);
		g_free(text);
	}
}

void gtkhash_hash_file_report_unknown_cb(G_GNUC_UNUSED void *data, const goffset total_read,
	GTimer *timer)
{
	gtk_progress_bar_pulse(gui.progressbar);

	const double elapsed = g_timer_elapsed(timer, NULL);

	char *text = gtkhash_format_progress_unknown(total_read, elapsed);
	if (text) {
		gtk_progress_bar_set_text(gui.progressbar, text);
		g_free(text);
	}
}

void gtkhash_hash_file_finish_cb(G_GNUC_UNUSED void *data, const char * const *digests)
{
	switch (gui.view) {
		case GUI_VIEW_FILE:
			for (int i = 0; i < HASH_FUNCS_N; i++)
				if (digests[i])
					gtk_entry_set_text(gui.hash_widgets[i].entry_file, digests[i]);

			// Reset enabled state following single-function hash
			gui_update_hash_funcs();
			break;
		case GUI_VIEW_FILE_LIST:
			list_set_digests(hash_queue_peek(), digests);

			if (hash_queue_next())
				return;
			break;
		default:
			g_assert_not_reached();
	}

	gui_check_digests();
	gui_set_state(GUI_STATE_IDLE);
}

void gtkhash_hash_file_stop_cb(G_GNUC_UNUSED void *data)
{
	g_clear_pointer(&hash_priv.iters, g_ptr_array_unref);

	if (gui.view != GUI_VIEW_FILE_LIST) {
		// Reset enabled state following single-function hash
		gui_update_hash_funcs();
	}

	gui_set_state(GUI_STATE_IDLE);
}

void hash_file_start(const char *uri)
{
	g_assert(gtkhash_hash_func_count_enabled(hash.funcs));

	const enum digest_format_e format = gui_get_digest_format();

	size_t key_size = 0;
	const uint8_t *hmac_key = gui_get_hmac_key(&key_size);

	gtkhash_hash_file(hash_priv.hfile, uri, format, hmac_key, key_size, NULL);
}

void hash_file_list_start(void)
{
	g_assert(gtkhash_hash_func_count_enabled(hash.funcs));
	g_assert(list_get_rows() > 0);
	g_assert(!hash_priv.iters);

	hash_priv.iters = list_get_iters();
	hash_queue_start();
}

void hash_file_stop(void)
{
	gtkhash_hash_file_cancel(hash_priv.hfile);
}

void hash_string(void)
{
	if (!gtkhash_hash_func_count_enabled(hash.funcs))
		return;

	const char *str = gtk_entry_get_text(gui.entry_text);
	const enum digest_format_e format = gui_get_digest_format();

	size_t key_size = 0;
	const uint8_t *hmac_key = gui_get_hmac_key(&key_size);

	gtkhash_hash_string(hash.funcs, str, format, hmac_key, key_size);
}

void hash_funcs_enable_strv(const char **funcs)
{
	for (int i = 0; funcs[i]; i++) {
		const bool enable = (*funcs[i] == '+');
		const bool disable = (*funcs[i] == '-');
		const char *name = (enable || disable) ? &funcs[i][1] : funcs[i];
		const enum hash_func_e id = gtkhash_hash_func_get_id_from_name(name);
		if (disable && HASH_FUNC_IS_VALID(id) && hash.funcs[id].supported) {
			hash.funcs[id].enabled = false;
			gui_disable_hash_func(id);
		} else if (HASH_FUNC_IS_VALID(id) && hash.funcs[id].supported) {
			hash.funcs[id].enabled = true;
			gui_enable_hash_func(id);
		} else
			g_message(_("Unknown Hash Function name \"%s\""), name);
	}
}

void hash_init(void)
{
	gtkhash_hash_func_init_all(hash.funcs);

	hash_priv.hfile = gtkhash_hash_file_new(hash.funcs);
}

void hash_deinit(void)
{
	gtkhash_hash_file_free(hash_priv.hfile);
	hash_priv.hfile = NULL;

	gtkhash_hash_func_deinit_all(hash.funcs);
}
