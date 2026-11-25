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

#define LIST_FILTER_IS_VALID(X) (((X) >= 0) && ((X) <= LIST_FILTER_ERROR))

enum list_filter_e {
	LIST_FILTER_ALL,
	LIST_FILTER_PENDING,
	LIST_FILTER_MATCH,
	LIST_FILTER_MISMATCH,
	LIST_FILTER_ERROR,
};

void list_init(void);
void list_deinit(void);
int list_get_store_rows(void);
void list_refilter(void);
void list_set_filter(enum list_filter_e filter);
void list_update_status(void);
void list_update_status_now(void);
void list_update(void);
void list_append_row(const char *uri, const char *check, const char *icon, const char *error);
void list_remove_selection(void);
char *list_get_uri(GtkTreeIter *store_iter);
char *list_get_basename(GtkTreeIter *store_iter);
void list_set_error(GtkTreeIter *store_iter, const char *message, const char *icon);
char *list_get_error(GtkTreeIter *filter_iter);
GPtrArray *list_get_store_iters(void);
GPtrArray *list_get_visible_iters(void);
void list_set_digests(GtkTreeIter *store_iter, const char * const *digests);
char *list_get_digest(GtkTreeIter *store_iter, enum hash_func_e id);
char *list_get_selected_digest(enum hash_func_e id);
void list_check_all_digests(void);
void list_clear_digests(void);
void list_clear_digests_func(enum hash_func_e id);
void list_clear(void);

#endif
