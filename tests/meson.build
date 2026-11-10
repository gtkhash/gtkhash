if build_gtkhash
    gtkhash_test = executable(
        'gtkhash-test',
        gtkhash_common_sources,
        gtkhash_resources,
        'test.c',
        'test-check.c',
        include_directories: [
            root_include,
            include_directories('../src'),
        ],
        link_with: [
            libhash,
            libutil,
        ],
        dependencies: gtk,
    )

    configure_file(
        input: 'lsan.supp',
        output: 'lsan.supp',
        copy: true,
    )

    configure_file(
        input: 'test.md5sum.in',
        output: 'test.md5sum',
        configuration: {
            'builddir': meson.current_build_dir(),
        },
    )

    test_files = []

    foreach bytes : ['0', '1', '10M', '5G']
        filename = bytes + '.bytes'
        test_files += custom_target(
            filename,
            output: filename,
            command: [
                'truncate',
                '-s',
                bytes,
                '@OUTPUT@',
            ],
        )
    endforeach

    test_env = [
        'LSAN_OPTIONS=suppressions=lsan.supp',
    ]

    test_kwargs = {
        'depends': test_files,
        'env': test_env,
        'workdir': meson.current_build_dir(),
        'protocol': 'tap',
        'suite': 'tests',
    }

    foreach lib, functions : hash_libs
        test(
            '/hash/lib/' + lib,
            gtkhash_test,
            kwargs: test_kwargs + {
                'env': test_env + ['GTKHASH_TEST_LIB=' + lib],
            },
        )
    endforeach

    foreach prefix : [
        '/hash/lib/any',
        '/hash/dev/blk',
        '/hash/dev/chr',
        '/opt',
        '/digest-format',
        '/check',
    ]
        test(
            prefix,
            gtkhash_test,
            kwargs: test_kwargs,
            args: ['-r', prefix],
        )
    endforeach
endif
