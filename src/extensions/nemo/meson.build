shared_library(
    'gtkhash-nemo',
    'module.c',
    include_directories: root_include,
    link_args: [
        '-Wl,--version-script,@0@'.format(meson.current_source_dir() / 'extension.map'),
    ],
    link_whole: [
        libmodule,
        libproperties,
    ],
    dependencies: nemo,
    install: true,
    install_dir: nemo_extension_dir,
)
