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

#ifndef GTKHASH_HASH_FILE_H
#define GTKHASH_HASH_FILE_H

#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "hash-func.h"

enum hash_file_state_e {
	HASH_FILE_STATE_START,
	HASH_FILE_STATE_OPEN,
	HASH_FILE_STATE_GET_SIZE,
	HASH_FILE_STATE_READ,
	HASH_FILE_STATE_HASH,
	HASH_FILE_STATE_REPORT,
	HASH_FILE_STATE_CLOSE,
	HASH_FILE_STATE_FINISH,
	HASH_FILE_STATE_TERM,
	HASH_FILE_STATE_IDLE
};

struct hash_file_s {
		void *cb_data;
		const char *uri;
		GFile *file;
		GFileInputStream *stream;
		goffset file_size;
		goffset total_read;
		gssize just_read;
		uint8_t *buffer;
		GTimer *timer;
		enum hash_func_e current_func;
		struct hash_func_s *funcs;
		struct {
			GMutex *mutex;
			unsigned int source;
			bool stop;
			enum hash_file_state_e state;
		} priv;
};

unsigned int gtkhash_hash_file_get_source(struct hash_file_s *data);
void gtkhash_hash_file_add_source(struct hash_file_s *data);
bool gtkhash_hash_file_get_stop(struct hash_file_s *data);
void gtkhash_hash_file_set_stop(struct hash_file_s *data, const bool stop);
enum hash_file_state_e gtkhash_hash_file_get_state(struct hash_file_s *data);
void gtkhash_hash_file_set_state(struct hash_file_s *data,
	const enum hash_file_state_e state);
void gtkhash_hash_file_set_uri(struct hash_file_s *data, const char *uri);
void gtkhash_hash_file_init(struct hash_file_s *data, struct hash_func_s *funcs,
	void *cb_data);
void gtkhash_hash_file_deinit(struct hash_file_s *data);
void gtkhash_hash_file_clear_digests(struct hash_file_s *data);

void gtkhash_hash_file_report_cb(void *data, goffset file_size,
	goffset total_read, GTimer *timer);
void gtkhash_hash_file_finish_cb(void *data);

#endif
