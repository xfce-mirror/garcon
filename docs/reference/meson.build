if get_option('gtk-doc')
  garcon_doc_name = 'garcon'

  conf_data = configuration_data()
  conf_data.set_quoted('PACKAGE_NAME', garcon_doc_name)
  conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
  conf_data.set_quoted('PACKAGE_STRING', '@0@-@1@'.format(garcon_doc_name, meson.project_version()))

  configure_file(
    input: 'gtkdocentities.ent.in',
    output: 'gtkdocentities.ent',
    configuration: conf_data,
  )

  gnome.gtkdoc(
    garcon_doc_name,
    src_dir: [
      'garcon',
      'garcon-gtk',
    ],
    ignore_headers: [
      'garcon-visibility.h',
      'garcon-gtk-visibility.h',
      'garcon-marshal.h',
    ],
    main_xml: 'garcon-docs.xml',
    dependencies: [
      garcon_dep,
      garcon_gtk_dep,
    ],
    gobject_typesfile: 'garcon.types',
    install: true,
    check: false,
  )
endif
