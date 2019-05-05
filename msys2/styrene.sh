#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/styrene

do_styrene() {
	local tmpdir=$(mktemp -d -t w${1}-XXXXXX -p /tmp)
	local pkgdir=$(readlink -f ../mingw-w64-gtkhash)

	MSYSTEM=MINGW${1} bash -l -x -c \
		"exec styrene --pkg-dir \"${pkgdir}\" --output-dir \"${tmpdir}\" styrene.cfg"

	mv -vf "${tmpdir}"/gtkhash-w${1}-*-installer.exe .
	mv -vf "${tmpdir}"/gtkhash-w${1}-*-standalone.zip .
}

do_styrene 64
do_styrene 32
