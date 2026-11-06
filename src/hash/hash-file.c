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
#include <string.h>
#include <sys/stat.h>
#include <glib.h>

#include "hash-file.h"
#include "hash-func.h"
#include "hash-lib.h"
#include "digest-format.h"

#define HASH_FILE_BUFFER_SIZE (1 << 23) // File read buffer size (bytes)
#define HASH_FILE_REPORT_INTERVAL 100 // Progress report interval (ms)

static gboolean gtkhash_hash_file_source_func(struct hash_file_s *data);
static void gtkhash_hash_file_hash_thread_func(struct hash_func_s *func, struct hash_file_s *data);

enum hash_file_type_e {
	HASH_FILE_TYPE_REGULAR,
	HASH_FILE_TYPE_BLOCK_DEVICE,
	HASH_FILE_TYPE_CHARACTER_DEVICE,
	HASH_FILE_TYPE_UNSUPPORTED,
};

enum hash_file_state_e {
	HASH_FILE_STATE_IDLE,
	HASH_FILE_STATE_START,
	HASH_FILE_STATE_OPEN,
	HASH_FILE_STATE_GET_SIZE,
	HASH_FILE_STATE_READ,
	HASH_FILE_STATE_HASH,
	HASH_FILE_STATE_HASH_DONE,
	HASH_FILE_STATE_CLOSE,
	HASH_FILE_STATE_FINISH,
	HASH_FILE_STATE_CALLBACK,
};

struct hash_file_s {
	goffset file_size, total_read;
	const void *cb_data;
	const char *uri;
	GFile *file;
	const uint8_t *hmac_key;
	size_t key_size;
	GCancellable *cancellable;
	GFileInputStream *stream;
	gssize just_read;
	uint8_t *buffer;
	GTimer *timer;
	GThreadPool *thread_pool;
	struct hash_func_s *funcs;
	enum hash_file_state_e state;
	enum digest_format_e format;
	int threads;
	unsigned int report_source;
	unsigned int source;
	bool size_known;
	GMutex mtx;
};

static void gtkhash_hash_file_add_source(struct hash_file_s *data)
{
	g_mutex_lock(&data->mtx);
	g_assert(!data->source);
	data->source = g_idle_add(G_SOURCE_FUNC(gtkhash_hash_file_source_func), data);
	g_mutex_unlock(&data->mtx);
}

static void gtkhash_hash_file_remove_source(struct hash_file_s *data)
{
	g_mutex_lock(&data->mtx);
	if (!g_source_remove(data->source))
		g_assert_not_reached();
	data->source = 0;
	g_mutex_unlock(&data->mtx);
}

void gtkhash_hash_file_cancel(struct hash_file_s *data)
{
	g_cancellable_cancel(data->cancellable);
}

static gboolean gtkhash_hash_file_report_source_func(struct hash_file_s *data)
{
	if (data->report_source && data->total_read) {
		if (data->size_known)
			gtkhash_hash_file_report_cb((void *)data->cb_data, data->file_size,
				data->total_read, data->timer);
		else
			gtkhash_hash_file_report_unknown_cb((void *)data->cb_data,
				data->total_read, data->timer);
	}

	return G_SOURCE_CONTINUE;
}

static void gtkhash_hash_file_add_report_source(struct hash_file_s *data)
{
	g_assert(!data->report_source);
	data->report_source = g_timeout_add(HASH_FILE_REPORT_INTERVAL,
		G_SOURCE_FUNC(gtkhash_hash_file_report_source_func), data);
}

static void gtkhash_hash_file_remove_report_source(struct hash_file_s *data)
{
	if (data->report_source) {
		if (!g_source_remove(data->report_source))
			g_assert_not_reached();
		data->report_source = 0;
	}
}

GFileInfo *gtkhash_hash_file_query_info(GFile *file, GCancellable *cancellable, GError **error)
{
	if (!file)
		return NULL;

	return g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_UNIX_MODE,
		G_FILE_QUERY_INFO_NONE, cancellable, error);
}

static enum hash_file_type_e gtkhash_hash_file_info_get_type(GFileInfo *info)
{
	switch (g_file_info_get_file_type(info)) {
		case G_FILE_TYPE_REGULAR:
			return HASH_FILE_TYPE_REGULAR;
		case G_FILE_TYPE_SPECIAL:
			break;
		default:
			return HASH_FILE_TYPE_UNSUPPORTED;
	}

	if (!g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE))
		return HASH_FILE_TYPE_UNSUPPORTED;

	const uint32_t mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
	if (S_ISBLK(mode))
		return HASH_FILE_TYPE_BLOCK_DEVICE;
	if (S_ISCHR(mode))
		return HASH_FILE_TYPE_CHARACTER_DEVICE;

	return HASH_FILE_TYPE_UNSUPPORTED;
}

bool gtkhash_hash_file_info_is_supported(GFileInfo *info)
{
	if (!info)
		return false;

	return gtkhash_hash_file_info_get_type(info) != HASH_FILE_TYPE_UNSUPPORTED;
}

bool gtkhash_hash_file_type_is_supported(const GFileType file_type, const char *mime_type,
	const char *uri)
{
	if (!uri || !*uri)
		return false;

	switch (file_type) {
		case G_FILE_TYPE_REGULAR:
			return true;
		case G_FILE_TYPE_SYMBOLIC_LINK:
		case G_FILE_TYPE_SPECIAL:
			break;
		default:
			return false;
	}

	if (g_strcmp0(mime_type, "inode/blockdevice") == 0)
		return true;
	if (g_strcmp0(mime_type, "inode/chardevice") == 0)
		return true;

	GFile *file = g_file_new_for_uri(uri);

	// Non-native symlinks and devices could work, but it's safer to avoid the I/O query
	if (!g_file_is_native(file)) {
		g_object_unref(file);
		return false;
	}

	// Blocking I/O is required to determine the type
	GFileInfo *info = gtkhash_hash_file_query_info(file, NULL, NULL);

	g_object_unref(file);

	if (!info)
		return false;

	const bool supported = gtkhash_hash_file_info_is_supported(info);

	g_object_unref(info);

	return supported;
}

static void gtkhash_hash_file_start(struct hash_file_s *data)
{
	g_assert(data->uri);

	int funcs_enabled = 0;

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (data->funcs[i].enabled) {
			gtkhash_hash_lib_start(&data->funcs[i], data->hmac_key, data->key_size);
			funcs_enabled++;
		}
	}

	g_assert(funcs_enabled > 0);

	// Determine max number of threads to use
	const int cpus = g_get_num_processors();
	const int threads = CLAMP(MIN(funcs_enabled, cpus), 1, HASH_FUNCS_N);

	g_atomic_int_set(&data->threads, 0);
	data->thread_pool = g_thread_pool_new(
		(GFunc)gtkhash_hash_file_hash_thread_func, data, threads, true, NULL);

	data->file = g_file_new_for_uri(data->uri);
	data->just_read = 0;
	data->buffer = g_malloc(HASH_FILE_BUFFER_SIZE);
	data->timer = g_timer_new();
	data->total_read = 0;
	data->file_size = 0;
	data->size_known = false;

	data->state = HASH_FILE_STATE_OPEN;
}

static void gtkhash_hash_file_open_finish(G_GNUC_UNUSED GObject *source,
	GAsyncResult *res, struct hash_file_s *data)
{
	data->stream = g_file_read_finish(data->file, res, NULL);
	if (!data->stream && !g_cancellable_is_cancelled(data->cancellable)) {
		g_warning("failed to open file (%s)", data->uri);
		g_cancellable_cancel(data->cancellable);
	}

	if (g_cancellable_is_cancelled(data->cancellable)) {
		if (data->stream)
			data->state = HASH_FILE_STATE_CLOSE;
		else
			data->state = HASH_FILE_STATE_FINISH;
	} else
		data->state = HASH_FILE_STATE_GET_SIZE;

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_open(struct hash_file_s *data)
{
	if (g_cancellable_is_cancelled(data->cancellable)) {
		data->state = HASH_FILE_STATE_FINISH;
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_file_read_async(data->file, G_PRIORITY_DEFAULT, data->cancellable,
		(GAsyncReadyCallback)gtkhash_hash_file_open_finish, data);
}

static goffset gtkhash_hash_file_get_block_device_size(GFileInputStream *stream,
	GCancellable *cancellable, GError **error)
{
	g_assert(error);

	GSeekable *seekable = G_SEEKABLE(stream);

	if (!g_seekable_can_seek(seekable)) {
		g_set_error(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, "stream is not seekable");
		return -1;
	}

	if (!g_seekable_seek(seekable, 0, G_SEEK_END, cancellable, error))
		return -1;

	const goffset size = g_seekable_tell(seekable);

	// Remember to rewind
	if (!g_seekable_seek(seekable, 0, G_SEEK_SET, cancellable, error))
		return -1;

	return size;
}

static void gtkhash_hash_file_get_size_finish(G_GNUC_UNUSED GObject *source, GAsyncResult *res,
	struct hash_file_s *data)
{
	GError *error = NULL;
	GFileInfo *info = g_file_input_stream_query_info_finish(data->stream, res, &error);

	if (g_cancellable_is_cancelled(data->cancellable)) {
		g_clear_error(&error);
		g_clear_object(&info);

		data->state = HASH_FILE_STATE_CLOSE;
	} else if (!info) {
		g_warning("failed to query info (%s): %s", data->uri, error->message);
		g_clear_error(&error);

		data->state = HASH_FILE_STATE_CLOSE;
		g_cancellable_cancel(data->cancellable);
	} else {
		switch (gtkhash_hash_file_info_get_type(info)) {
			case HASH_FILE_TYPE_REGULAR:
				data->size_known = true;
				data->file_size = g_file_info_get_size(info);
				if (data->file_size == 0)
					data->state = HASH_FILE_STATE_HASH;
				else {
					data->state = HASH_FILE_STATE_READ;
					gtkhash_hash_file_add_report_source(data);
				}
				break;

			case HASH_FILE_TYPE_BLOCK_DEVICE:
				data->file_size = gtkhash_hash_file_get_block_device_size(data->stream,
					data->cancellable, &error);
				if (data->file_size < 0) {
					g_warning("failed to get size (%s): %s", data->uri, error->message);
					g_clear_error(&error);
					data->state = HASH_FILE_STATE_CLOSE;
					g_cancellable_cancel(data->cancellable);
				} else {
					data->size_known = true;
					if (data->file_size == 0)
						data->state = HASH_FILE_STATE_HASH;
					else {
						data->state = HASH_FILE_STATE_READ;
						gtkhash_hash_file_add_report_source(data);
					}
				}
				break;

			case HASH_FILE_TYPE_CHARACTER_DEVICE:
				data->size_known = false;
				data->file_size = 0;
				data->state = HASH_FILE_STATE_READ;
				gtkhash_hash_file_add_report_source(data);
				break;

			default:
				g_warning("unsupported file type (%s)", data->uri);
				data->state = HASH_FILE_STATE_CLOSE;
				g_cancellable_cancel(data->cancellable);
				break;
		}

		g_object_unref(info);
	}

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_get_size(struct hash_file_s *data)
{
	if (g_cancellable_is_cancelled(data->cancellable)) {
		data->state = HASH_FILE_STATE_CLOSE;
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_file_input_stream_query_info_async(data->stream,
		G_FILE_ATTRIBUTE_STANDARD_TYPE ","
		G_FILE_ATTRIBUTE_STANDARD_SIZE ","
		G_FILE_ATTRIBUTE_UNIX_MODE,
		G_PRIORITY_DEFAULT, data->cancellable,
		(GAsyncReadyCallback)gtkhash_hash_file_get_size_finish, data);
}

static void gtkhash_hash_file_read_finish(G_GNUC_UNUSED GObject *source,
	GAsyncResult *res, struct hash_file_s *data)
{
	data->just_read = g_input_stream_read_finish(
		G_INPUT_STREAM(data->stream), res, NULL);

	if (data->just_read == -1 && !g_cancellable_is_cancelled(data->cancellable)) {
		g_warning("failed to read file (%s)", data->uri);
		g_cancellable_cancel(data->cancellable);
	} else if (data->just_read == 0) {
		if (!data->size_known) {
			// Normal EOF for character device
			data->state = HASH_FILE_STATE_CLOSE;
		} else {
			g_warning("unexpected EOF (%s)", data->uri);
			g_cancellable_cancel(data->cancellable);
		}
	} else if (data->just_read > 0) {
		if (data->total_read == 0) {
			// Resetting here excludes the initial OPEN and GET_SIZE time
			g_timer_reset(data->timer);
		}
		data->total_read += data->just_read;
		if (data->size_known && (data->total_read > data->file_size)) {
			g_warning("read %" G_GOFFSET_FORMAT
				" more bytes than expected (%s)", data->total_read -
				data->file_size, data->uri);
			g_cancellable_cancel(data->cancellable);
		} else
			data->state = HASH_FILE_STATE_HASH;
	}

	if (g_cancellable_is_cancelled(data->cancellable))
		data->state = HASH_FILE_STATE_CLOSE;

	gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_read(struct hash_file_s *data)
{
	if (g_cancellable_is_cancelled(data->cancellable)) {
		data->state = HASH_FILE_STATE_CLOSE;
		return;
	}

	gtkhash_hash_file_remove_source(data);
	g_input_stream_read_async(G_INPUT_STREAM(data->stream),
		data->buffer, HASH_FILE_BUFFER_SIZE, G_PRIORITY_DEFAULT,
		data->cancellable, (GAsyncReadyCallback)gtkhash_hash_file_read_finish,
		data);
}

static void gtkhash_hash_file_hash_thread_func(struct hash_func_s *func, struct hash_file_s *data)
{
	gtkhash_hash_lib_update(func, data->buffer, data->just_read);

	if (g_atomic_int_dec_and_test(&data->threads))
		gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_hash(struct hash_file_s *data)
{
	if (g_cancellable_is_cancelled(data->cancellable)) {
		data->state = HASH_FILE_STATE_CLOSE;
		return;
	}

	gtkhash_hash_file_remove_source(data);
	data->state = HASH_FILE_STATE_HASH_DONE;

	g_atomic_int_inc(&data->threads);
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (data->funcs[i].enabled) {
			g_atomic_int_inc(&data->threads);
			g_thread_pool_push(data->thread_pool, &data->funcs[i], NULL);
		}
	}

	if (g_atomic_int_dec_and_test(&data->threads))
		gtkhash_hash_file_add_source(data);
}

static void gtkhash_hash_file_hash_done(struct hash_file_s *data)
{
	g_assert(g_atomic_int_get(&data->threads) == 0);

	if (g_cancellable_is_cancelled(data->cancellable)) {
		data->state = HASH_FILE_STATE_CLOSE;
		return;
	}

	if (data->size_known && (data->total_read >= data->file_size))
		data->state = HASH_FILE_STATE_CLOSE;
	else
		data->state = HASH_FILE_STATE_READ;
}

static void gtkhash_hash_file_close_finish(G_GNUC_UNUSED GObject *source, GAsyncResult *res,
	struct hash_file_s *data)
{
	if (!g_input_stream_close_finish(G_INPUT_STREAM(data->stream), res, NULL) &&
		!g_cancellable_is_cancelled(data->cancellable))
	{
		g_warning("failed to close file (%s)", data->uri);
	}

	g_clear_object(&data->stream);

	gtkhash_hash_file_remove_report_source(data);
	data->state = HASH_FILE_STATE_FINISH;
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
	if (g_cancellable_is_cancelled(data->cancellable)) {
		for (int i = 0; i < HASH_FUNCS_N; i++)
			if (data->funcs[i].enabled)
				gtkhash_hash_lib_stop(&data->funcs[i]);
	} else {
		for (int i = 0; i < HASH_FUNCS_N; i++)
			if (data->funcs[i].enabled)
				gtkhash_hash_lib_finish(&data->funcs[i]);
	}

	g_clear_object(&data->file);
	g_clear_pointer(&data->buffer, g_free);
	g_clear_pointer(&data->timer, g_timer_destroy);

	g_thread_pool_free(data->thread_pool, true, false);
	data->thread_pool = NULL;

	data->state = HASH_FILE_STATE_CALLBACK;
}

static gboolean gtkhash_hash_file_callback_stop_func(void *cb_data)
{
	gtkhash_hash_file_stop_cb(cb_data);

	return G_SOURCE_REMOVE;
}

static gboolean gtkhash_hash_file_callback_finish_func(struct hash_file_s *data)
{
	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!data->funcs[i].enabled)
			continue;

		char *digest = gtkhash_hash_func_get_digest(&data->funcs[i],
			data->format);

		gtkhash_hash_file_digest_cb(i, digest, (void *)data->cb_data);

		g_free(digest);

		gtkhash_hash_func_clear_digest(&data->funcs[i]);
	}

	gtkhash_hash_file_finish_cb((void *)data->cb_data);

	return G_SOURCE_REMOVE;
}

static void gtkhash_hash_file_callback(struct hash_file_s *data)
{
	gtkhash_hash_file_remove_source(data);
	data->state = HASH_FILE_STATE_IDLE;

	if (g_cancellable_is_cancelled(data->cancellable))
		g_idle_add(gtkhash_hash_file_callback_stop_func, (void *)data->cb_data);
	else
		g_idle_add(G_SOURCE_FUNC(gtkhash_hash_file_callback_finish_func), data);

	g_clear_object(&data->cancellable);
}

static gboolean gtkhash_hash_file_source_func(struct hash_file_s *data)
{
	static void (* const state_funcs[])(struct hash_file_s *) = {
		[HASH_FILE_STATE_IDLE]        = NULL,
		[HASH_FILE_STATE_START]       = gtkhash_hash_file_start,
		[HASH_FILE_STATE_OPEN]        = gtkhash_hash_file_open,
		[HASH_FILE_STATE_GET_SIZE]    = gtkhash_hash_file_get_size,
		[HASH_FILE_STATE_READ]        = gtkhash_hash_file_read,
		[HASH_FILE_STATE_HASH]        = gtkhash_hash_file_hash,
		[HASH_FILE_STATE_HASH_DONE]   = gtkhash_hash_file_hash_done,
		[HASH_FILE_STATE_CLOSE]       = gtkhash_hash_file_close,
		[HASH_FILE_STATE_FINISH]      = gtkhash_hash_file_finish,
		[HASH_FILE_STATE_CALLBACK]    = gtkhash_hash_file_callback,
	};

	state_funcs[data->state](data);

	return G_SOURCE_CONTINUE;
}

void gtkhash_hash_file(struct hash_file_s *data, const char *uri,
	const enum digest_format_e format, const uint8_t *hmac_key,
	const size_t key_size, const void *cb_data)
{
	g_assert(data);
	g_assert(uri && *uri);
	g_assert(DIGEST_FORMAT_IS_VALID(format));
	g_assert(data->state == HASH_FILE_STATE_IDLE);
	g_assert(data->report_source == 0);
	g_assert(data->source == 0);
	g_assert(!data->cancellable);

	data->uri = uri;
	data->format = format;
	data->hmac_key = hmac_key;
	data->key_size = key_size;
	data->cb_data = cb_data;
	data->cancellable = g_cancellable_new();

	data->state = HASH_FILE_STATE_START;
	gtkhash_hash_file_add_source(data);
}

struct hash_file_s *gtkhash_hash_file_new(struct hash_func_s *funcs)
{
	struct hash_file_s *data = g_new(struct hash_file_s, 1);

	data->file_size = 0;
	data->total_read = 0;
	data->cb_data = NULL;
	data->uri = NULL;
	data->file = NULL;
	data->hmac_key = NULL;
	data->key_size = 0;
	data->cancellable = NULL;
	data->stream = NULL;
	data->just_read = 0;
	data->buffer = NULL;
	data->timer = NULL;
	data->thread_pool = NULL;
	data->funcs = funcs;
	data->state = HASH_FILE_STATE_IDLE;
	data->format = DIGEST_FORMAT_INVALID;
	g_atomic_int_set(&data->threads, 0);
	data->report_source = 0;
	data->source = 0;
	data->size_known = false;
	g_mutex_init(&data->mtx);

	return data;
}

void gtkhash_hash_file_free(struct hash_file_s *data)
{
	g_assert(data);

	// Shouldn't still be running
	g_assert(data->state == HASH_FILE_STATE_IDLE);
	g_assert(data->report_source == 0);
	g_assert(data->source == 0);

	g_mutex_clear(&data->mtx);

	g_free(data);
}
