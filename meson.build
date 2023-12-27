phantom_srcs = []

phantom_deps = [
  dep_libenigma,
  dependency('vulkan', required : true),
  ]

phantom_c_args = [
  '-D_PHANTOM_INTERNAL',
  ]

## Phantom build setup
# Windows
if host_machine.system() == 'windows'
  phantom_srcs += [
    files('src/p_win32_window.c'),
    files('src/p_windows_app.c'),
    files('src/p_windows_events.c'),
    ]
  phantom_c_args += [
    '-DBUILDING_MY_LIBRARY',
    ]
# Linux
elif host_machine.system() == 'linux'
  phantom_srcs += [
    files('src/p_linux_app.c'),
    files('src/p_linux_events.c'),
    ]
  phantom_deps += [
    dependency('libevdev', required : true),
    dependency('libudev', required : true),
    ]
  phantom_c_args += [
    '-fvisibility=hidden',
    ]
# X11
  if dep_x11[0].found() and dep_x11[1].found()
    phantom_srcs += [
      files('src/p_x11_window.c'),
      files('src/p_x11_vulkan.c'),
      ]
    phantom_deps += dep_x11
    phantom_c_args += [
      '-DPHANTOM_DISPLAY_X11',
      ]
# Wayland
  elif dep_wayland.found()
    phantom_srcs += [
      files('src/p_wayland_window.c'),
      ]
    phantom_deps += dep_wayland
    phantom_c_args += [
      '-DPHANTOM_DISPLAY_WAYLAND',
      ]
  elif dep_ncurses.found()
    message('TODO: implement me')
  else
    error('There must be at least one graphical back-end installed.')
  endif

endif

libphantom = library(
  'phantom',
  sources: phantom_srcs,
  dependencies: phantom_deps,
  include_directories: include_directories('include'),
  c_args : phantom_c_args,
  install : false)

dep_libphantom = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libphantom)
