#!/usr/bin/env bash
# All rights waived: https://creativecommons.org/publicdomain/zero/1.0/

set -e

cd "$(dirname "$0")"/styrene

do_styrene() {
	local pkgdir=$(readlink -f ../mingw-w64-gtkhash)

	MSYSTEM=MINGW${1} bash -l -c "exec styrene --pkg-dir \"${pkgdir}\" styrene.cfg"
}

do_styrene 64
do_styrene 32
