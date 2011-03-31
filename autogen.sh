#!/bin/sh

set -e
set -x

[ -f autogen.sh ]

[ ! -d m4 ] && mkdir m4

glib-gettextize --copy --force
intltoolize --automake --copy --force
autoreconf --force --install --warnings=all
