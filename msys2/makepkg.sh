#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/..

meson _build \
	--buildtype=release \
	-Db_lto=true \
	-Dappstream=false \
	-Dblake2=false \
	-Dlinux-crypto=false \
	-Dlibcrypto=false \
	-Dmbedtls=false \
	-Dnettle=false \
	-Dzlib=false

pushd _build >/dev/null
	ninja
	meson dist
popd >/dev/null

cp -avf _build/meson-dist/gtkhash-*.tar.xz msys2/mingw-w64-gtkhash/gtkhash.tar.gz

pushd msys2/mingw-w64-gtkhash >/dev/null
	PKGEXT='.pkg.tar.xz' makepkg-mingw \
		--noconfirm \
		--noprogressbar \
		--syncdeps \
		--clean \
		--cleanbuild \
		--force \
		--install
popd >/dev/null
