/*
 *   GtkHash - Deepin File Manager integration
 *
 *   Adds (or removes) a right-click context-menu entry to the Deepin
 *   File Manager (dde-file-manager) using its "Custom Menu Script"
 *   mechanism (documented in dde-file-manager/docs/extension/).
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

#include <stdbool.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "dfm-integration.h"

// launcher script + context menu config are installed under the user's
// home directory, so no root privileges are required
#define DFM_SCRIPT_DIR ".local/share/gtkhash-filemanager"
#define DFM_SCRIPT_NAME "hash-files.sh"
#define DFM_MENU_DIR ".local/share/deepin/dde-file-manager/context-menus"
#define DFM_MENU_NAME "gtkhash.conf"

// launcher script executed by the context menu entry; falls back to a
// portable AppImage in common locations if gtkhash is not installed
static const char dfm_script_contents[] =
	"#!/bin/bash\n"
	"# GtkHash file-manager launcher (handles paths with spaces correctly)\n"
	"# Used by the dde-file-manager custom context menu.\n"
	"\n"
	"# Optional: set the full path to your AppImage if it is not found\n"
	"# automatically (e.g. after moving it or renaming it arbitrarily).\n"
	"GTKHASH_APPIMAGE=\"\"\n"
	"\n"
	"launch() { # $1 = AppImage path, rest = file args\n"
	"    echo \"$1\" > \"$(dirname \"$0\")/appimage-path\" 2>/dev/null\n"
	"    exec \"$1\" \"${@:2}\"\n"
	"}\n"
	"\n"
	"# 1) system installation\n"
	"if command -v gtkhash >/dev/null 2>&1; then\n"
	"    exec /usr/bin/gtkhash \"$@\"\n"
	"fi\n"
	"\n"
	"# 2) manually configured AppImage path\n"
	"if [ -n \"$GTKHASH_APPIMAGE\" ] && [ -x \"$GTKHASH_APPIMAGE\" ]; then\n"
	"    launch \"$GTKHASH_APPIMAGE\" \"$@\"\n"
	"fi\n"
	"\n"
	"# 3) recorded AppImage path (written by register-appimage.sh, or by\n"
	"#    launch() above once an AppImage has been found)\n"
	"record=\"$(dirname \"$0\")/appimage-path\"\n"
	"if [ -f \"$record\" ]; then\n"
	"    registered=$(cat \"$record\" 2>/dev/null)\n"
	"    if [ -n \"$registered\" ] && [ -x \"$registered\" ]; then\n"
	"        launch \"$registered\" \"$@\"\n"
	"    fi\n"
	"fi\n"
	"\n"
	"# 4) common locations: any GtkHash*.AppImage (any case)\n"
	"for d in \"$(dirname \"$0\")\" \"$HOME/Downloads\" \"$HOME/下载\" \"$HOME/Desktop\" \"$HOME/桌面\" \"$HOME/Applications\" \"$HOME/应用\" \"$HOME/软件\" \"$HOME/opt\" \"$HOME/.local/bin\"; do\n"
	"    [ -d \"$d\" ] || continue\n"
	"    for f in \"$d\"/GtkHash*.AppImage; do\n"
	"        if [ -x \"$f\" ]; then\n"
	"            launch \"$f\" \"$@\"\n"
	"        fi\n"
	"    done\n"
	"done\n"
	"\n"
	"# 5) bounded search under $HOME (skips hidden dirs/Trash/node_modules)\n"
	"while IFS= read -r f; do\n"
	"    if [ -x \"$f\" ]; then\n"
	"        launch \"$f\" \"$@\"\n"
	"    fi\n"
	"done < <(find \"$HOME\" -maxdepth 5 \\\n"
	"    \\( -name '.*' -o -name 'Trash' -o -name 'node_modules' \\) -prune \\\n"
	"    -o -type f -iname 'GtkHash*.AppImage' -print 2>/dev/null | head -n 50)\n"
	"\n"
	"notify-send \"GtkHash\" \"未找到 GtkHash 或 GtkHash-x86_64.AppImage，请先安装。\" 2>/dev/null\n"
	"exit 1\n";

static char *get_script_path(void)
{
	return g_build_filename(g_get_home_dir(), DFM_SCRIPT_DIR, DFM_SCRIPT_NAME, NULL);
}

static char *get_menu_path(void)
{
	return g_build_filename(g_get_home_dir(), DFM_MENU_DIR, DFM_MENU_NAME, NULL);
}

bool dfm_integration_is_supported(void)
{
	return g_find_program_in_path("dde-file-manager") != NULL;
}

static bool install_files(void)
{
	bool ok = true;
	char *script_path = get_script_path();
	char *menu_path = get_menu_path();
	char *script_dir = g_path_get_dirname(script_path);
	char *menu_dir = g_path_get_dirname(menu_path);

	if (g_mkdir_with_parents(script_dir, 0755) != 0) {
		g_warning("Failed to create directory \"%s\"", script_dir);
		ok = false;
	}

	if (ok && g_mkdir_with_parents(menu_dir, 0755) != 0) {
		g_warning("Failed to create directory \"%s\"", menu_dir);
		ok = false;
	}

	// If GtkHash is running from a portable AppImage, copy it next to the
	// launcher script under a canonical name. The launcher's first fallback
	// location is its own directory ("$(dirname "$0")"), so the integration
	// keeps working even if the user later moves or renames the original
	// AppImage file.
	const char *appimage = g_getenv("APPIMAGE");
	if (ok && appimage != NULL && g_file_test(appimage, G_FILE_TEST_IS_EXECUTABLE)) {
		char *dest = g_build_filename(script_dir, "GtkHash-x86_64.AppImage", NULL);
		if (!g_file_copy(g_file_new_for_path(appimage),
				 g_file_new_for_path(dest),
				 G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL)) {
			g_warning("Failed to copy AppImage to \"%s\"", dest);
		} else {
			g_chmod(dest, 0755);
		}
		g_free(dest);
	}

	// menu config references the launcher script by absolute path
	char *menu_contents = g_strdup_printf(
		"[Menu Entry]\n"
		"Version=1.0\n"
		"Comment=Calculate file hashes with GtkHash\n"
		"Comment[zh_CN]=使用 GtkHash 校验文件哈希\n"
		"\n"
		"Actions=HashWithGtkHash:SendToGtkHash\n"
		"\n"
		"[Menu Action HashWithGtkHash]\n"
		"Name=Check Hash (GtkHash)\n"
		"Name[zh_CN]=校验哈希 (GtkHash)\n"
		"X-DFM-MenuTypes=SingleFile:MultiFiles:SingleDir:MultiDirs\n"
		"X-DFM-SupportSchemes=file\n"
		"PosNum=3\n"
		"Separator=Top\n"
		"Exec=%s %%F\n"
		"\n"
		"[Menu Action SendToGtkHash]\n"
		"Name=GtkHash\n"
		"Name[zh_CN]=GtkHash\n"
		"X-DFM-MenuTypes=SingleFile:MultiFiles:SingleDir:MultiDirs\n"
		"X-DFM-SupportSchemes=file\n"
		"X-DFM-ParentMenuPath=send-to\n"
		"PosNum=1\n"
		"Exec=%s %%F\n",
		script_path, script_path);

	if (ok && !g_file_set_contents(script_path, dfm_script_contents,
		(gssize)sizeof(dfm_script_contents) - 1, NULL)) {
		g_warning("Failed to write \"%s\"", script_path);
		ok = false;
	}

	if (ok && g_chmod(script_path, 0755) != 0) {
		g_warning("Failed to chmod \"%s\"", script_path);
		ok = false;
	}

	if (ok && !g_file_set_contents(menu_path, menu_contents, -1, NULL)) {
		g_warning("Failed to write \"%s\"", menu_path);
		ok = false;
	}

	g_free(menu_contents);
	g_free(script_dir);
	g_free(menu_dir);
	g_free(script_path);
	g_free(menu_path);

	return ok;
}

static bool remove_files(void)
{
	char *script_path = get_script_path();
	char *menu_path = get_menu_path();
	char *script_dir = g_path_get_dirname(script_path);
	char *menu_dir = g_path_get_dirname(menu_path);

	g_remove(script_path);
	g_remove(menu_path);

	// also remove the AppImage copy installed by install_files()
	char *appimage_copy = g_build_filename(script_dir, "GtkHash-x86_64.AppImage", NULL);
	g_remove(appimage_copy);
	g_free(appimage_copy);

	// best effort: remove now-empty parent directories
	g_rmdir(script_dir);
	g_rmdir(menu_dir);

	g_free(script_dir);
	g_free(menu_dir);
	g_free(script_path);
	g_free(menu_path);

	return true;
}

bool dfm_integration_apply(bool enabled)
{
	if (!dfm_integration_is_supported())
		return false;

	if (enabled)
		return install_files();

	return remove_files();
}
