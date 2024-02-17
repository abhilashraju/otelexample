project('otellogs', 'cpp', version: 'cpp_std=20')
cxx = meson.get_compiler('cpp')
cpp_source_files = ['main.cpp']
opentelemetry_trace = cxx.find_library('opentelemetry_trace', required: true)
opentelemetry_logs =  cxx.find_library('opentelemetry_logs', required: true)
opentelemetry_exporter_ostream_span = cxx.find_library('opentelemetry_exporter_ostream_span', required: true)
opentelemetry_exporter_ostream_logs =  cxx.find_library('opentelemetry_exporter_ostream_logs', required: true)

opentelemetry_dep = [opentelemetry_trace, opentelemetry_logs,opentelemetry_exporter_ostream_span,opentelemetry_exporter_ostream_logs]
executable('otellogs', 
cpp_source_files,
dependencies: opentelemetry_dep
)
