if build_gtkhash
    gtkhash_test_sources = []
    foreach s : gtkhash_common_sources
        gtkhash_test_sources += '../src/' + s
    endforeach

    gtkhash_test = executable(
        'gtkhash-test',
        gtkhash_test_sources,
        gtkhash_resources,
        'test.c',
        include_directories: [
            root_include,
            include_directories('../src'),
        ],
        dependencies: [
            glib,
            gtk,
        ],
        link_with: [
            libhash,
            libutil,
        ],
    )

    test_files = []

    configure_file(
        input: 'test.md5sum.in',
        output: 'test.md5sum',
        configuration: {
            'builddir': meson.current_build_dir(),
        },
    )

    bytes_tests_values = [
        '0',
        '1',
        '10M',
        '5G',
    ]
    foreach bytes_test : bytes_tests_values
        filename = '@0@.bytes'.format(bytes_test)
        test_files += custom_target(
            filename,
            output: filename,
            command: [
                'truncate',
                '-s',
                '@0@'.format(bytes_test),
                '@OUTPUT@',
            ],
        )
    endforeach

    foreach lib, functions : hash_libs
        test(
            '/hash/lib/@0@'.format(lib),
            gtkhash_test,
            depends: test_files,
            env: ['GTKHASH_TEST_LIB=@0@'.format(lib)],
            protocol: 'tap',
            suite: 'tests',
        )
    endforeach

    foreach prefix : ['/hash/lib/any', '/opt', '/digest-format']
        test(
            prefix,
            gtkhash_test,
            depends: test_files,
            args: ['-r', prefix],
            protocol: 'tap',
            suite: 'tests',
        )
    endforeach
endif
