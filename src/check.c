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
#include <string.h>
#include <gtk/gtk.h>

#include "check.h"
#include "main.h"
#include "hash.h"
#include "gui.h"
#include "list.h"
#include "uri-digest.h"
#include "hash/hash-func.h"

#define CHECK_FORMAT_N 3

enum check_format_e {
	CHECK_FORMAT_UNKNOWN = -1,
	CHECK_FORMAT_BSD,
	CHECK_FORMAT_GNU,
	CHECK_FORMAT_SFV,
};

struct {
	GRegex *regex[CHECK_FORMAT_N];
	const char * const pattern[CHECK_FORMAT_N];
} check_priv = {
	.regex = {
		NULL, NULL, NULL
	},
	.pattern = {
		[CHECK_FORMAT_BSD] =
			"^[ \t]*"
			"(?<FUNCTION>[[:upper:][:digit:]-]{3,16})" // capture FUNCTION
			" \\("
			"(?<FILENAME>.+)"                          // capture FILENAME
			"\\) = "
			"(?<DIGEST>[[:xdigit:]]{8,})"              // capture DIGEST
			"\\r?$",

		[CHECK_FORMAT_GNU] =
			"^[ \t]*"
			"(?<DIGEST>[[:xdigit:]]{8,})" // capture DIGEST (8+ hex chars)
			"[ \t][ *]?"                  // '*' indicates binary mode (md5sum -b)
			"(?<FILENAME>.+)"             // capture FILENAME
			"\\r?$",

		[CHECK_FORMAT_SFV] =
			"^[ \t]*"
			"(?<FILENAME>[^;].*?[^ ]*)"   // capture FILENAME
			"[ ]+"
			"(?<DIGEST>[[:xdigit:]]{8})"  // capture DIGEST (8 hex chars)
			"\\r?$",
	},
};

static enum check_format_e check_regex_match(const char * const line,
	GMatchInfo **info)
{
	if (!line || !*line)
		return CHECK_FORMAT_UNKNOWN;

	for (enum check_format_e i = 0; i < CHECK_FORMAT_N; i++) {
		if (!check_priv.regex[i])
			continue;
		if (g_regex_match(check_priv.regex[i], line, 0, info))
			return i;
	}

	return CHECK_FORMAT_UNKNOWN;
}

static bool check_file_parse_line(const char * const line,
	enum hash_func_e *id, char **filename, char **digest)
{
	GMatchInfo *info = NULL;

	if (check_regex_match(line, &info) != CHECK_FORMAT_UNKNOWN) {
		char *function = g_match_info_fetch_named(info, "FUNCTION");
		if (function) {
			*id = gtkhash_hash_func_get_id_from_name(function);
			g_free(function);
		}

		*filename = g_match_info_fetch_named(info, "FILENAME");
		*digest = g_match_info_fetch_named(info, "DIGEST");

		g_match_info_free(info);
		return true;
	} else {
		g_match_info_free(info);
		return false;
	}
}

static void check_file_enable_hinted_hash_func(GFile *file)
{
	static const struct {
		const char * const suffix;
		const enum hash_func_e id;
	} hints[] = {
		{ ".md5",       HASH_FUNC_MD5 },
		{ ".md5sum",    HASH_FUNC_MD5 },
		{ ".sfv",       HASH_FUNC_CRC32 },
		{ ".sha1",      HASH_FUNC_SHA1 },
		{ ".sha1sum",   HASH_FUNC_SHA1 },
		{ ".sha224",    HASH_FUNC_SHA224 },
		{ ".sha224sum", HASH_FUNC_SHA224 },
		{ ".sha256",    HASH_FUNC_SHA256 },
		{ ".sha256sum", HASH_FUNC_SHA256 },
		{ ".sha384",    HASH_FUNC_SHA384 },
		{ ".sha384sum", HASH_FUNC_SHA384 },
		{ ".sha512",    HASH_FUNC_SHA512 },
		{ ".sha512sum", HASH_FUNC_SHA512 },
	};

	char *basename = g_file_get_basename(file);
	if (!basename)
		return;

	const size_t len = strlen(basename);

	for (size_t i = 0; i < G_N_ELEMENTS(hints); i++) {
		const size_t suffix_len = strlen(hints[i].suffix);
		if (len <= suffix_len)
			continue;
		if (g_ascii_strcasecmp(basename + len - suffix_len, hints[i].suffix) == 0) {
			gui_enable_hash_func(hints[i].id);
			break;
		}
	}

	g_free(basename);
}

static GSList *check_file_add_uri(GSList *ud_list, GFile *file,
	const char * const filename, const char * const digest)
{
	g_assert(file);
	g_assert(filename && *filename);
	g_assert(digest && *digest);

	GFile *target = NULL;

	if (g_path_is_absolute(filename)) {
		target = g_file_new_for_path(filename);
	} else {
		// Assume path is relative to the check file
		GFile *dir = g_file_get_parent(file);
		if (!dir)
			return ud_list;

		if (g_file_is_native(dir)) {
			// Native path
			char *dir_path = g_file_get_path(dir);
			if (dir_path) {
				char *target_path = g_build_filename(dir_path, filename, NULL);
				target = g_file_new_for_path(target_path);
				g_free(target_path);
				g_free(dir_path);
			}
		} else {
			// URI
			char *dir_uri = g_file_get_uri(dir);
			if (dir_uri) {
				char **segments = g_strsplit(filename, "/", -1);
				GString *uri = g_string_new(dir_uri);
				for (int i = 0; segments[i]; i++) {
					if (!*segments[i])
						continue;
					g_string_append_c(uri, '/');
					g_string_append_uri_escaped(uri, segments[i], NULL, true);
				}
				g_strfreev(segments);
				char *target_uri = g_string_free(uri, false);
				target = g_file_new_for_uri(target_uri);
				g_free(target_uri);
				g_free(dir_uri);
			}
		}

		g_object_unref(dir);
	}

	if (!target)
		return ud_list;

	// '\' is valid in filenames but could be intended as a dir separator
	if (strchr(filename, '\\') && !g_file_query_exists(target, NULL)) {
		char *filename2 = g_strdelimit(g_strdup(filename), "\\", '/');
		ud_list = check_file_add_uri(ud_list, file, filename2, digest);
		g_free(filename2);
	} else {
		ud_list = g_slist_prepend(ud_list, uri_digest_new(g_file_get_uri(target),
			g_strdup(digest)));
	}

	g_object_unref(target);

	return ud_list;
}

static void check_file_error(GFile *file, GError *error)
{
	g_assert(error);

	char *uri = g_file_get_uri(file);
	g_message(_("Failed to read check file \"%s\": %s"), uri,
		error->message);
	g_free(uri);
}

GSList *check_file_load(GSList *ud_list, GFile *file)
{
	g_assert(file);

	char *data = NULL;
	GError *error = NULL;
	GFileInputStream *fis = g_file_read(file, NULL, &error);

	if (!fis) {
		check_file_error(file, error);
		g_error_free(error);
		return ud_list;
	}

	GDataInputStream *dis = g_data_input_stream_new((GInputStream *)fis);

	char *line = NULL;
	gsize len = 0;
	error = NULL;

	while ((line = g_data_input_stream_read_line_utf8(dis, &len, NULL, &error))) {
		enum hash_func_e id = HASH_FUNC_INVALID;
		char *filename = NULL;
		char *digest = NULL;

		if ((len >= 8) &&
			check_file_parse_line(line, &id, &filename, &digest))
		{
			if (HASH_FUNC_IS_VALID(id))
				gui_enable_hash_func(id);

			ud_list = check_file_add_uri(ud_list, file, filename, digest);
			g_free(filename);
			g_free(digest);
		}

		g_free(line);
	}

	if (error) {
		check_file_error(file, error);
		g_error_free(error);
	} else
		check_file_enable_hinted_hash_func(file);

	g_object_unref(dis);
	g_object_unref(fis);
	g_free(data);

	return ud_list;
}

void check_file_save(const char * const filename)
{
	g_assert(filename);

	GString *string = g_string_sized_new(1024);

	for (enum hash_func_e i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		switch (gui.view) {
			case GUI_VIEW_FILE: {
				const char *digest = gtk_entry_get_text(
					gui.hash_widgets[i].entry_file);
				if (!(digest && *digest))
					continue;

				const bool hmac_active = gtk_toggle_button_get_active(
					gui.togglebutton_hmac_file);

				g_string_append_printf(string,
					(hmac_active && hash.funcs[i].hmac_supported) ?
					"# HMAC-%s\n" : "# %s\n", hash.funcs[i].name);

				GFile *file = gtk_file_chooser_get_file(
					GTK_FILE_CHOOSER(gui.filechooserbutton));
				char *basename = g_file_get_basename(file);

				g_string_append_printf(string, "%s  %s\n",
					gtk_entry_get_text(gui.hash_widgets[i].entry_file),
						basename);

				g_free(basename);
				g_object_unref(file);

				break;
			}
			case GUI_VIEW_TEXT: {
				const bool hmac_active = gtk_toggle_button_get_active(
					gui.togglebutton_hmac_text);

				g_string_append_printf(string,
					(hmac_active && hash.funcs[i].hmac_supported) ?
					"# HMAC-%s\n" : "# %s\n", hash.funcs[i].name);
				g_string_append_printf(string, "%s  \"%s\"\n",
					gtk_entry_get_text(gui.hash_widgets[i].entry_text),
					gtk_entry_get_text(gui.entry_text));

				break;
			}
			case GUI_VIEW_FILE_LIST: {
				enum hash_func_e prev = HASH_FUNC_INVALID;

				for (unsigned int row = 0; row < list.rows; row++) {
					char *digest = list_get_digest(row, i);

					if (digest && *digest) {
						if (i != prev) {
							g_string_append_printf(string, "# %s\n",
								hash.funcs[i].name);
						}
						char *basename = list_get_basename(row);
						g_string_append_printf(string, "%s  %s\n",
							digest, basename);
						g_free(basename);
					}

					g_free(digest);
					prev = i;
				}
				break;
			}
			default:
				g_assert_not_reached();
		}
	}

	char *data = g_string_free(string, false);
	g_file_set_contents(filename, data, -1, NULL);

	g_free(data);
}

void check_file_add_filters(GtkFileFilter *filter)
{
	gtk_file_filter_add_mime_type(filter, "application/x-md5");
	gtk_file_filter_add_mime_type(filter, "application/x-sha1");
	gtk_file_filter_add_mime_type(filter, "text/x-sfv");

	gtk_file_filter_add_pattern(filter, "*.md5");
	gtk_file_filter_add_pattern(filter, "*.sfv");
	gtk_file_filter_add_pattern(filter, "*.sha1");
	gtk_file_filter_add_pattern(filter, "*.sha224");
	gtk_file_filter_add_pattern(filter, "*.sha256");
	gtk_file_filter_add_pattern(filter, "*.sha384");
	gtk_file_filter_add_pattern(filter, "*.sha512");

	gtk_file_filter_add_pattern(filter, "*md5sum*");
	gtk_file_filter_add_pattern(filter, "*sha1sum*");
	gtk_file_filter_add_pattern(filter, "*sha224sum*");
	gtk_file_filter_add_pattern(filter, "*sha256sum*");
	gtk_file_filter_add_pattern(filter, "*sha384sum*");
	gtk_file_filter_add_pattern(filter, "*sha512sum*");

	gtk_file_filter_add_pattern(filter, "*MD5SUM*");
	gtk_file_filter_add_pattern(filter, "*SHA1SUM*");
	gtk_file_filter_add_pattern(filter, "*SHA224SUM*");
	gtk_file_filter_add_pattern(filter, "*SHA256SUM*");
	gtk_file_filter_add_pattern(filter, "*SHA384SUM*");
	gtk_file_filter_add_pattern(filter, "*SHA512SUM*");

	gtk_file_filter_add_pattern(filter, "*CHECKSUM*");
	gtk_file_filter_add_pattern(filter, "*DIGEST*");
}

void check_init(void)
{
	for (enum check_format_e i = 0; i < CHECK_FORMAT_N; i++) {
		GError *error = NULL;
		check_priv.regex[i] = g_regex_new(check_priv.pattern[i], 0, 0, &error);

		if (error) {
			g_warning("g_regex_new(): %s", error->message);
			g_error_free(error);
			error = NULL;
		}
	}
}

void check_deinit(void)
{
	for (enum check_format_e i = 0; i < CHECK_FORMAT_N; i++) {
		if (check_priv.regex[i]) {
			g_regex_unref(check_priv.regex[i]);
			check_priv.regex[i] = NULL;
		}
	}
}
