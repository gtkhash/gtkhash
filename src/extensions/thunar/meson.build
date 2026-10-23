shared_library(
    'gtkhash-thunar',
    'module.c',
    include_directories: root_include,
    link_whole: libproperties,
    link_args: [
        '-Wl,--version-script,@0@'.format(meson.current_source_dir() / 'extension.map'),
    ],
    dependencies: thunar,
    install: true,
    install_dir: thunar_extension_dir,
)
