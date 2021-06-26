#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/..

meson _build \
	-Dblake2=false \
	-Dgcrypt=false \
	-Dappstream=false
meson dist -C _build --formats gztar --no-tests

cp -avf _build/meson-dist/gtkhash-*.tar.gz msys2/mingw-w64-gtkhash/gtkhash.tar.gz

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
