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

#ifndef GTKHASH_URI_DIGEST_H
#define GTKHASH_URI_DIGEST_H

#include <glib.h>

struct uri_digest_s {
	char *uri;
	char *digest;
};

struct uri_digest_s *uri_digest_new(char *uri, char *digest);
void uri_digest_free_full(struct uri_digest_s *ud);
GSList *uri_digest_list_from_uri_list(GSList *uris);
GSList *uri_digest_list_from_uri_strv(char **uris);
void uri_digest_list_free(GSList *ud_list);
void uri_digest_list_free_full(GSList *ud_list);

#endif
