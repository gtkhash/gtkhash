metainfos = []

if build_gtkhash
    metainfos += 'org.gtkhash.gtkhash.appdata.xml'
endif
if build_caja
    metainfos += 'org.gtkhash.caja.metainfo.xml'
endif
if build_nautilus
    metainfos += 'org.gtkhash.nautilus.metainfo.xml'
endif
if build_nemo
    metainfos += 'org.gtkhash.nemo.metainfo.xml'
endif
if build_peony
    metainfos += 'org.gtkhash.peony.metainfo.xml'
endif
if build_thunar
    metainfos += 'org.gtkhash.thunar.metainfo.xml'
endif

foreach metainfo : metainfos
    metainfo_in = configure_file(
        input:  metainfo + '.in.in',
        output: metainfo + '.in',
        configuration: data_config,
    )
    i18n.merge_file(metainfo,
        input:  metainfo_in,
        output: metainfo,
        install: true,
        install_dir: get_option('datadir') / 'metainfo',

        po_dir: po_dir,
        type: 'xml',
    )
endforeach
