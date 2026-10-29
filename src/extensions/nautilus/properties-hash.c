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

#include <glib.h>

#include "properties.h"
#include "properties-hash.h"
#include "../../hash/digest-format.h"
#include "../../hash/hash-file.h"
#include "../../hash/hash-func.h"
#include "../../util/util.h"

void gtkhash_hash_file_report_cb(void *data, const goffset file_size, const goffset total_read,
	GTimer *timer)
{
	struct model_s *model = data;

	if (file_size <= 0 || total_read <= 0)
		return;

	const double elapsed = g_timer_elapsed(timer, NULL);
	if (elapsed <= 0)
		return;

	char *text = gtkhash_format_progress(file_size, total_read, elapsed);
	gtkhash_properties_model_set_progress(model, text);
	g_free(text);
}

void gtkhash_hash_file_digest_cb(const enum hash_func_e id, const char *digest, void *data)
{
	struct model_s *model = data;

	gtkhash_properties_model_add_digest(model, model->funcs[id].name, digest);
}

void gtkhash_hash_file_finish_cb(void *data)
{
	struct model_s *model = data;

	model->state = MODEL_STATE_DONE; // If not already set by _add_digest()
	g_object_unref(model);
}

void gtkhash_hash_file_stop_cb(void *data)
{
	gtkhash_hash_file_finish_cb(data);
}

void gtkhash_properties_hash_start(struct model_s *model)
{
	gtkhash_hash_file(model->hfile, model->uri, DIGEST_FORMAT_HEX_LOWER, NULL, 0, model);
}

void gtkhash_properties_hash_stop(struct model_s *model)
{
	gtkhash_hash_file_cancel(model->hfile);
}

void gtkhash_properties_hash_init(struct model_s *model)
{
	gtkhash_hash_func_init_all(model->funcs);

	model->hfile = gtkhash_hash_file_new(model->funcs);
}

void gtkhash_properties_hash_deinit(struct model_s *model)
{
	gtkhash_hash_file_free(model->hfile);
	model->hfile = NULL;

	gtkhash_hash_func_deinit_all(model->funcs);
}
