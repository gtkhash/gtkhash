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
#include <glib.h>

#include "uri-digest.h"

struct uri_digest_s *uri_digest_new(char *uri, char *digest)
{
	struct uri_digest_s *ud = g_new(struct uri_digest_s, 1);
	ud->uri = uri;
	ud->digest = digest;

	return ud;
}

void uri_digest_free_full(struct uri_digest_s *ud)
{
	if (!ud)
		return;

	g_free(ud->uri);
	g_free(ud->digest);
	g_free(ud);
}

GSList *uri_digest_list_from_uri_list(GSList *uris)
{
	GSList *ud_list = NULL;

	for (; uris; uris = uris->next)
		ud_list = g_slist_prepend(ud_list, uri_digest_new(g_strdup(uris->data), NULL));

	return g_slist_reverse(ud_list);
}

GSList *uri_digest_list_from_uri_strv(char **uris)
{
	if (!uris)
		return NULL;

	GSList *ud_list = NULL;

	for (int i = 0; uris[i]; i++)
		ud_list = g_slist_prepend(ud_list, uri_digest_new(g_strdup(uris[i]), NULL));

	return g_slist_reverse(ud_list);
}

void uri_digest_list_free_full(GSList *ud_list)
{
	g_slist_free_full(ud_list, (GDestroyNotify)uri_digest_free_full);
}
