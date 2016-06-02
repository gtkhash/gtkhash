/*
 *   Copyright (C) 2007-2016 Tristan Heaven <tristan@tristanheaven.net>
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

#ifndef GTKHASH_HASH_HASH_FILE_H
#define GTKHASH_HASH_HASH_FILE_H

#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "hash-func.h"

struct hash_file_s;

void gtkhash_hash_file_cancel(struct hash_file_s *data);
struct hash_file_s *gtkhash_hash_file_new(struct hash_func_s *funcs);
void gtkhash_hash_file_free(struct hash_file_s *data);
void gtkhash_hash_file_clear_digests(struct hash_file_s *data);
void gtkhash_hash_file(struct hash_file_s *data, const char *uri,
	const uint8_t *hmac_key, const size_t key_size, const void *cb_data);

// Callbacks
void gtkhash_hash_file_report_cb(void *data, goffset file_size,
	goffset total_read, GTimer *timer);
void gtkhash_hash_file_finish_cb(void *data);
void gtkhash_hash_file_stop_cb(void *data);

#endif
