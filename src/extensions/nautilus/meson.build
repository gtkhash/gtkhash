shared_library(
    'gtkhash-nautilus',
    'module.c',
    'properties.c',
    'properties-hash.c',
    'properties-prefs.c',
    include_directories: root_include,
    link_args: [
        '-Wl,--version-script,@0@'.format(meson.current_source_dir() / 'extension.map'),
    ],
    link_whole: [
        libmodule,
    ],
    link_with: [
        libhash,
        libutil,
    ],
    dependencies: nautilus,
    install: true,
    install_dir: nautilus_extension_dir,
)
