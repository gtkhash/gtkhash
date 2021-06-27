icons_install_dir = get_option('datadir')/'icons'/'hicolor'
generated_sizes = [ 32, 48, 64, 128, 256, ]

icon_basename = 'org.gtkhash.gtkhash'

svg_file = files('scalable/@0@.svg'.format(icon_basename))

install_data(svg_file, install_dir: icons_install_dir / 'scalable' / 'apps')

foreach size : generated_sizes
    sizestr = '@0@'.format(size)
    sizedir = '@0@x@0@'.format(size)

    local_icon_name = '@0@.@1@.png'.format(icon_basename, sizedir)

    icon = custom_target(local_icon_name,
        input: svg_file,
        output: local_icon_name,
        command: [
            'rsvg-convert',
            '-w', sizestr,
            '-h', sizestr,
            '@INPUT@', '-o', '@OUTPUT@'
        ],
        install: true,
        install_dir: icons_install_dir / sizedir / 'apps',
        # install_rename: '@0@.png'.format(icon_basename),
    )

    # That's required because Meson does not yet allow custom_target to be
    # renamed on install step.
    meson.add_install_script('rename_icons.sh',
        icons_install_dir/sizedir/'apps',
        local_icon_name, '@0@.png'.format(icon_basename),
    )
endforeach
