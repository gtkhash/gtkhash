subdir('icons')

if enable_appstream
    subdir('appdata')
endif

if build_caja
    subdir('caja')
endif

if build_peony
    subdir('peony')
endif

if enable_nautilus_data
    subdir('nautilus')
endif

if build_gtkhash
    i18n.merge_file(
        input:  'org.gtkhash.gtkhash.desktop.in',
        output: 'org.gtkhash.gtkhash.desktop',
        install: true,
        install_dir: get_option('datadir') / 'applications',

        po_dir: po_dir,
        type: 'desktop',
    )

    install_data(
        'org.gtkhash.gschema.xml',
        install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
    )
endif

gtkhash_gtk3_gresource = files('gtkhash-gtk3.gresource.xml')
