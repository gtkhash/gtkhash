project('gtkhash',
    'c',
    version: '1.4',
    meson_version: '>=0.53.0',
    default_options: [
        'warning_level=2',
    ],
)

gnome       = import('gnome')
pkgconfig   = import('pkgconfig')
i18n        = import('i18n')

################################################################################
# Project configuration

config = configuration_data()
data_config = configuration_data()

gettext_package = meson.project_name()
config.set_quoted('GETTEXT_PACKAGE', gettext_package)

config.set10('ENABLE_NLS', true)

build_gtkhash    = get_option('build-gtkhash')
enable_appstream = get_option('appstream')

config.set10('ENABLE_NATIVE_FILE_CHOOSER', get_option('native-file-chooser'))

config.set('HASH_FILE_BUFFER_SIZE', 131072, description: 'File read buffer size (bytes)')
config.set('HASH_FILE_REPORT_INTERVAL', 166, description: 'Progress report interval (milliseconds)')

package_name = 'GtkHash'
config.set_quoted('VERSION',        meson.project_version())
config.set_quoted('PACKAGE',        meson.project_name())
config.set_quoted('PACKAGE_NAME',   package_name)
config.set_quoted('PACKAGE_STRING', package_name + ' ' + meson.project_version())
config.set_quoted('G_LOG_DOMAIN',   package_name)

config.set_quoted('LOCALEDIR',      get_option('prefix') / get_option('localedir'))

data_config.set('VERSION', meson.project_version())
data_config.set('RELEASE_DATE', '2020-07-11')

################################################################################
# Compiler options

cc = meson.get_compiler('c')

assert(get_option('warning_level').to_int() >= 2)

# Todo check which options are used by Meson
compiler_options = [
    '-fno-common',
    '-fvisibility=hidden',
    '-pedantic',
    '-Waggregate-return',
    '-Wbad-function-cast',
    '-Wcast-align',
    '-Werror=implicit-function-declaration',
    '-Wfloat-equal',
    '-Wformat-nonliteral',
    '-Wformat-security',
    '-Wformat-signedness',
    '-Wformat',
    '-Wlogical-op',
    '-Wmissing-declarations',
    '-Wmissing-noreturn',
    '-Wno-parentheses-equality',
    '-Wredundant-decls',
    '-Wshadow',
    '-Wswitch-default',
    '-Wundef',
    '-Wwrite-strings',
]
add_project_arguments(cc.get_supported_arguments(compiler_options), language: 'c')

################################################################################
# Dependencies

no_dep = dependency('', required: false)

glib = dependency('glib-2.0', version: '>=2.48.0')
gio  = dependency('gio-2.0',  version: '>=2.48.0')
gtk  = dependency('gtk+-3.0', version: '>=3.18')

#######################################
# Hash libraries

##### Blake2
blake2 = get_option('blake2') ? dependency('libb2') : no_dep
config.set10('ENABLE_BLAKE2', get_option('blake2'))


##### GCrypt
libgcrypt = get_option('gcrypt') ? dependency('libgcrypt', version: '>=1.6.0') : no_dep
config.set10('ENABLE_GCRYPT', get_option('gcrypt'))
config.set_quoted('HASH_LIB_GCRYPT_MIN_VER', '1.6.0')
detected_gcrypt_functions = []
if cc.has_header_symbol('gcrypt.h', 'GCRY_MD_BLAKE2B_512')
    detected_gcrypt_functions += [ 'BLAKE2b', 'BLAKE2s' ]
endif
if cc.has_header_symbol('gcrypt.h', 'GCRY_MD_SHA3_512')
    detected_gcrypt_functions += [ 'SHA3-224', 'SHA3-256', 'SHA3-384', 'SHA3-512' ]
endif

##### Glib checksums
config.set10('ENABLE_GLIB_CHECKSUMS', get_option('glib-checksums'))

##### Internal MD6
config.set10('ENABLE_INTERNAL_MD6', get_option('internal-md6'))

##### Libcrypto (openssl)
libcrypto = get_option('libcrypto') ? dependency('libcrypto', version: '>=1.1') : no_dep
config.set10('ENABLE_LIBCRYPTO', get_option('libcrypto'))

##### Linux crypto
if get_option('linux-crypto')
    cc.has_header('linux/if_alg.h') or error('Header if_alg.h not found !')
endif
config.set10('ENABLE_LINUX_CRYPTO', get_option('linux-crypto'))

##### MbedTLS
mbedtls = get_option('mbedtls') ? \
    cc.find_library('mbedcrypto', has_headers: 'mbedtls/md.h') : no_dep

config.set10('ENABLE_MBEDTLS', get_option('mbedtls'))

##### Nettle
nettle = get_option('nettle') ? dependency('nettle') : no_dep
config.set10('ENABLE_NETTLE', nettle.found())

##### Zlib
zlib = get_option('zlib') ? dependency('zlib') : no_dep
config.set10('ENABLE_ZLIB', zlib.found())

#######################################
# File manager extensions

build_caja = get_option('build-caja')
if build_caja
    caja = dependency('libcaja-extension')
    caja_extension_dir = caja.get_pkgconfig_variable('extensiondir')
endif

build_nautilus = get_option('build-nautilus')
if build_nautilus
    nautilus = dependency('libnautilus-extension')
    nautilus_extension_dir = nautilus.get_pkgconfig_variable('extensiondir')
    have_nautilus_header = cc.has_header('nautilus/nautilus-extension.h')
endif
config.set10('HAVE_NAUTILUS_EXTENSION_H', build_nautilus and have_nautilus_header)

build_nemo = get_option('build-nemo')
if build_nemo
    nemo = dependency('libnemo-extension')
    nemo_extension_dir = nemo.get_pkgconfig_variable('extensiondir')
endif

build_thunar = get_option('build-thunar')
thunarx = get_option('thunarx')
if build_thunar
    thunar = dependency('thunarx-' + thunarx, version: '>=1.7.0')
    thunar_extension_dir = thunar.get_pkgconfig_variable('extensionsdir')
endif

enable_nautilus_data = build_caja or build_nautilus or build_nemo or build_thunar

if [ 'debug', 'debugoptimized' ].contains(get_option('buildtype'))
    config.set10('G_DISABLE_DEPRECATED', true)
    config.set10('GDK_DISABLE_DEPRECATED', true)
    config.set10('GTK_DISABLE_DEPRECATED', true)
else
    config.set10('G_DISABLE_ASSERT', true)
    config.set10('G_DISABLE_CAST_CHECKS', true)
endif

add_project_arguments('-DHAVE_CONFIG_H', language: 'c')
root_include = include_directories('.')
configure_file(
    output: 'config.h',
    configuration: config,
)

summary({
    'gtkhash'           : '@0@ (@1@)'.format(build_gtkhash, gtk.name()),
    'gtkhash-caja'      : '@0@'.format(build_caja),
    'gtkhash-nautilus'  : '@0@'.format(build_nautilus),
    'gtkhash-nemo'      : '@0@'.format(build_nemo),
    'gtkhash-thunar'    : '@0@ (thunarx-@1@)'.format(build_thunar, thunarx),
    },
    section: 'Interfaces',
)

################################################################################
# List available hash functions

hash_functions = [
    'ADLER32',
    'BLAKE2b', 'BLAKE2s', 'BLAKE2bp', 'BLAKE2sp',
    'CRC32', 'CRC32C',
    'GOST',
    'MD2',
    'MD4',
    'MD5',
    'MD6-224', 'MD6-256', 'MD6-384', 'MD6-512',
    'MDC2',
    'RIPEMD128', 'RIPEMD160', 'RIPEMD256', 'RIPEMD320',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
    'SHA3-224', 'SHA3-256', 'SHA3-384', 'SHA3-512',
    'SM3',
    'TIGER192',
    'WHIRLPOOL',
    'XXH64',
]

gcrypt_funcs = detected_gcrypt_functions + [
    'CRC32',
    'GOST',
    'MD4',
    'MD5',
    'RIPEMD160',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
    'TIGER192',
    'WHIRLPOOL',
]

libcrypto_funcs = [
    'BLAKE2b', 'BLAKE2s',
    'MD2',
    'MD4',
    'MD5',
    'MDC2',
    'RIPEMD160',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
    'SHA3-224', 'SHA3-256', 'SHA3-384', 'SHA3-512',
    'SM3',
    'WHIRLPOOL',
]

linux_crypto_funcs = [
    'BLAKE2b', 'BLAKE2s',
    'CRC32C',
    'MD4',
    'MD5',
    'RIPEMD128', 'RIPEMD160', 'RIPEMD256', 'RIPEMD320',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
    'SHA3-224', 'SHA3-256', 'SHA3-384', 'SHA3-512',
    'TIGER192',
    'WHIRLPOOL',
    'XXH64',
]

mbedtls_funcs = [
    'MD2',
    'MD4',
    'MD5',
    'RIPEMD160',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
]

nettle_funcs = [
    'GOST',
    'MD2',
    'MD4',
    'MD5',
    'RIPEMD160',
    'SHA1',
    'SHA224', 'SHA256', 'SHA384', 'SHA512',
    'SHA3-224', 'SHA3-256', 'SHA3-384', 'SHA3-512',
]

# Matches preferred order from src/hash/hash-lib.c
hash_libs = {
    'blake2':           [ 'BLAKE2b', 'BLAKE2s', 'BLAKE2bp', 'BLAKE2sp', ],
    'gcrypt':           gcrypt_funcs,
    'glib_checksums':   [ 'MD5', 'SHA1', 'SHA256', 'SHA384', 'SHA512', ],
    'internal_md6':     [ 'MD6-224', 'MD6-256', 'MD6-384', 'MD6-512', ],
    'libcrypto':        libcrypto_funcs,
    'linux_crypto':     linux_crypto_funcs,
    'mbedtls':          mbedtls_funcs,
    'nettle':           nettle_funcs,
    'zlib':             [ 'ADLER32', 'CRC32', ],
}

functions_provided = { 'Hash Function': 'Preferred Implementation' }
foreach function : hash_functions
    provide = []
    foreach lib, functions : hash_libs
        lib_enabled = config.get('ENABLE_' + lib.to_upper()) == 1
        if lib_enabled and functions.contains(function)
            provide += lib
        endif
    endforeach
    functions_provided += { function: ', '.join(provide) }
endforeach

summary(functions_provided, section: 'Libraries')

if get_option('linux-crypto')
    summary({ 'linux-crypto': 'support depends on kernel module availability - check /proc/crypto'})
endif

###############################################################################

subdir('po')
subdir('data')
subdir('src')
