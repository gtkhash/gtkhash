install_data(
    'org.gtkhash.plugin.gschema.xml',
    install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
)

gnome.compile_schemas(depend_files: 'org.gtkhash.plugin.gschema.xml')

gtkhash_properties_gresource = files('gtkhash-properties.gresource.xml')
