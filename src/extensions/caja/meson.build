shared_library(
    'gtkhash-caja',
    'module.c',
    include_directories: root_include,
    link_whole: libproperties,
    link_args: [
        '-Wl,--version-script,@0@'.format(meson.current_source_dir() / 'extension.map'),
    ],
    dependencies: caja,
    install: true,
    install_dir: caja_extension_dir,
)
