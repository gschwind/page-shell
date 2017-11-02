
install_data([
  'close_button.png',
  'edit-delete.png',
  'go-next.png',
  'go-previous.png',
  'hsplit_button.png',
  'media-eject.png',
  'pop.png',
  'pop_button_bk.png',
  'pop_selected.png',
  'pops.png',
  'pops_button.png',
  'tiny_hsplit_button.png',
  'tiny_pop.png',
  'tiny_pops.png',
  'tiny_vsplit_button.png',
  'view-restore.png',
  'vsplit_button.png',
  'window-close-1.png',
  'window-close-2.png',
  'window-close-3.png',
  'window-close-4.png',
  'window-close.png'],
  install_dir: join_paths(pkgdatadir, 'data')
)

cdatax = configuration_data()
cdatax.set('bindir', bindir)
configure_file(
  input:  'net.hzog.Shell.desktop.in',
  output: 'net.hzog.Shell.desktop',
  configuration: cdatax,
  install_dir: desktopdir
)
