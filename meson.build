project('otellogs', 'cpp', version: 'cpp_std=20')

cpp_source_files = ['main.cpp']
cmake = import('cmake')

opentelemetry_proj = cmake.subproject('opentelemetry')
opentelemetry_dep = opentelemetry_proj.dependency('opentelemetry')


executable('otellogs', 
cpp_source_files,
dependencies: opentelemetry_dep
)
