subdir('hash')
subdir('util')

if enable_extensions
    subdir('extensions')
endif

if build_gtkhash
    subdir('ui')

    gtkhash_common_sources = [
        'callbacks.c',
        'check.c',
        'gui.c',
        'hash.c',
        'list.c',
        'opts.c',
        'uri-digest.c',
    ]

    gtkhash = executable(
        'gtkhash',
        gtkhash_common_sources,
        gtkhash_resources,
        'main.c',
        'prefs.c',
        include_directories: root_include,
        link_with: [
            libhash,
            libutil,
        ],
        dependencies: gtk,
        install: true,
    )
endif
