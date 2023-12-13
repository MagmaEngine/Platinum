phantom_srcs = files()

phantom_deps = [
  dep_libenigma,
  dep_vulkan,
  ]


message(host_machine.system())

## Phantom build setup
# Check OS
if target_os == 'windows'
elif target_os == 'linux'
  phantom_srcs += [
    files('src/p_linux_events.c'),
    files('src/p_linux_app.c'),
    ]
  phantom_deps += [
    dep_evdev,
    dep_udev,
    ]
endif

# Check Display
if target_display == 'windows'
  phantom_srcs += [
    files('src/p_win32_window.c'),
  ]
elif target_display == 'x11'
  phantom_deps += dep_x11
  phantom_srcs += [
    files('src/p_x11_window.c'),
  ]
elif target_display == 'wayland'
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
