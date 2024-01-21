glsl_compiler = find_program('glslangValidator')

src_dir = meson.current_source_dir()
build_dir = meson.current_build_dir()

glsl_sources = [
  'shader.vert',
  'shader.frag',
  ]

foreach shader : glsl_sources
  shader_output = shader.replace('.vert', '_vert.spv').replace('.frag', '_frag.spv')
  #shader = src_dir + '/' + shader
  message('Compiling GLSL:', shader, '->', shader_output)
  shader_target = custom_target(
    input : shader,
    output : shader_output,
    command : [glsl_compiler, '-V', '@INPUT@', '-o', '@OUTPUT@'],
    build_by_default : true,
    install : true,
    install_dir : 'share/DarkEngine/shaders',
    )
endforeach
