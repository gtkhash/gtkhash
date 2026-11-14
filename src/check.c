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
#include <errno.h>
#include <gtk/gtk.h>

#include "check.h"
#include "main.h"
#include "hash.h"
#include "gui.h"
#include "list.h"
#include "uri-digest.h"
#include "hash/hash-func.h"
#include "hash/hash-file.h"
#include "util/util.h"

#define CHECK_LINE_MIN 10
#define CHECK_LINE_MAX 8192

struct {
	GRegex *regex[CHECK_FORMATS_N];
	const char * const pattern[CHECK_FORMATS_N];
} check_priv = {
	.regex = {
		NULL, NULL, NULL
	},
	.pattern = {
		[CHECK_FORMAT_BSD] =
			"^[ \t]*"
			"(?<ESCAPED>\\\\)?"                             // capture ESCAPED (indicates FILENAME is escaped)
			"(?<FUNCTION>[[:alpha:]]{2}[[:alnum:]-]{1,14})" // capture FUNCTION (e.g. "MD5", "BLAKE2b")
			" *\\("
			"(?<FILENAME>.+)"                               // capture FILENAME
			"\\)[ \t]*=[ \t]*"
			"(?<DIGEST>[[:xdigit:]]{8,})"                   // capture DIGEST
			"\\r?$",

		[CHECK_FORMAT_GNU] =
			"^[ \t]*"
			"(?<ESCAPED>\\\\)?"           // capture ESCAPED (indicates FILENAME is escaped)
			"(?<DIGEST>[[:xdigit:]]{8,})" // capture DIGEST (8+ hex chars)
			"[ \t][ *]?"                  // '*' indicates binary mode (md5sum -b)
			"(?<FILENAME>.+)"             // capture FILENAME
			"\\r?$",

		[CHECK_FORMAT_SFV] =
			"^(?<FILENAME>.+?)"           // capture FILENAME
			"[ \t]"
			"(?<DIGEST>[[:xdigit:]]{8})"  // capture DIGEST (8 hex chars)
			"[[:space:]]*$",
	},
};

static enum check_format_e check_regex_match(const char * const line, GMatchInfo **info)
{
	for (enum check_format_e i = 0; i < CHECK_FORMATS_N; i++) {
		if (!check_priv.regex[i])
			continue;
		if (g_regex_match(check_priv.regex[i], line, 0, info))
			return i;
		g_match_info_free(*info);
		*info = NULL;
	}

	return CHECK_FORMAT_INVALID;
}

static enum check_format_e check_line_classify(const char * const line, const gsize len,
	GMatchInfo **info)
{
	if (!line)
		return CHECK_FORMAT_INVALID;

	if (len == 1 && line[0] == '\r')
		return CHECK_FORMAT_COMMENT;

	switch (line[0]) {
		case '\0':
		case '#':
		case ';':
			return CHECK_FORMAT_COMMENT;
		default:
			break;
	}

	if ((len < CHECK_LINE_MIN) || (len > CHECK_LINE_MAX))
		return CHECK_FORMAT_INVALID;

	if (!g_utf8_validate_len(line, len, NULL))
		return CHECK_FORMAT_INVALID;

	return check_regex_match(line, info);
}

// Unescape filename in-place
static bool check_unescape_filename(char *filename)
{
	size_t r = 0, w = 0;

	for (; filename[r]; r++) {
		if (filename[r] != '\\') {
			filename[w] = filename[r];
			w++;
			continue;
		}

		switch (filename[r + 1]) {
			case '\\':
				filename[w] = '\\';
				break;
			case 'n':
				filename[w] = '\n';
				break;
			default:
				// Invalid or unknown escaping
				return false;
		}

		r++;
		w++;
	}

	filename[w] = '\0';

	return true;
}

static enum check_format_e check_line_parse(const char * const line, const gsize len,
	enum hash_func_e *id, char **filename, char **digest)
{
	*id = HASH_FUNC_INVALID;
	*filename = NULL;
	*digest = NULL;

	GMatchInfo *info = NULL;
	const enum check_format_e format = check_line_classify(line, len, &info);

	if (CHECK_FORMAT_IS_VALID(format)) {
		char *escaped = g_match_info_fetch_named(info, "ESCAPED");

		char *function = g_match_info_fetch_named(info, "FUNCTION");
		if (function) {
			*id = gtkhash_hash_func_get_id_from_name(function);
			g_free(function);
		}

		*filename = g_match_info_fetch_named(info, "FILENAME");
		*digest = g_match_info_fetch_named(info, "DIGEST");

		g_match_info_free(info);

		if (escaped && *escaped && !check_unescape_filename(*filename)) {
			*id = HASH_FUNC_INVALID;
			g_clear_pointer(filename, g_free);
			g_clear_pointer(digest, g_free);
			g_free(escaped);
			return CHECK_FORMAT_INVALID;
		}

		g_free(escaped);

		// Ignore stdin filenames (e.g. `echo | md5sum`)
		if ((g_strcmp0(*filename, "-") == 0) ||
			(g_strcmp0(*filename, "stdin") == 0) ||
			(g_strcmp0(*filename, "(stdin)") == 0))
		{
			*id = HASH_FUNC_INVALID;
			g_clear_pointer(filename, g_free);
			g_clear_pointer(digest, g_free);
			return CHECK_FORMAT_INVALID;
		}
	}

	return format;
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

static GSList *check_file_add_uri(GSList *ud_list, GFile *file, const char * const filename,
	const char * const digest)
{
	g_assert(file);
	g_assert(filename && *filename);
	g_assert(digest && *digest);

	GFile *target = NULL;

	if (g_path_is_absolute(filename)) {
		// Only allow a single leading "/" as "//" would be interpreted as a URI network-path
		const char *abs_filename = filename;
		while (abs_filename[1] == '/')
			abs_filename++;

		if (g_file_is_native(file)) {
			// Absolute path on local system
			target = g_file_new_for_path(abs_filename);
		} else {
			// Absolute path on remote system
			char *uri = g_file_get_uri(file);
			char *path = g_uri_escape_string(abs_filename, G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, true);
			GError *error = NULL;
			char *target_uri = g_uri_resolve_relative(uri, path, G_URI_FLAGS_NONE, &error);
			if (target_uri) {
				target = g_file_new_for_uri(target_uri);
			} else {
				char *pname = g_file_get_parse_name(file);
				g_message("%s: %s", pname, error->message);
				g_free(pname);
			}
			g_clear_error(&error);
			g_free(target_uri);
			g_free(path);
			g_free(uri);
		}
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

GSList *check_file_load(GSList *ud_list, GFile *file)
{
	g_assert(file);

	GError *error = NULL;
	GFileInfo *info = gtkhash_hash_file_query_info(file, NULL, &error);

	if (!info) {
		gtkhash_file_error(file, error);
		g_clear_error(&error);
		return ud_list;
	}

	const GFileType type = g_file_info_get_file_type(info);
	g_object_unref(info);

	if (type != G_FILE_TYPE_REGULAR) {
		char *pname = g_file_get_parse_name(file);
		g_message("%s: %s", pname, g_strerror(ENOTSUP));
		g_free(pname);
		return ud_list;
	}

	GFileInputStream *fis = g_file_read(file, NULL, &error);

	if (!fis) {
		gtkhash_file_error(file, error);
		g_clear_error(&error);
		return ud_list;
	}

	GDataInputStream *dis = g_data_input_stream_new((GInputStream *)fis);

	char *line = NULL;
	gsize len = 0;
	char *pname = g_file_get_parse_name(file);
	unsigned long valid_n = 0, invalid_n = 0;

	while ((line = g_data_input_stream_read_line(dis, &len, NULL, &error))) {
		enum hash_func_e id = HASH_FUNC_INVALID;
		char *filename = NULL;
		char *digest = NULL;
		const enum check_format_e format = check_line_parse(line, len, &id, &filename, &digest);

		if (format == CHECK_FORMAT_COMMENT) {
			g_free(line);
			continue;
		}

		if (CHECK_FORMAT_IS_VALID(format) && filename && *filename && digest && *digest) {
			if (HASH_FUNC_IS_VALID(id))
				gui_enable_hash_func(id);
			valid_n++;
			ud_list = check_file_add_uri(ud_list, file, filename, digest);
		} else
			invalid_n++;

		g_free(filename);
		g_free(digest);
		g_free(line);
	}

	if (error) {
		g_message("%s: %s", pname, error->message);
		g_clear_error(&error);
	} else
		check_file_enable_hinted_hash_func(file);

	if (invalid_n) {
		g_message(g_dngettext(GETTEXT_PACKAGE,
			"%s: Ignored %lu invalid line",
			"%s: Ignored %lu invalid lines", invalid_n),
			pname, invalid_n);
	}

	if (!valid_n)
		g_message(_("%s: No valid lines found"), pname);

	g_free(pname);
	g_object_unref(dis);
	g_object_unref(fis);

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

				const bool hmac_enabled = gtk_toggle_button_get_active(gui.togglebutton_hmac_file);

				g_string_append_printf(string, "# %s\n",
					(hmac_enabled && hash.funcs[i].hmac_supported) ?
					hash.funcs[i].hmac_name : hash.funcs[i].name);

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
			case GUI_VIEW_FILE_LIST: {
				enum hash_func_e prev = HASH_FUNC_INVALID;
				const int rows = list_get_rows();

				for (int row = 0; row < rows; row++) {
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
	for (enum check_format_e i = 0; i < CHECK_FORMATS_N; i++) {
		GError *error = NULL;
		check_priv.regex[i] = g_regex_new(check_priv.pattern[i], 0, 0, &error);

		if (error) {
			g_warning("g_regex_new(): %s", error->message);
			g_clear_error(&error);
		}
	}
}

void check_deinit(void)
{
	for (enum check_format_e i = 0; i < CHECK_FORMATS_N; i++) {
		if (check_priv.regex[i]) {
			g_regex_unref(check_priv.regex[i]);
			check_priv.regex[i] = NULL;
		}
	}
}
