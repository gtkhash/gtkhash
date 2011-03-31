/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
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
#include <gtk/gtk.h>

#include "properties.h"
#include "properties-hash.h"
#include "properties-list.h"
#include "../hash/hash-string.h"
#include "../hash/hash-file.h"

void gtkhash_hash_string_finish_cb(G_GNUC_UNUSED const enum hash_func_e id,
	G_GNUC_UNUSED const char *digest)
{
}

void gtkhash_hash_file_report_cb(void *data, goffset file_size,
	goffset total_read, GTimer *timer)
{
	struct page_s *page = data;

	gtk_progress_bar_set_fraction(page->progressbar,
		(double)total_read /
		(double)file_size);

	if (total_read < FILE_BUFFER_SIZE * 3)
		return;

	// Update progres bar text...
	int remaining = g_timer_elapsed(timer, NULL) /
		total_read * (file_size - total_read);
	char *text;
	if (remaining > 60) {
		int minutes = remaining / 60;
		if (minutes == 1)
			text = g_strdup_printf(_("Estimated time remaining: 1 minute"));
		else
			text = g_strdup_printf(_("Estimated time remaining: %d minutes"),
				minutes);
	} else {
		if (remaining == 1)
			text = g_strdup_printf(_("Estimated time remaining: 1 second"));
		else
			text = g_strdup_printf(_("Estimated time remaining: %d seconds"),
				remaining);
	}
	gtk_progress_bar_set_text(page->progressbar, text);
	g_free(text);
}

void gtkhash_hash_file_finish_cb(void *data)
{
	struct page_s *page = data;

	if (!gtkhash_hash_file_get_stop(&page->hash_file))
		gtkhash_properties_list_update_digests(page);

	gtkhash_properties_idle(page);
}

void gtkhash_properties_hash_start(struct page_s *page)
{
	gtkhash_hash_file_set_stop(&page->hash_file, false);
	gtkhash_hash_file_set_state(&page->hash_file, HASH_FILE_STATE_START);
	gtkhash_hash_file_add_source(&page->hash_file);
}

void gtkhash_properties_hash_stop(struct page_s *page)
{
	gtkhash_hash_file_set_stop(&page->hash_file, true);

	while (gtkhash_hash_file_get_state(&page->hash_file) != HASH_FILE_STATE_IDLE)
		gtk_main_iteration_do(false);
}

void gtkhash_properties_hash_init(struct page_s *page, const char *uri)
{
	gtkhash_hash_func_init_all(page->funcs);
	gtkhash_hash_file_init(&page->hash_file, page->funcs, page);
	gtkhash_hash_file_set_uri(&page->hash_file, uri);
}

void gtkhash_properties_hash_deinit(struct page_s *page)
{
	gtkhash_hash_file_deinit(&page->hash_file);
	gtkhash_hash_func_deinit_all(page->funcs);
}
