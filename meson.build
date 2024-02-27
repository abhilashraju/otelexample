project('otelexample', 'cpp', version: 'cpp_std=20',default_options: [
    'warning_level=3',
    'cpp_std=c++20',
    'wrap_mode=nodownload',
  ])
cxx = meson.get_compiler('cpp')
cpp_source_files = ['main.cpp','foo_library.cc']

bindir = get_option('prefix') + '/' +get_option('bindir')


boost_dep = dependency('boost',modules: ['coroutine','url'])
openssl_dep = dependency('openssl', version: '>=1.1.1')

opentelemetry_trace = cxx.find_library('opentelemetry_trace', required: true)
opentelemetry_logs =  cxx.find_library('opentelemetry_logs', required: true)
opentelemetry_exporter_ostream_span = cxx.find_library('opentelemetry_exporter_ostream_span', required: true)
opentelemetry_exporter_ostream_logs =  cxx.find_library('opentelemetry_exporter_ostream_logs', required: true)
opentelemetry_resources = cxx.find_library('opentelemetry_resources', required: true)
opentelemetry_common = cxx.find_library('opentelemetry_common', required: true)
opentelemetry_metrics = cxx.find_library('opentelemetry_metrics', required: true)
opentelemetry_exporter_ostream_metrics=cxx.find_library('opentelemetry_exporter_ostream_metrics', required: true)
opentelemetry_dep = [opentelemetry_exporter_ostream_metrics,opentelemetry_metrics,opentelemetry_common,opentelemetry_resources,opentelemetry_trace, opentelemetry_logs,opentelemetry_exporter_ostream_span,opentelemetry_exporter_ostream_logs]
opentelemetry_includes=['.','http/']
executable('otelexample', 
cpp_source_files,
dependencies: [opentelemetry_dep,boost_dep,openssl_dep],
include_directories:opentelemetry_includes,
install: true,
install_dir:bindir,
)
