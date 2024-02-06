platinum_srcs = []

platinum_deps = [
  dep_libenigma,
  ]

platinum_c_args = [
  '-D_PLATINUM_INTERNAL',
  ]

## Phantom build setup
# Vulkan
if graphics == 'vulkan'
  platinum_srcs += [
    files('src/p_graphics_vulkan.c'),
  ]
  platinum_deps += [
    dep_vulkan,
  ]
endif
# Windows
if host_machine.system() == 'windows'
  platinum_srcs += [
    files('src/p_window_win32.c'),
    files('src/p_app_windows.c'),
    #files('src/input/p_windows_events.c'),
    ]
  platinum_c_args += [
    '-DPLATINUM_PLATFORM_WINDOWS',
    ]
# Linux
elif host_machine.system() == 'linux'
  platinum_srcs += [
    files('src/p_app_linux.c'),
    files('src/p_events_linux.c'),
    ]
  platinum_deps += [
    dep_display,
    dependency('libevdev', required : true),
    dependency('libudev', required : true),
    ]
  platinum_c_args += [
    '-DPLATINUM_PLATFORM_LINUX',
    ]
# X11
  if display == 'x11'
    platinum_srcs += [
      files('src/p_window_x11.c'),
      ]
# Wayland
  elif display == 'wayland'
    platinum_srcs += [
      files('src/p_window_wayland.c'),
      ]
  elif display == 'ncurses'
    message('TODO: implement me')
  else
    error('There must be at least one graphical back-end installed.')
  endif
endif

libplatinum = library(
  'platinum',
  sources: platinum_srcs,
  dependencies: platinum_deps,
  include_directories: include_directories('include'),
  c_args : platinum_c_args,
  install : true)

subdir('shaders')

dep_libplatinum = declare_dependency(
  include_directories: include_directories('include'),
  link_with : libplatinum)
