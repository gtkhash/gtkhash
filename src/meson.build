subdir('hash')
subdir('util')

if enable_nautilus_data
    subdir('nautilus')
endif

if build_gtkhash
    gtkhash_common_sources = [
        'callbacks.c',
        'check.c',
        'dfm-integration.c',
        'gui.c',
        'hash.c',
        'list.c',
        'opts.c',
        'uri-digest.c',
    ]
    gtkhash_resources = gnome.compile_resources(
        'resources',
        gtkhash_gtk3_gresource,
        c_name: 'resources',
        source_dir: meson.current_source_dir() / '..' / 'data',
        extra_args: [
            '--internal',
            '--manual-register',
        ],
    )

    gtkhash = executable(
        'gtkhash',
        gtkhash_common_sources,
        gtkhash_resources,
        'main.c',
        'prefs.c',
        include_directories: root_include,
        dependencies: [
            glib,
            gtk,
        ],
        link_with: [
            libhash,
            libutil,
        ],
        install: true,
    )

endif
