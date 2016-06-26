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
			"^"
			"(?<FUNCTION>[[:upper:][:digit:]-]{3,16})" // capture FUNCTION
			" \\("
			"(?<FILENAME>.+)"                          // capture FILENAME
			"\\) = "
			"(?<DIGEST>[[:xdigit:]]{8,})"              // capture DIGEST
			"$",

		[CHECK_FORMAT_GNU] =
			"^"
			"(?<DIGEST>[[:xdigit:]]{8,})" // capture DIGEST (8+ hex chars)
			" [* ]"                       // * indicates binary mode
			"(?<FILENAME>.+)"             // capture FILENAE
			"$",

		[CHECK_FORMAT_SFV] =
			"^"
			"(?<FILENAME>[^;].*?[^ ]*)"   // capture FILENAME
			"[ ]+"
			"(?<DIGEST>[[:xdigit:]]{8})"  // capture DIGEST (8 hex chars)
			"$",
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
	char *basename = g_file_get_basename(file);
	if (!basename)
		return;

	size_t len = strlen(basename);

#define MATCH(STR, LEN, FUNC) \
	g_assert(strlen(STR) == LEN); \
	if (g_ascii_strcasecmp(basename + len - LEN, STR) == 0) { \
		gui_enable_hash_func(G_PASTE(HASH_FUNC_, FUNC)); \
		break; \
	}

	do {
		if (len > 4) {
			MATCH(".md5", 4, MD5);
			MATCH(".sfv", 4, CRC32);
		} else
			break;
		if (len > 5) {
			MATCH(".sha1", 5, SHA1);
		} else
			break;
		if (len > 7) {
			MATCH(".md5sum", 7, MD5);
			MATCH(".sha224", 7, SHA224);
			MATCH(".sha256", 7, SHA256);
			MATCH(".sha384", 7, SHA384);
			MATCH(".sha512", 7, SHA512);
		} else
			break;
		if (len > 8) {
			MATCH(".sha1sum", 8, SHA1);
		} else
			break;
		if (len > 10) {
			MATCH(".sha224sum", 10, SHA224);
			MATCH(".sha256sum", 10, SHA256);
			MATCH(".sha384sum", 10, SHA384);
			MATCH(".sha512sum", 10, SHA512);
		} else
			break;
	} while (false);

#undef MATCH

	g_free(basename);
}

static GSList *check_file_add_uri(GSList *ud_list, GFile *file, char *filename,
	char *digest)
{
	g_assert(file);
	g_assert(filename && *filename);
	g_assert(digest && *digest);

	char *target_uri = NULL;

	if (g_path_is_absolute(filename)) {
		GFile *target = g_file_new_for_path(filename);
		target_uri = g_file_get_uri(target);
		g_object_unref(target);
	} else {
		// Assume path is relative to the check file
		GFile *dir = g_file_get_parent(file);
		char *dir_uri = g_file_get_uri(dir);
		target_uri = g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s",
			dir_uri, filename);
		g_free(dir_uri);
		g_object_unref(dir);
	}

	g_free(filename);

	return g_slist_prepend(ud_list, uri_digest_new(target_uri, digest));
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
	gsize len = 0;
	GError *error = NULL;

	if (!g_file_load_contents(file, NULL, &data, &len, NULL, &error)) {
		check_file_error(file, error);
		g_error_free(error);
		return 0;
	}

	GInputStream *is = g_memory_input_stream_new_from_data(data, len, NULL);
	GDataInputStream *dis = g_data_input_stream_new(is);

	char *line = NULL;
	len = 0;
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
		}

		g_free(line);
	}

	if (error) {
		check_file_error(file, error);
		g_error_free(error);
	} else
		check_file_enable_hinted_hash_func(file);

	g_object_unref(dis);
	g_object_unref(is);
	g_free(data);

	return ud_list;
}

void check_file_save(const char * const filename)
{
	g_assert(filename);

	GString *string = g_string_sized_new(1024);

	for (int i = 0; i < HASH_FUNCS_N; i++) {
		if (!hash.funcs[i].enabled)
			continue;

		switch (gui.view) {
			case GUI_VIEW_FILE: {
				const bool hmac_active = gtk_toggle_button_get_active(
					gui.togglebutton_hmac_file);
				const char *digest = gtk_entry_get_text(
					gui.hash_widgets[i].entry_file);
				if (digest && *digest) {
					g_string_append_printf(string,
						(hmac_active && hash.funcs[i].hmac_supported) ?
						"# HMAC-%s\n" : "# %s\n", hash.funcs[i].name);
				} else
					continue;
				char *path = gtk_file_chooser_get_filename(
					GTK_FILE_CHOOSER(gui.filechooserbutton));
				char *basename = g_path_get_basename(path);
				g_free(path);
				g_string_append_printf(string, "%s  %s\n",
				gtk_entry_get_text(gui.hash_widgets[i].entry_file),
					basename);
				g_free(basename);
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
				int prev = -1;
				for (unsigned int row = 0; row < list.rows; row++)
				{
					char *digest = list_get_digest(row, i);
					if (digest && *digest) {
						if (i != prev)
							g_string_append_printf(string, "# %s\n",
								hash.funcs[i].name);
						prev = i;
					} else {
						if (digest)
							g_free(digest);
						prev = i;
						continue;
					}
					char *uri = list_get_uri(row);
					char *basename = g_filename_display_basename(uri);
					g_string_append_printf(string, "%s  %s\n",
						digest, basename);
					g_free(basename);
					g_free(uri);
					g_free(digest);
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
