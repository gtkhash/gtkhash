subdir('hash')
subdir('util')

if enable_nautilus_data
    subdir('nautilus')
endif

if build_gtkhash

    gtkhash_common_sources = [
        'callbacks.c',
        'check.c',
        'gui.c',
        'hash.c',
        'list.c',
        'opts.c',
        'uri-digest.c',
    ]
    gtkhash_common_sources += gnome.compile_resources('resources',
        gtkhash_gtk3_gresource,
        c_name: 'resources',
        source_dir: meson.current_source_dir() / '..' / 'data',
        extra_args: [
            '--internal',
            '--manual-register',
        ],
    )

    gtkhash = executable('gtkhash',
        gtkhash_common_sources,
        'main.c',
        'prefs.c',

        include_directories: root_include,
        dependencies: [
            glib,
            gtk3,
        ],
        link_with: [
            libhash,
            libutil,
        ],
        install: true,
    )

    # Test executable.

    gtkhash_test = executable('gtkhash-test',
        gtkhash_common_sources,
        'test.c',

        include_directories: root_include,
        dependencies: [
            glib,
            gtk3,
        ],
        link_with: [
            libhash,
            libutil,
        ],
    )

    test_files = []

    configure_file(
        input : 'test.md5sum.in',
        output: 'test.md5sum',
        configuration: { 'builddir': meson.current_build_dir(), },
    )

    bytes_tests_values = [ '0', '1', '10M', '5G', ]
    foreach bytes_test : bytes_tests_values
        filename = '@0@.bytes'.format(bytes_test)
        test_files += custom_target(filename,
            output: filename,
            command: [ 'truncate', '-s', '@0@'.format(bytes_test), '@OUTPUT@', ],
        )
    endforeach

    foreach lib, functions : hash_libs
        lib_enabled = config.get('ENABLE_' + lib.to_upper()) == 1

        test('gtkhash-test-@0@'.format(lib),
            gtkhash_test,
            depends: test_files,
            env: [
                'GTKHASH_TEST_LIB=@0@'.format(lib)
            ],
            protocol: 'tap',
        )
    endforeach

    test('gtkhash-test',
        gtkhash_test,
        depends: test_files,
        protocol: 'tap',
    )
    # LOG_COMPILER = xvfb-run -s "-screen 0 800x600x24" $(srcdir)/gtkhash-test.sh

endif
