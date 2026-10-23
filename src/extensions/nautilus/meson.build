shared_library(
    'gtkhash-nautilus',
    'module.c',
    include_directories: root_include,
    link_whole: libproperties,
    link_args: [
        '-Wl,--version-script,@0@'.format(meson.current_source_dir() / 'extension.map'),
    ],
    dependencies: nautilus,
    install: true,
    install_dir: nautilus_extension_dir,
)
