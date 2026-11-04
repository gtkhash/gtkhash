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

#ifndef GTKHASH_LIST_H
#define GTKHASH_LIST_H

#include <gtk/gtk.h>

#include "hash/hash-func.h"

void list_init(void);
int list_get_rows(void);
void list_update(void);
void list_append_row(const char *uri, const char *check);
void list_remove_selection(void);
GPtrArray *list_get_iters(void);
char *list_get_uri(GtkTreeIter *iter);
char *list_get_basename(int row);
void list_set_digest(GtkTreeIter *iter, enum hash_func_e id, const char *digest);
char *list_get_digest(int row, enum hash_func_e id);
char *list_get_selected_digest(enum hash_func_e id);
bool list_has_digests(void);
void list_check_digests(GtkTreeIter *iter);
void list_check_all_digests(void);
void list_clear_digests(void);
void list_clear(void);

#endif
