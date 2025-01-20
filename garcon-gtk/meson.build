garcon_gtk_libname = 'garcon-gtk'
garcon_gtk_pkgname = '@0@3-@1@'.format(garcon_gtk_libname, garcon_version_api)

garcon_gtk_install_subdir = garcon_gtk_pkgname / garcon_gtk_libname
garcon_gtk_install_dir = get_option('prefix') / get_option('includedir') / garcon_gtk_install_subdir

garcon_gtk_headers = [
  'garcon-gtk-menu.h',
  'garcon-gtk.h',
]

garcon_gtk_sources = [
  'garcon-gtk-menu.c',
]

garcon_gtk_sources += custom_target(
  'garcon-gtk-visibility.h',
  input: 'garcon-gtk.symbols',
  output: 'garcon-gtk-visibility.h',
  command: [xdt_gen_visibility, '--kind=header', '@INPUT@', '@OUTPUT@'],
)
garcon_gtk_sources += custom_target(
  'garcon-gtk-visibility.c',
  input: 'garcon-gtk.symbols',
  output: 'garcon-gtk-visibility.c',
  command: [xdt_gen_visibility, '--kind=source', '@INPUT@', '@OUTPUT@'],
)

garcon_gtk_deps = [
  glib,
  gio,
  gobject,
  gthread,
  gtk,
  libxfce4ui,
  libxfce4util,
]

garcon_gtk_so_version = '0'
garcon_gtk_version = '@0@.0.0'.format(garcon_gtk_so_version)

garcon_gtk = library(
  garcon_gtk_pkgname,
  sources: garcon_gtk_sources,
  version: garcon_gtk_version,
  soversion: garcon_gtk_so_version,
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format(garcon_gtk_libname),
    '-DGARCON_GTK_COMPILATION=1',
  ],
  include_directories: [
    include_directories('..'),
  ],
  gnu_symbol_visibility: gnu_symbol_visibility,
  dependencies: garcon_gtk_deps,
  link_with: [
    garcon,
  ],
  install: true,
)

install_headers(
  garcon_gtk_headers,
  subdir: garcon_gtk_install_subdir,
)

pkgconfig.generate(
  libraries: [
    garcon_gtk,
  ],
  version: meson.project_version(),
  name: garcon_gtk_libname,
  filebase: garcon_gtk_pkgname,
  description: 'Freedesktop.org compliant menu library',
  subdirs: [
    garcon_gtk_pkgname,
  ],
  variables: [
    'garcon_api_version=@0@'.format(garcon_version_api),
  ],
  install_dir: get_option('prefix') / get_option('libdir') / 'pkgconfig',
)

if get_option('introspection')
  garcon_gtk_gir = gnome.generate_gir(
    garcon_gtk,
    sources: garcon_gtk_sources + garcon_gtk_headers,
    namespace: 'GarconGtk',
    nsversion: '@0@.0'.format(garcon_version_api),
    identifier_prefix: [
      'GarconGtk',
    ],
    symbol_prefix: [
      'garcon_gtk',
    ],
    export_packages: garcon_gtk_pkgname,
    includes: [
      'GLib-2.0',
      'GObject-2.0',
      'Gio-2.0',
      'Gtk-3.0',
      'Libxfce4util-1.0',
      'Libxfce4ui-2.0',
      garcon_gir[0],
    ],
    header: 'garcon-gtk' / 'garcon-gtk.h',
    install: true,
    extra_args: [
      '-DGARCON_GTK_COMPILATION=1',
    ],
  )
endif

garcon_gtk_dep = declare_dependency(
  dependencies: garcon_gtk_deps,
  link_with: garcon_gtk,
)
