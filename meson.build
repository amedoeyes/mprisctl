project(
  'mprisctl',
  'cpp',
  version: '0.1.1',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
    'prefer_static=true',
  ],
)

sources = files(
  './src/main.cpp',
  './src/mpris/mpris.cpp',
  './src/mpris/player.cpp',
  './src/mpris/track_list.cpp',
)
include = include_directories('./include')
dependencies = [
  dependency('dbus-1'),
  dependency('boost', modules: ['program_options'], static: true),
]

executable(
  'mprisctl',
  sources,
  include_directories: include,
  dependencies: dependencies,
  cpp_args: '-DVERSION="@0@"'.format(meson.project_version()),
)
