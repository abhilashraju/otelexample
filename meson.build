project('otellogs', 'cpp', version: 'cpp_std=20')
cxx = meson.get_compiler('cpp')
cpp_source_files = ['main.cpp','foo_library.cc']

opentelemetry_trace = cxx.find_library('opentelemetry_trace', required: true)
opentelemetry_logs =  cxx.find_library('opentelemetry_logs', required: true)
opentelemetry_exporter_ostream_span = cxx.find_library('opentelemetry_exporter_ostream_span', required: true)
opentelemetry_exporter_ostream_logs =  cxx.find_library('opentelemetry_exporter_ostream_logs', required: true)
opentelemetry_resources = cxx.find_library('opentelemetry_resources', required: true)
opentelemetry_common = cxx.find_library('opentelemetry_common', required: true)

opentelemetry_dep = [opentelemetry_common,opentelemetry_resources,opentelemetry_trace, opentelemetry_logs,opentelemetry_exporter_ostream_span,opentelemetry_exporter_ostream_logs]
opentelemetry_includes=['.']
executable('otellogs', 
cpp_source_files,
dependencies: opentelemetry_dep,
include_directories:opentelemetry_includes,
)
