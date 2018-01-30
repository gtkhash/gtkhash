/*
 *   Copyright (C) 2007-2018 Tristan Heaven <tristan@tristanheaven.net>
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

#ifndef GTKHASH_LIST_H
#define GTKHASH_LIST_H

#include "hash/hash-func.h"

extern struct list_s {
	unsigned int rows;
} list;

void list_init(void);
void list_update(void);
void list_append_row(const char *uri, const char *check);
void list_remove_selection(void);
char *list_get_uri(unsigned int row);
char *list_get_basename(unsigned int row);
void list_set_digest(unsigned int row, enum hash_func_e id, const char *digest);
char *list_get_digest(unsigned int row, enum hash_func_e id);
char *list_get_selected_digest(enum hash_func_e id);
void list_check_digests(unsigned int row);
void list_clear_digests(void);
void list_clear(void);

#endif
