project('otelexample', 'cpp', version: 'cpp_std=20',default_options: [
    'warning_level=3',
    'cpp_std=c++20',
  ])
cxx = meson.get_compiler('cpp')
cpp_source_files = ['main.cpp','foo_library.cc']
add_project_arguments('-DSSL_ON', language : 'cpp')
bindir = get_option('prefix') + '/' +get_option('bindir')

#gtest = subproject('gtest')
prometheus=subproject('prometheus')
reactor=subproject('reactor')
reactor_dep=reactor.get_variable('reactor_dep')
boost_dep = dependency('boost',modules: ['coroutine','url'])
openssl_dep = dependency('openssl', version: '>=1.1.1')
nlohmann_json_dep = dependency('nlohmann_json', version: '>=3.11.2', include_type: 'system')

opentelemetry_trace = cxx.find_library('opentelemetry_trace', required: true,dirs :'/usr/local/lib')
opentelemetry_logs =  cxx.find_library('opentelemetry_logs', required: true,dirs :'/usr/local/lib')
opentelemetry_exporter_ostream_span = cxx.find_library('opentelemetry_exporter_ostream_span', required: true,dirs :'/usr/local/lib')
opentelemetry_exporter_ostream_logs =  cxx.find_library('opentelemetry_exporter_ostream_logs', required: true,dirs :'/usr/local/lib')
opentelemetry_resources = cxx.find_library('opentelemetry_resources', required: true,dirs :'/usr/local/lib')
opentelemetry_common = cxx.find_library('opentelemetry_common', required: true,dirs :'/usr/local/lib')
opentelemetry_metrics = cxx.find_library('opentelemetry_metrics', required: true,dirs :'/usr/local/lib')
opentelemetry_exporter_ostream_metrics=cxx.find_library('opentelemetry_exporter_ostream_metrics', required: true,dirs :'/usr/local/lib')
opentelemetry_dep = [opentelemetry_exporter_ostream_metrics,opentelemetry_metrics,opentelemetry_common,opentelemetry_resources,opentelemetry_trace, opentelemetry_logs,opentelemetry_exporter_ostream_span,opentelemetry_exporter_ostream_logs]
prometheus_dep =prometheus.get_variable('prometheus_dep')
opentelemetry_includes=['.','/usr/local/include/']
executable('otelexample', 
cpp_source_files,
dependencies: [reactor_dep,opentelemetry_dep,boost_dep,openssl_dep,nlohmann_json_dep,prometheus_dep],
include_directories:opentelemetry_includes,
install: true,
install_dir:bindir,
link_with:prometheus.get_variable('prometheus_core')
)
