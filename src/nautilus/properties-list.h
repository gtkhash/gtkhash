/*
 *   Copyright (C) 2007-2020 Tristan Heaven <tristan@tristanheaven.net>
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

#ifndef GTKHASH_NAUTILUS_PROPERTIES_LIST_H
#define GTKHASH_NAUTILUS_PROPERTIES_LIST_H

#include "properties.h"

bool gtkhash_properties_list_update_enabled(struct page_s *page,
	char *path_str);
void gtkhash_properties_list_reset_enabled(struct page_s *page);
void gtkhash_properties_list_update_hash_func_names(struct page_s *page);
void gtkhash_properties_list_clear_digests(struct page_s *page);
void gtkhash_properties_list_check_digests(struct page_s *page);
void gtkhash_properties_list_set_digest(struct page_s *page, enum hash_func_e id,
	const char *digest);
char *gtkhash_properties_list_get_selected_digest(struct page_s *page);
bool gtkhash_properties_list_hash_selected(struct page_s *page);
void gtkhash_properties_list_refilter(struct page_s *page);
void gtkhash_properties_list_init(struct page_s *page);

#endif

