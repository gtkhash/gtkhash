schemas = []

if build_gtkhash
    schemas += 'org.gtkhash.gschema.xml'
endif

if enable_extensions
    schemas += 'org.gtkhash.plugin.gschema.xml'
endif

install_data(
    schemas,
    install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
)

gnome.compile_schemas(depend_files: schemas)
