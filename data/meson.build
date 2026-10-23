if enable_extensions
    subdir('extensions')
endif

if enable_icons
    subdir('icons')
endif

if enable_appstream
    subdir('metainfo')
endif

if enable_schemas
    subdir('schemas')
endif

if build_gtkhash
    desktop_name = application_id + '.desktop'

    desktop_file = i18n.merge_file(
        input: desktop_name + '.in',
        output: desktop_name,
        install: true,
        install_dir: get_option('datadir') / 'applications',
        po_dir: po_dir,
        type: 'desktop',
    )

    desktop_file_validate = find_program('desktop-file-validate', required: false)
    if desktop_file_validate.found()
        test(
            'desktop-file-validate ' + desktop_name,
            desktop_file_validate,
            args: [desktop_file],
            suite: 'data',
        )
    endif
endif

gtkhash_gtk3_gresource = files('gtkhash-gtk3.gresource.xml')
