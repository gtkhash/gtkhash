
peonyextension = 'libgtkhash-properties-peony.peony-extension'

peonyextension_in = configure_file(
    input:  peonyextension + '.in.in',
    output: peonyextension + '.in',
    configuration: data_config,
)

i18n.merge_file(peonyextension,
    input:  peonyextension_in,
    output: peonyextension,
    install: true,
    install_dir: get_option('datadir') / 'peony' / 'extensions',

    po_dir: po_dir,
    type: 'desktop',
    args: [
        '--keyword', 'Name',
        '--keyword', 'Description',
    ]
)
