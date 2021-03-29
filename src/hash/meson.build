libhash_sources = [
    'digest-format.c',
    'digest.c',
    'hash-file.c',
    'hash-func.c',
    'hash-lib.c',
    'hash-string.c',
    'hmac.c',
]

if get_option('blake2')
    libhash_sources += 'hash-lib-blake2.c'
endif
if get_option('gcrypt')
    libhash_sources += 'hash-lib-gcrypt.c'
endif
if get_option('glib-checksums')
    libhash_sources += 'hash-lib-glib.c'
endif
if get_option('libcrypto')
    libhash_sources += 'hash-lib-crypto.c'
endif
if get_option('linux-crypto')
    libhash_sources += 'hash-lib-linux.c'
endif
if get_option('mbedtls')
    libhash_sources += 'hash-lib-mbedtls.c'
endif
if get_option('internal-md6')
    libhash_sources += 'hash-lib-md6.c'
endif
if get_option('nettle')
    libhash_sources += 'hash-lib-nettle.c'
endif
if get_option('zlib')
    libhash_sources += 'hash-lib-zlib.c'
endif

libhash = static_library('hash',
    libhash_sources,

    include_directories: root_include,
    c_args: [
        '-DIN_HASH_LIB',
    ],
    dependencies: [
        blake2,
        gio,
        glib,
        libcrypto,
        libgcrypt,
        mbedtls,
        nettle,
        zlib,
    ],

)
# hash_la_LDFLAGS = -avoid-version -module
