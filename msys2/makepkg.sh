#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/..

./autogen.sh
./configure \
	--disable-appstream \
	--disable-blake2

make -j1 dist-gzip
cp -avf gtkhash-*.tar.gz msys2/mingw-w64-gtkhash/gtkhash.tar.gz

cd msys2/mingw-w64-gtkhash
PKGEXT='.pkg.tar.xz' makepkg-mingw \
	--noconfirm \
	--noprogressbar \
	--syncdeps \
	--clean \
	--cleanbuild \
	--force \
	--install
