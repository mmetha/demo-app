project(
    'demorecipe',
    'cpp',
    version: '0.1',
    meson_version: '>=1.1.1',
    default_options: [
        'werror=true',
        'warning_level=3',
        'cpp_std=c++23',
  ])

boost_dep = dependency('boost', required: true)
sdbusplus_dep = dependency('sdbusplus', required: true)

deps = [
    boost_dep,
    sdbusplus_dep
]

demorecipe = executable(
    'demorecipe',
    'main.cpp',
    implicit_include_directories: false,
    dependencies: deps,
    install: true,
    install_dir: get_option('bindir')
)
