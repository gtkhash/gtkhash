#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/styrene

do_styrene() {
	local tmpdir=$(mktemp -d -t w${1}-XXXXXX -p /tmp)

	MSYSTEM=MINGW${1} /mingw${1}/bin/styrene \
		--pkg-dir ../mingw-w64-gtkhash \
		--output-dir "${tmpdir}" \
		styrene.cfg

	mv -vf "${tmpdir}"/gtkhash-w${1}-*-installer.exe .
	mv -vf "${tmpdir}"/gtkhash-w${1}-*-standalone.zip .
}

do_styrene 64
do_styrene 32
