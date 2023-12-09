phantom_srcs = files()

phantom_deps = [
  libenigma_dep,
  vulkan_dep,
  x11_dep,
  ]

# Conditional srcs
if x11_dep.found()
  phantom_srcs += [
    files('src/p_x11_window.c'),
    ]
  phantom_deps += x11_dep
endif

libphantom = library(
  'phantom',
  sources: phantom_srcs,
  dependencies: phantom_deps,
  include_directories: include_directories('include'),
  install : false)

libphantom_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libphantom)
