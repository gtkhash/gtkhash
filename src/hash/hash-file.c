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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <gdk/gdk.h>
#include <glib.h>

#include "hash-file.h"
#include "hash-func.h"
#include "hash-lib.h"

static bool gtkhash_hash_file(struct hash_file_s *data);
static void gtkhash_hash_file_hash_thread(void *func, struct hash_file_s *data);

static void gtkhash_hash_file_set_source(struct hash_file_s *data,
	const unsigned int source)
{
	g_mutex_lock(data->priv.mutex);
	data->priv.source = source;
	g_mutex_unlock(data->priv.mutex);
}

void gtkhash_hash_file_add_source(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	if (G_UNLIKELY(data->priv.source))
		g_error("source was already added");
	data->priv.source = gdk_threads_add_idle(
		(GSourceFunc)gtkhash_hash_file, data);
	g_mutex_unlock(data->priv.mutex);
}

static void gtkhash_hash_file_remove_source(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	if (G_UNLIKELY(!g_source_remove(data->priv.source)))
		g_error("failed to remove source");
	data->priv.source = 0;
	g_mutex_unlock(data->priv.mutex);
}

enum hash_file_state_e gtkhash_hash_file_get_state(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	enum hash_file_state_e state = data->priv.state;
	g_mutex_unlock(data->priv.mutex);

	return state;
}

void gtkhash_hash_file_set_state(struct hash_file_s *data,
	const enum hash_file_state_e state)
{
	g_mutex_lock(data->priv.mutex);
	data->priv.state = state;
	g_mutex_unlock(data->priv.mutex);
}

void gtkhash_hash_file_set_uri(struct hash_file_s *data, const char *uri)
{
	data->uri = uri;
}

void gtkhash_hash_file_cancel(struct hash_file_s *data)
{
	g_cancellable_cancel(data->cancellable);
}

bool gtkhash_hash_file_is_cancelled(struct hash_file_s *data)
{
	return g_cancellable_is_cancelled(data->cancellable);
}

static bool gtkhash_hash_file_report(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	if (data->priv.report_source)
		gtkhash_hash_file_report_cb(data->cb_data, data->file_size,
			data->priv.total_read, data->timer);
	g_mutex_unlock(data->priv.mutex);

	return true;
}

static void gtkhash_hash_file_add_report_source(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	if (G_UNLIKELY(data->priv.report_source))
		g_error("report source was already added");
	data->priv.report_source = gdk_threads_add_timeout(
		HASH_FILE_REPORT_INTERVAL, (GSourceFunc)gtkhash_hash_file_report, data);
	g_mutex_unlock(data->priv.mutex);
}

static void gtkhash_hash_file_remove_report_source(struct hash_file_s *data)
{
	g_mutex_lock(data->priv.mutex);
	if (data->priv.report_source) {
		if (G_UNLIKELY(!g_source_remove(data->priv.report_source)))
			g_error("failed to remove report source");
		data->priv.report_source = 0;
	}
	g_mutex_unlock(data->priv.mutex);
}

static void gtkhash_hash_file_start(struct hash_file_s *data)
{
	g_assert(data->uri);

	g_cancellable_reset(data->cancellable);

	int funcs_n = 0;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (data->funcs[i].enabled) {
			gtkhash_hash_lib_start(&data->funcs[i]);
			funcs_n++;
		}
	}

	int threads_n = CLAMP(MIN(funcs_n, sysconf(_SC_NPROCESSORS_ONLN)), 1,
		HASH_FUNCS_N);
	g_atomic_int_set(&data->pool_threads_n, 0);
	data->thread_pool = g_thread_pool_new((GFunc)gtkhash_hash_file_hash_thread,
		data, threads_n, true, NULL);

	data->buffer = g_malloc(HASH_FILE_BUFFER_SIZE);
	data->file = g_file_new_for_uri(data->uri);
	data->just_read = 0;
	data->priv.total_read = 0;
	data->timer = g_timer_new();

	gtkhash_hash_file_set_state(data, HASH_FILE_STATE_OPEN);
}

static void gtkhash_hash_file_open_finish(
	G_GNUC_UNUSED GObject *source, GAsyncResult *res, struct hash_file_s *data)
{
	data->stream = g_file_read_finish(data->file, res, NULL);
	if (G_UNLIKELY(!data->stream &&
		!g_cancellable_is_cancelled(data->cancellable)))
	{
		g_warning("failed to open file (%s)", data->uri);
		g_cancellable_cancel(data->cancellable);
	}

	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		if (data->stream)
			gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
		else
			gtkhash_hash_file_set_state(data, HASH_FILE_STATE_FINISH);
	} else
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_GET_SIZE);

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_open(struct hash_file_s *data)
{
	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_FINISH);
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_file_read_async(data->file, G_PRIORITY_DEFAULT, data->cancellable,
		(GAsyncReadyCallback)gtkhash_hash_file_open_finish, data);
}

static void gtkhash_hash_file_get_size_finish(
	G_GNUC_UNUSED GObject *source, GAsyncResult *res, struct hash_file_s *data)
{
	GFileInfo *info = g_file_input_stream_query_info_finish(
		data->stream, res, NULL);
	data->file_size = g_file_info_get_size(info);
	g_object_unref(info);

	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable)))
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
	else if (data->file_size == 0)
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_HASH);
	else {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_READ);
		gtkhash_hash_file_add_report_source(data);
	}

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_get_size(struct hash_file_s *data)
{
	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_file_input_stream_query_info_async(data->stream,
		G_FILE_ATTRIBUTE_STANDARD_SIZE, G_PRIORITY_DEFAULT, data->cancellable,
		(GAsyncReadyCallback)gtkhash_hash_file_get_size_finish, data);
}

static void gtkhash_hash_file_read_finish(
	G_GNUC_UNUSED GObject *source, GAsyncResult *res, struct hash_file_s *data)
{
	data->just_read = g_input_stream_read_finish(
		G_INPUT_STREAM(data->stream), res, NULL);

	if (G_UNLIKELY(data->just_read == -1) &&
		!g_cancellable_is_cancelled(data->cancellable))
	{
		g_warning("failed to read file (%s)", data->uri);
		g_cancellable_cancel(data->cancellable);
	} else if (G_UNLIKELY(data->just_read == 0)) {
		g_warning("unexpected EOF (%s)", data->uri);
		g_cancellable_cancel(data->cancellable);
	} else {
		g_mutex_lock(data->priv.mutex);
		data->priv.total_read += data->just_read;
		const goffset total_read = data->priv.total_read;
		g_mutex_unlock(data->priv.mutex);
		if (G_UNLIKELY(total_read > data->file_size)) {
			g_warning("read %" G_GOFFSET_FORMAT
				" more bytes than expected (%s)", total_read - data->file_size,
				data->uri);
			g_cancellable_cancel(data->cancellable);
		} else
			gtkhash_hash_file_set_state(data, HASH_FILE_STATE_HASH);
	}

	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable)))
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_read(struct hash_file_s *data)
{
	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_input_stream_read_async(G_INPUT_STREAM(data->stream),
		data->buffer, HASH_FILE_BUFFER_SIZE, G_PRIORITY_DEFAULT,
		data->cancellable, (GAsyncReadyCallback)gtkhash_hash_file_read_finish,
		data);
}

static void gtkhash_hash_file_hash_thread(void *func, struct hash_file_s *data)
{
	gtkhash_hash_lib_update(&data->funcs[GPOINTER_TO_UINT(func) - 1],
		data->buffer, data->just_read);

	if (g_atomic_int_dec_and_test(&data->pool_threads_n))
		gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_hash(struct hash_file_s *data)
{
	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
		return;
	}

	gtkhash_hash_file_remove_source(data);
	gtkhash_hash_file_set_state(data, HASH_FILE_STATE_HASH_FINISH);

	g_atomic_int_inc(&data->pool_threads_n);
	for (unsigned int i = 0; i < HASH_FUNCS_N; i++) {
		if (data->funcs[i].enabled) {
			g_atomic_int_inc(&data->pool_threads_n);
			g_thread_pool_push(data->thread_pool, GUINT_TO_POINTER(i + 1), NULL);
		}
	}

	if (g_atomic_int_dec_and_test(&data->pool_threads_n))
		gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_hash_finish(struct hash_file_s *data)
{
	g_assert(g_atomic_int_get(&data->pool_threads_n) == 0);

	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
		return;
	}

	if (data->priv.total_read >= data->file_size)
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_CLOSE);
	else
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_READ);
}

static void gtkhash_hash_file_close_finish(
	G_GNUC_UNUSED GObject *source, GAsyncResult *res, struct hash_file_s *data)
{
	if (G_UNLIKELY(!g_input_stream_close_finish(G_INPUT_STREAM(data->stream), res, NULL) &&
		!g_cancellable_is_cancelled(data->cancellable)))
	{
		g_warning("failed to close file (%s)", data->uri);
	}

	g_object_unref(data->stream);

	gtkhash_hash_file_remove_report_source(data);
	gtkhash_hash_file_set_state(data, HASH_FILE_STATE_FINISH);
	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_close(struct hash_file_s *data)
{
	gtkhash_hash_file_remove_source(data);
	g_input_stream_close_async(G_INPUT_STREAM(data->stream),
		G_PRIORITY_DEFAULT, data->cancellable,
		(GAsyncReadyCallback)gtkhash_hash_file_close_finish, data);
}

static void gtkhash_hash_file_finish(struct hash_file_s *data)
{
	if (G_UNLIKELY(g_cancellable_is_cancelled(data->cancellable))) {
		for (int i = 0; i < HASH_FUNCS_N; i++)
			if (data->funcs[i].enabled)
				gtkhash_hash_lib_stop(&data->funcs[i]);
	} else {
		for (int i = 0; i < HASH_FUNCS_N; i++)
			if (data->funcs[i].enabled)
				gtkhash_hash_lib_finish(&data->funcs[i]);
	}

	g_thread_pool_free(data->thread_pool, true, false);
	g_timer_destroy(data->timer);
	g_free(data->buffer);
	g_object_unref(data->file);

	gtkhash_hash_file_set_source(data, 0);
	gtkhash_hash_file_set_state(data, HASH_FILE_STATE_TERM);
}

static bool gtkhash_hash_file(struct hash_file_s *data)
{
	static void (* const state_funcs[])(struct hash_file_s *) = {
		[HASH_FILE_STATE_START]       = gtkhash_hash_file_start,
		[HASH_FILE_STATE_OPEN]        = gtkhash_hash_file_open,
		[HASH_FILE_STATE_GET_SIZE]    = gtkhash_hash_file_get_size,
		[HASH_FILE_STATE_READ]        = gtkhash_hash_file_read,
		[HASH_FILE_STATE_HASH]        = gtkhash_hash_file_hash,
		[HASH_FILE_STATE_HASH_FINISH] = gtkhash_hash_file_hash_finish,
		[HASH_FILE_STATE_CLOSE]       = gtkhash_hash_file_close,
		[HASH_FILE_STATE_FINISH]      = gtkhash_hash_file_finish,
		[HASH_FILE_STATE_TERM]        = NULL,
		[HASH_FILE_STATE_IDLE]        = NULL,
	};

	const enum hash_file_state_e state = gtkhash_hash_file_get_state(data);

	if (G_UNLIKELY(state == HASH_FILE_STATE_TERM)) {
		gtkhash_hash_file_set_state(data, HASH_FILE_STATE_IDLE);
		gtkhash_hash_file_finish_cb(data->cb_data);
		return false;
	}

	state_funcs[state](data);

	return true;
}

void gtkhash_hash_file_init(struct hash_file_s *data, struct hash_func_s *funcs,
	void *cb_data)
{
	data->priv.mutex = g_mutex_new();
	data->priv.source = 0;
	data->priv.report_source = 0;
	data->priv.state = HASH_FILE_STATE_IDLE;
	data->funcs = funcs;
	data->cb_data = cb_data;
	data->uri = NULL;
	data->cancellable = g_cancellable_new();
}

void gtkhash_hash_file_deinit(struct hash_file_s *data)
{
	// Shouldn't still be running
	g_assert(data->priv.source == 0);
	g_assert(data->priv.report_source == 0);

	g_mutex_free(data->priv.mutex);

	g_object_unref(data->cancellable);
}

void gtkhash_hash_file_clear_digests(struct hash_file_s *data)
{
	for (int i = 0; i < HASH_FUNCS_N; i++)
		gtkhash_hash_func_clear_digest(&data->funcs[i]);
}
