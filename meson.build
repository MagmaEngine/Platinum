libphantom = library(
  'phantom',
  'src/ph_main.c',
  include_directories: include_directories('include'),
  install : false)

libphantom_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libphantom)
