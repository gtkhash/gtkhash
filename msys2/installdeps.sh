#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

pacman -S --noconfirm --needed --noprogressbar \
	mingw-w64-{x86_64,i686}-toolchain \
	mingw-w64-{x86_64,i686}-pkg-config \
	mingw-w64-{x86_64,i686}-libtool \
	mingw-w64-{x86_64,i686}-glib2 \
	mingw-w64-{x86_64,i686}-gtk3 \
	mingw-w64-{x86_64,i686}-librsvg \
	mingw-w64-{x86_64,i686}-gsettings-desktop-schemas \
	mingw-w64-{x86_64,i686}-libgcrypt \
	mingw-w64-{x86_64,i686}-styrene \
	base-devel
