cajaextension = 'libgtkhash-caja.caja-extension'

cajaextension_in = configure_file(
    input: 'libgtkhash-caja.desktop.in.in',
    output: cajaextension + '.in',
    configuration: data_config,
)

i18n.merge_file(
    input: cajaextension_in,
    output: cajaextension,
    install: true,
    install_dir: get_option('datadir') / 'caja' / 'extensions',
    po_dir: po_dir,
    type: 'desktop',
    args: [
        '--keyword=Name',
        '--keyword=Description',
    ],
)
