project(
    'bfi',
    'cpp',
    meson_version: '>=0.55.0',
    version: '1.0.0',
    license: 'MIT',
    default_options: [
        'buildtype=release',
        'strip=true',
        'warning_level=3',
        'b_lto=true',
        'b_ndebug=false',
        'c_std=c2x',
        'cpp_std=c++2a',
        'cpp_eh=none',
        'cpp_rtti=false'
    ]
)

executable(
    'bfi',
    'src/main.cpp',
    dependencies: [dependency('fmt')],
    cpp_args: [
        '-Wextra',
        '-Wdouble-promotion',
        '-Werror=pedantic',
        '-pedantic-errors',
        '-Wfatal-errors'
    ],
    cpp_pch : 'pch/main_pch.hpp'
)
