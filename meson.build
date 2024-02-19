platinum_srcs = [
  'src/p_app.c',
  'src/p_window.c',
  'src/p_graphics.c',
  'src/util/p_debug_mem.c',
  'src/util/p_file.c',
  'src/util/p_log.c',
  'src/util/p_thread.c',
  ]

platinum_deps = [
  dep_libenigma,
  ]

platinum_c_args = [
  '-D_PLATINUM_INTERNAL',
  ]

# Platinum Graphics Settings
if graphics == 'vulkan'
  platinum_c_args += [
    '-DPLATINUM_GRAPHICS_VULKAN',
  ]
  platinum_srcs += [
    'src/p_graphics_vulkan.c',
  ]
  platinum_deps += [
    dependency('vulkan', required : true)
  ]
endif


## Phantom Window Settings
# Windows
if host_machine.system() == 'windows'
  platinum_srcs += [
    'src/p_window_win32.c',
    'src/p_app_windows.c',
    ]
  platinum_c_args += [
    '-DPLATINUM_PLATFORM_WINDOWS',
    '-DPLATINUM_DISPLAY_WIN32',
    ]
# Linux
elif host_machine.system() == 'linux'
  platinum_srcs += [
    'src/p_app_linux.c',
    ]
  platinum_deps += [
    dependency('libevdev', required : true),
    dependency('libudev', required : true),
    ]
  platinum_c_args += [
    '-DPLATINUM_PLATFORM_LINUX',
    ]
# X11
  if display == 'x11'
    platinum_deps += [
      dependency('xcb', required : true)
      ]
    platinum_c_args += [
      '-DPLATINUM_DISPLAY_X11',
    ]
    platinum_srcs += [
      'src/p_window_x11.c',
      ]
# Wayland
  elif display == 'wayland'
    platinum_deps += [
      dependency('wayland-client', required : true),
    ]
    platinum_c_args += [
      '-DPLATINUM_DISPLAY_WAYLAND',
    ]
    platinum_srcs += [
      'src/p_window_wayland.c',
      ]
  elif display == 'ncurses'
    platinum_deps += [
      dependency('ncurses', required : true)
    ]
    platinum_c_args += [
      '-DPLATINUM_DISPLAY_NCURSES',
    ]
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
