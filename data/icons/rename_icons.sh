#!/bin/sh

mv_dir="$1"
file_in="$2"
file_out="$3"

exec mv -- "$MESON_INSTALL_DESTDIR_PREFIX/$mv_dir/$file_in" \
   "$MESON_INSTALL_DESTDIR_PREFIX/$mv_dir/$file_out"
