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

configure_file(
    input: 'lsan.supp',
    output: 'lsan.supp',
    copy: true,
)

test_env = [
    'GIO_USE_VFS=local',
    'GSETTINGS_BACKEND=memory',
    'LSAN_OPTIONS=suppressions=lsan.supp',
]

if enable_schemas
    test_env += 'GSETTINGS_SCHEMA_DIR=@0@'.format(schemas_build_dir)
endif

test_kwargs = {
    'env': test_env,
    'workdir': meson.current_build_dir(),
    'protocol': 'tap',
    'suite': 'tests',
}

if build_gtkhash
    gtkhash_test = executable(
        'gtkhash-test',
        gtkhash_common_sources,
        gtkhash_resources,
        'test.c',
        'test-check.c',
        include_directories: [
            root_include,
            src_include,
        ],
        link_with: [
            libhash,
            libutil,
        ],
        dependencies: gtk,
    )

    configure_file(
        input: 'test.md5sum.in',
        output: 'test.md5sum',
        configuration: {
            'builddir': meson.current_build_dir(),
        },
    )

    foreach lib, functions : hash_libs
        test(
            '/hash/lib/' + lib,
            gtkhash_test,
            depends: [test_files, schemas_compiled],
            kwargs: test_kwargs + {
                'env': test_env + ['GTKHASH_TEST_LIB=' + lib],
            },
        )
    endforeach

    foreach prefix : [
        '/hash/lib/any',
        '/hash/dev/blk',
        '/hash/dev/chr',
        '/hash/file',
        '/opt',
        '/digest-format',
        '/check',
    ]
        test(
            prefix,
            gtkhash_test,
            depends: [test_files, schemas_compiled],
            kwargs: test_kwargs,
            args: ['-r', prefix],
        )
    endforeach
endif

if build_caja or build_nemo or build_thunar
    gtkhash_test_extensions_common_properties = executable(
        'gtkhash-test-extensions-common-properties',
        'test-extensions-common-properties.c',
        properties_resources,
        include_directories: [
            root_include,
            src_include,
        ],
        link_with: [
            libhash,
            libutil,
        ],
        dependencies: gtk,
    )

    test(
        '/extensions/common',
        gtkhash_test_extensions_common_properties,
        depends: [test_files, schemas_compiled],
        kwargs: test_kwargs,
    )
endif

if build_nautilus
    gtkhash_test_extensions_nautilus_properties = executable(
        'gtkhash-test-extensions-nautilus-properties',
        'test-extensions-nautilus-properties.c',
        include_directories: [
            root_include,
            src_include,
        ],
        link_with: [
            libhash,
            libutil,
        ],
        dependencies: nautilus,
    )

    test(
        '/extensions/nautilus',
        gtkhash_test_extensions_nautilus_properties,
        depends: [test_files, schemas_compiled],
        kwargs: test_kwargs,
    )
endif
