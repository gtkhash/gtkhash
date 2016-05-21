[![Build Status](https://travis-ci.org/tristanheaven/gtkhash.svg?branch=master)](https://travis-ci.org/tristanheaven/gtkhash)
[![Translation Status](https://hosted.weblate.org/widgets/gtkhash/-/svg-badge.svg)](https://hosted.weblate.org/projects/gtkhash/)

GtkHash
=======

GtkHash is a desktop utility for computing message digests or checksums.
Most well-known hash functions are supported, including: MD5, SHA1, SHA2 and
SHA3.

It's designed to be an easy to use, graphical alternative to command-line
programs such as md5sum.

Some interesting features:
* Keyed hashing (HMAC)
* Opens files from remote systems (using GIO/GVfs)
* Parallel hash calculation on multi-core CPUs
* Small and fast, written in C.

GtkHash is primarily developed for Linux or BSD desktops. Support for other
operating systems is untested, but patches are welcome.

Required Dependencies:
-------------
* GTK+ 3 or 2
* Glib

Optional Dependencies
---------------------
These libraries can be used to provide extra hash functions:
* libcrypto (OpenSSL)
* Libgcrypt
* Linux crypto (CONFIG_CRYPTO_USER_API_HASH)
* mbed TLS
* mhash
* Nettle
* NSS
* PolarSSL
* zlib

See `./configure --help` for options.

File Manager Extensions
-----------------------
GtkHash also includes extensions for the following file managers:
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
