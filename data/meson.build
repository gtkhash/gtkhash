if enable_icons
    subdir('icons')
endif

if enable_appstream
    subdir('appdata')
endif

if build_caja
    subdir('caja')
endif

if enable_nautilus_data
    subdir('nautilus')
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

    install_data(
        'org.gtkhash.gschema.xml',
        install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
    )

    gnome.compile_schemas(depend_files: 'org.gtkhash.gschema.xml')
endif

gtkhash_gtk3_gresource = files('gtkhash-gtk3.gresource.xml')
