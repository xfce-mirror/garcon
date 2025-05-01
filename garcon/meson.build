garcon_libname = 'garcon'
garcon_pkgname = '@0@-@1@'.format(garcon_libname, garcon_version_api)

garcon_install_subdir = garcon_pkgname / garcon_libname
garcon_install_dir = get_option('prefix') / get_option('includedir') / garcon_install_subdir

garcon_headers = [
  'garcon-environment.h',
  'garcon-menu-directory.h',
  'garcon-menu-element.h',
  'garcon-menu-item-action.h',
  'garcon-menu-item-cache.h',
  'garcon-menu-item-pool.h',
  'garcon-menu-item.h',
  'garcon-menu-merger.h',
  'garcon-menu-node.h',
  'garcon-menu-parser.h',
  'garcon-menu-separator.h',
  'garcon-menu-tree-provider.h',
  'garcon-menu.h',
  'garcon.h',
]

garcon_headers += configure_file(
  configuration: configuration_data({
    'GARCON_VERSION_MAJOR': version_parts[0],
    'GARCON_VERSION_MINOR': version_parts[1],
    'GARCON_VERSION_MICRO': version_parts[2],
  }),
  input: 'garcon-config.h.in',
  output: 'garcon-config.h',
  install: false,
)

garcon_sources = [
  'garcon-config.c',
  'garcon-environment.c',
  'garcon-menu-directory.c',
  'garcon-menu-element.c',
  'garcon-menu-item-action.c',
  'garcon-menu-item-cache.c',
  'garcon-menu-item-pool.c',
  'garcon-menu-item.c',
  'garcon-menu-merger.c',
  'garcon-menu-node.c',
  'garcon-menu-parser.c',
  'garcon-menu-separator.c',
  'garcon-menu-tree-provider.c',
  'garcon-menu.c',
  'garcon-private.c',
  'garcon-private.h',
]

garcon_sources += gnome.genmarshal(
  'garcon-marshal',
  sources: 'garcon-marshal.list',
  prefix: 'garcon_marshal',
  internal: true,
  install_header: false,
)

garcon_sources += configure_file(
  input: 'garcon.symbols',
  output: 'garcon-visibility.h',
  command: [xdt_gen_visibility, '--kind=header', '@INPUT@', '@OUTPUT@'],
)
garcon_sources += configure_file(
  input: 'garcon.symbols',
  output: 'garcon-visibility.c',
  command: [xdt_gen_visibility, '--kind=source', '@INPUT@', '@OUTPUT@'],
)

garcon_deps = [
  glib,
  gio,
  gobject,
  gthread,
  libxfce4util,
]

garcon_so_version = '0'
garcon_version = '@0@.0.0'.format(garcon_so_version)

garcon = library(
  garcon_pkgname,
  sources: garcon_sources,
  version: garcon_version,
  soversion: garcon_so_version,
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format(garcon_libname),
    '-DGARCON_COMPILATION=1',
  ],
  include_directories: [
    include_directories('..'),
  ],
  gnu_symbol_visibility: gnu_symbol_visibility,
  dependencies: garcon_deps,
  install: true,
)

install_headers(
  garcon_headers,
  subdir: garcon_install_subdir,
)

pkgconfig.generate(
  libraries: [
    garcon,
  ],
  version: meson.project_version(),
  name: garcon_libname,
  filebase: garcon_pkgname,
  description: 'Freedesktop.org compliant menu library',
  subdirs: [
    garcon_pkgname,
  ],
  variables: [
    'garcon_api_version=@0@'.format(garcon_version_api),
  ],
  install_dir: get_option('prefix') / get_option('libdir') / 'pkgconfig',
)

if get_option('introspection')
  garcon_gir = gnome.generate_gir(
    garcon,
    sources: garcon_sources + garcon_headers,
    namespace: 'Garcon',
    nsversion: '@0@.0'.format(garcon_version_api),
    identifier_prefix: [
      'Garcon',
    ],
    symbol_prefix: [
      'garcon',
    ],
    export_packages: garcon_pkgname,
    includes: [
      'GLib-2.0',
      'GObject-2.0',
      'Gio-2.0',
      'Libxfce4util-1.0',
    ],
    header: 'garcon' / 'garcon.h',
    install: true,
    extra_args: [
      '-DGARCON_COMPILATION=1',
    ],
  )
endif

garcon_dep = declare_dependency(
  dependencies: garcon_deps,
  link_with: garcon,
)
