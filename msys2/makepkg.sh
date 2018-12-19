#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/..

if [[ ! -f configure ]] ; then
	./autogen.sh
fi

if [[ ! -f Makefile ]] ; then
	./configure \
		--disable-appstream \
		--disable-blake2
fi

make -j$(nproc) dist-gzip
cp -avf gtkhash-*.tar.gz msys2/mingw-w64-gtkhash/gtkhash.tar.gz

cd msys2/mingw-w64-gtkhash
makepkg-mingw \
	--noconfirm \
	--noprogressbar \
	--syncdeps \
	--clean \
	--cleanbuild \
	--force \
	--install
