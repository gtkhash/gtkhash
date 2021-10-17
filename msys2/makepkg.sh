#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/..
git archive --prefix gtkhash/ -o msys2/mingw-w64-gtkhash/gtkhash.tar.gz HEAD

cd msys2/mingw-w64-gtkhash
MINGW_ARCH="mingw64 mingw32" PKGEXT='.pkg.tar.xz' exec makepkg-mingw \
	--noconfirm \
	--noprogressbar \
	--syncdeps \
	--clean \
	--cleanbuild \
	--force
