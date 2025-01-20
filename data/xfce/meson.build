desktop_files = [
  'xfce-accessories.desktop.in',
  'xfce-development.desktop.in',
  'xfce-education.desktop.in',
  'xfce-games.desktop.in',
  'xfce-graphics.desktop.in',
  'xfce-hardware.desktop.in',
  'xfce-multimedia.desktop.in',
  'xfce-network.desktop.in',
  'xfce-office.desktop.in',
  'xfce-other.desktop.in',
  'xfce-personal.desktop.in',
  'xfce-science.desktop.in',
  'xfce-screensavers.desktop.in',
  'xfce-settings.desktop.in',
  'xfce-system.desktop.in',
] 

foreach file : desktop_files
  i18n.merge_file(
    input: file,
    output: file.replace('.desktop.in', '.directory'),
    po_dir: '..' / '..' / 'po',
    type: 'desktop',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'desktop-directories',
  )
endforeach

install_data(
  'xfce-applications.menu',
  install_dir: get_option('prefix') / get_option('sysconfdir') / 'xdg' / 'menus',
)
