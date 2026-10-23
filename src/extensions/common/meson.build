properties_resources = gnome.compile_resources(
    'properties-resources',
    'properties.gresource.xml',
    c_name: 'gtkhash_properties_resources',
)

libproperties = static_library(
    'properties',
    'properties.c',
    'properties-hash.c',
    'properties-list.c',
    'properties-prefs.c',
    properties_resources,
    include_directories: root_include,
    dependencies: [
        glib,
        gtk,
    ],
    link_with: [
        libhash,
        libutil,
    ],
)
