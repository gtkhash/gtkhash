properties_resources = gnome.compile_resources(
    'resources',
    gtkhash_properties_gresource,
    c_name: 'gtkhash_properties_resources',
    source_dir: meson.current_source_dir() / '..' / '..' / 'data' / 'nautilus',
)
extensions_sources = [
    'properties.c',
    'properties-hash.c',
    'properties-list.c',
    'properties-prefs.c',
    properties_resources,
]

extensions = []

if build_caja
    extensions += { 'name': 'caja',
        'dep': caja,
        'dir': caja_extension_dir,
        'symbol_prefix': 'caja_module_',
    }
endif
if build_nautilus
    extensions += { 'name': 'nautilus',
        'dep': nautilus,
        'dir': nautilus_extension_dir,
        'symbol_prefix': 'nautilus_module_',
    }
endif
if build_nemo
    extensions += { 'name': 'nemo',
        'dep': nemo,
        'dir': nemo_extension_dir,
        'symbol_prefix': 'nemo_module_',
    }
endif
if build_peony
    extensions += { 'name': 'peony',
        'dep': peony,
        'dir': peony_extension_dir,
        'symbol_prefix': 'peony_module_',
    }
endif
if build_thunar
    extensions += { 'name': 'thunar',
        'dep': thunar,
        'dir': thunar_extension_dir,
        'symbol_prefix': 'thunar_extension_',
    }
endif

foreach extension : extensions
    name = extension.get('name')

    extension_map_file = configure_file(
        input:  'extension.map.in',
        output: 'extension_@0@.map'.format(name),
        configuration: { 'symbol_prefix': extension.get('symbol_prefix')},
    )

    shared_library('gtkhash-properties-@0@'.format(name),
        extensions_sources,
        include_directories: root_include,
        c_args: [ '-DIN_@0@_EXTENSION'.format(name.to_upper()), ],
        link_with: [
            libhash,
            libutil,
        ],
        link_args: [ '-Wl,--version-script,@0@'.format(extension_map_file), ],
        dependencies: extension.get('dep'),
        install: true,
        install_dir: extension.get('dir'),
    )
endforeach
