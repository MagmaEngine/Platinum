phantom_srcs = files('src/p_linux_events.c')

phantom_deps = [
  dep_libenigma,
  dep_vulkan,
  ]

if build_phantom == 'windows'
else
  phantom_srcs += [
    files('src/p_linux_events.c'),
    ]
  phantom_deps += [
    dep_evdev,
    dep_udev,
    ]
endif

## Phantom build setup
# Check for Windows
if build_phantom == 'windows'
  phantom_srcs += [
    files('src/p_win32_window.c'),
  ]
elif build_phantom == 'x11'
  phantom_deps += dep_x11
  phantom_srcs += [
    files('src/p_x11_window.c'),
  ]
elif build_phantom == 'wayland'
  phantom_deps += dep_wayland
  phantom_srcs += [
    files('src/p_wayland_window.c'),
  ]
endif

libphantom = library(
  'phantom',
  sources: phantom_srcs,
  dependencies: phantom_deps,
  include_directories: include_directories('include'),
  install : false)

dep_libphantom = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libphantom)
