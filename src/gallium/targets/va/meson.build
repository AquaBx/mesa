# Copyright © 2017-2018 Intel Corporation
# SPDX-License-Identifier: MIT

# TODO: support non-static targets
# Static targets are always enabled in autotools (unless you modify
# configure.ac)

va_link_args = []
va_link_depends = []
va_drivers = []

va_sym = configure_file(input : 'va.sym.in', output : 'va.sym', configuration : sym_config)

if with_ld_version_script
  va_link_args += ['-Wl,--version-script', join_paths(meson.current_build_dir(), 'va.sym')]
  va_link_depends += va_sym
endif
if with_ld_dynamic_list
  va_link_args += ['-Wl,--dynamic-list', join_paths(meson.current_source_dir(), '../dri.dyn')]
  va_link_depends += files('../dri.dyn')
endif

link_with_libva_gallium = [
  libgalliumvlwinsys, libgalliumvl, libgallium,
  libpipe_loader_static, libws_null, libwsw, libswdri, libswkmsdri,
]

if with_glx == 'xlib'
  link_with_libva_gallium += [libws_xlib]
endif

devenv.set('LIBVA_DRIVERS_PATH', meson.current_build_dir())

if host_machine.system() == 'windows'
  link_with_libva_gallium += [libwsgdi]

  libva_init_version = dep_va.version().split('.')
  vaon12_def_input = configure_file(
    configuration: {
      '__vaDriverInit_X_Y': '__vaDriverInit_@0@_@1@'.format(libva_init_version[0], libva_init_version[1]) + '@4'
    },
    input: 'vaon12.def.in',
    output: 'vaon12.def.in',
  )

  vaon12_def = custom_target(
    'vaon12.def',
    input: vaon12_def_input,
    output : 'vaon12.def',
    command : gen_vs_module_defs_normal_command,
  )

  libva_gallium = shared_library(
    'vaon12_drv_video',
    'va_target.c',
    link_args : [va_link_args, ld_args_gc_sections, ld_args_build_id],
    vs_module_defs : vaon12_def,
    include_directories : [
      inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_util, inc_gallium_winsys, inc_gallium_drivers, inc_gallium_winsys_sw
    ],
    link_whole : [libva_st],
    link_with : link_with_libva_gallium,
    dependencies : [driver_d3d12, idep_mesautil],
    link_depends : va_link_depends,
    install : true,
    name_suffix : 'dll',
    name_prefix : '',  # otherwise mingw will create libvaon12_drv_video.dll
  )
else
  libva_gallium = shared_library(
    'gallium_drv_video',
    'va_target.c',
    gnu_symbol_visibility : 'hidden',
    link_args : [va_link_args, ld_args_gc_sections, ld_args_build_id],
    include_directories : [
      inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_util, inc_gallium_winsys, inc_gallium_drivers,
    ],
    link_whole : [libva_st],
    link_with : link_with_libva_gallium,
    dependencies : [
      dep_libdrm, driver_r600, driver_radeonsi, driver_nouveau, driver_d3d12, driver_virgl,
      idep_mesautil,
    ],
    link_depends : va_link_depends,
    install : true,
    install_dir : va_drivers_path,
    name_suffix : libname_suffix,
  )

  foreach d : [[with_gallium_r600, 'r600'],
              [with_gallium_radeonsi, 'radeonsi'],
              [with_gallium_nouveau, 'nouveau'],
              [with_gallium_virgl, 'virtio_gpu'],
              [with_gallium_d3d12_video, 'd3d12']]
    if d[0]
      va_drivers += '@0@_drv_video.so'.format(d[1])
    endif
  endforeach

  if va_drivers.length() > 0
    meson.add_install_script(
      install_megadrivers,
      libva_gallium.full_path(),
      va_drivers_path,
      va_drivers,
      install_tag : 'runtime',
    )
  endif
endif
