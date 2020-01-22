[![Build Status](https://travis-ci.com/tristanheaven/gtkhash.svg?branch=master)](https://travis-ci.com/tristanheaven/gtkhash)
[![Pipeline Status](https://gitlab.com/tristanheaven/gtkhash/badges/master/pipeline.svg)](https://gitlab.com/tristanheaven/gtkhash/commits/master)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/1hm3cs5f0islas0w/branch/master?svg=true)](https://ci.appveyor.com/project/tristanheaven/gtkhash/branch/master)
[![Test Coverage](https://codecov.io/gh/tristanheaven/gtkhash/branch/master/graph/badge.svg)](https://codecov.io/gh/tristanheaven/gtkhash)
[![Translations](https://hosted.weblate.org/widgets/gtkhash/-/svg-badge.svg)](https://hosted.weblate.org/engage/gtkhash/)
[![Snap Status](https://build.snapcraft.io/badge/tristanheaven/gtkhash.svg)](https://build.snapcraft.io/user/tristanheaven/gtkhash)

GtkHash
=======
![Screenshot](screenshots/readme.png)

GtkHash is a desktop utility for computing message digests or checksums.
Most well-known hash functions are supported, including MD5, SHA1,
SHA2 (SHA256/SHA512), SHA3 and BLAKE2.

It's designed to be an easy to use, graphical alternative to command-line
tools such as md5sum.

Some interesting features:
* Support for verifying checksum files from sfv, sha256sum, etc.
* Keyed hashing (HMAC)
* Parallel/multi-threaded hash calculation
* Remote file access using GIO/GVfs
* File manager integration
* Small and fast, written in C

GtkHash is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

Required Dependencies
-------------
* GTK 3.18
* GLib 2.48

Optional Dependencies
---------------------
* Libgcrypt (default)
* libb2 (default)
* Nettle
* OpenSSL
* mbed TLS
* mhash
* Linux Kernel Crypto (AF_ALG)

GtkHash attempts to select the fastest available hash function
implementations at startup. See `./configure --help` for a full list of
options. Build dependencies are not detected implicitly or "automagically".

File Manager Extensions
-----------------------
* Caja (MATE)
* Nautilus (GNOME)
* Nemo (Cinnamon)
* Peony (UKUI)
* Thunar (Xfce)

Translations
------------
![Translation Details](https://hosted.weblate.org/widgets/gtkhash/-/multi-auto.svg)

If you would like to contribute a translation, the easiest way is by using
Weblate:

https://hosted.weblate.org/engage/gtkhash/

Alternatively, updated .po files can be submitted as a Pull Request or Issue on
GitHub:

https://github.com/tristanheaven/gtkhash
