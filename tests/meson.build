add_test_setup(
  'default',
  exclude_suites: 'gui',
  is_default: true,
)
add_test_setup(
  'full',
)

test_bins = [
  'test-menu-parser',
  'test-menu-spec',
]
test_gui_bins = [
  'test-display-menu',
]

foreach bin : test_bins + test_gui_bins
  e = executable(
    bin,
    sources: [
      '@0@.c'.format(bin),
    ],
    include_directories: [
      include_directories('..'),
    ],
    dependencies: [
      glib,
      gio,
      gobject,
      gtk,
      libxfce4util,
    ],
    link_with: [
      garcon,
      garcon_gtk,
    ],
    install: false,
  )

  if bin in test_gui_bins
    test(bin, e, suite: 'gui')
  else
    test(bin, e)
  endif
endforeach
