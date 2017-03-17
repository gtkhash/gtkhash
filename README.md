[![Build Status](https://travis-ci.org/tristanheaven/gtkhash.svg?branch=master)](https://travis-ci.org/tristanheaven/gtkhash)
[![Translation Status](https://hosted.weblate.org/widgets/gtkhash/-/svg-badge.svg)](https://hosted.weblate.org/projects/gtkhash/)

GtkHash
=======

GtkHash is a desktop utility for computing message digests or checksums.
Most well-known hash functions are supported, including MD5, SHA1,
SHA2 (SHA256/SHA512), SHA3 and BLAKE2.

It's designed to be an easy to use, graphical alternative to command-line
tools such as md5sum.

Some interesting features:
* Support for verifying checksum files from sfv, sha256sum, etc.
* Keyed hashing (HMAC)
* Parallel/threaded hash calculation
* Remote file access using GIO/GVfs
* File manager integration
* Small and fast, written in C

Required Dependencies
-------------
* GTK+ 3 or 2
* GLib

Optional Dependencies
---------------------
* Libgcrypt
* Nettle
* libb2
* OpenSSL
* mbed TLS
* mhash
* Linux Kernel Crypto (`CONFIG_CRYPTO_USER_API_HASH=y`)

GtkHash will attempt to select the best available implementation of each
hash function based on its performance, but this can differ wildly between
library versions and CPU architectures.

See `./configure --help` for a full list of options. Dependencies are not
detected implicitly or "automagically".

To enable SHA3, use `--enable-gcrypt` or `--enable-nettle`

To enable BLAKE2, use `--enable-blake2` or `--enable-libcrypto`

File Manager Extensions
-----------------------
* Caja (MATE)
* Nautilus (GNOME)
* Nemo (Cinnamon)
* Thunar (Xfce)

Licence
-------
GtkHash is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

Translations
------------
If you would like to help with translations, the easiest way is by using
Weblate:

https://hosted.weblate.org/projects/gtkhash/

Alternatively, updated .po files can be submitted as a Pull Request or Issue on
GitHub:

https://github.com/tristanheaven/gtkhash
