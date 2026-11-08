if build_gtkhash
    gtkhash_test = executable(
        'gtkhash-test',
        gtkhash_common_sources,
        gtkhash_resources,
        'test.c',
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

    foreach bytes_test : ['0', '1', '10M', '5G']
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

    test_env = [
        'LSAN_OPTIONS=suppressions=lsan.supp',
    ]

    foreach lib, functions : hash_libs
        test(
            '/hash/lib/@0@'.format(lib),
            gtkhash_test,
            depends: test_files,
            env: test_env + ['GTKHASH_TEST_LIB=@0@'.format(lib)],
            workdir: meson.current_build_dir(),
            protocol: 'tap',
            suite: 'tests',
        )
    endforeach

    foreach prefix : ['/hash/lib/any', '/hash/dev/blk', '/hash/dev/chr', '/opt', '/digest-format']
        test(
            prefix,
            gtkhash_test,
            depends: test_files,
            args: ['-r', prefix],
            env: test_env,
            workdir: meson.current_build_dir(),
            protocol: 'tap',
            suite: 'tests',
        )
    endforeach
endif
