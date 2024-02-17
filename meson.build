project('otellogs', 'cpp', version: 'cpp_std=20')

cpp_source_files = ['main.cpp']
opentelemetry=dependency('opentelemetry', version: '1.0.0')

executable('otellogs', 
cpp_source_files,
dependencies: opentelemetry
)
