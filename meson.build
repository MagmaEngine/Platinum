phantom_srcs = []

phantom_deps = [
  dep_libenigma,
  ]

phantom_c_args = []

## Phantom build setup
# Vulkan
if graphics == 'vulkan'
  phantom_srcs += [
    files('src/p_vulkan.c'),
  ]
  phantom_deps += [
    dep_vulkan,
  ]
endif
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
    dep_display,
    dependency('libevdev', required : true),
    dependency('libudev', required : true),
    ]
  phantom_c_args += [
    '-fvisibility=hidden',
    ]
# X11
  if display == 'x11'
    phantom_srcs += [
      files('src/p_x11_window.c'),
      ]
# Wayland
  elif display == 'wayland'
    phantom_srcs += [
      files('src/p_wayland_window.c'),
      ]
  elif display == 'ncurses'
    message('TODO: implement me')
  else
    error('There must be at least one graphical back-end installed.')
  endif
endif

if library_buildtype == 'shared'
  phantom_c_args += '-D_PHANTOM_INTERNAL'
  libphantom = shared_library(
    'phantom',
    sources: phantom_srcs,
    dependencies: phantom_deps,
    include_directories: include_directories('include'),
    c_args : phantom_c_args,
    install : true)
elif library_buildtype == 'static'
  libphantom = static_library(
    'phantom',
    sources: phantom_srcs,
    dependencies: phantom_deps,
    include_directories: include_directories('include'),
    c_args : phantom_c_args,
    install : false)
endif

dep_libphantom = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libphantom)
