metainfos = []

if build_gtkhash
    metainfos += 'org.gtkhash.gtkhash.metainfo.xml'
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

if build_thunar
    metainfos += 'org.gtkhash.thunar.metainfo.xml'
endif

appstreamcli = find_program('appstreamcli')
override_args = []
if find_program('appstreamcli', version: '>=0.15.4', required: false).found()
    override_args += ['--override', 'developer-id-missing=info']
endif

foreach metainfo_name : metainfos
    metainfo_config = configure_file(
        input: metainfo_name + '.in.in',
        output: metainfo_name + '.in-config',
        configuration: data_config,
    )

    metainfo_in = custom_target(
        metainfo_name + '.in',
        input: metainfo_config,
        output: metainfo_name + '.in',
        command: [
            appstreamcli,
            'news-to-metainfo',
            '--format=text',
            '--limit=5',
            '--translatable-count=0',
            news_file,
            '@INPUT@',
            '@OUTPUT@',
        ],
    )

    metainfo_file = i18n.merge_file(
        input: metainfo_in,
        output: metainfo_name,
        install: true,
        install_dir: get_option('datadir') / 'metainfo',
        po_dir: po_dir,
        type: 'xml',
    )

    test(
        'appstreamcli validate ' + metainfo_name,
        appstreamcli,
        args: ['validate', '--pedantic', '--explain', '--no-net'] + override_args + [metainfo_file],
        suite: 'data',
    )
endforeach
