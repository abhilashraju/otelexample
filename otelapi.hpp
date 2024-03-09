#include "opentelemetry/exporters/ostream/log_record_exporter.h"
#include "opentelemetry/exporters/ostream/metric_exporter_factory.h"
#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/processor.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/meter_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/view_factory.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/sdk/version/version.h"
#include "opentelemetry/trace/provider.h"

#include "otelmetricexporter.hpp"
#include "prometheusexporter.hpp"
namespace bmctelemetry
{
namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;
namespace logs_api = opentelemetry::logs;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs_exporter = opentelemetry::exporter::logs;
namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_exporter = opentelemetry::exporter::trace;
namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace common = opentelemetry::common;
namespace exportermetrics = opentelemetry::exporter::metrics;
namespace metrics_api = opentelemetry::metrics;
inline nostd::shared_ptr<trace::Tracer> get_tracer()
{
    auto provider = trace::Provider::GetTracerProvider();
    return provider->GetTracer(libraryname, OPENTELEMETRY_SDK_VERSION);
}
struct OtelLogger
{
    OtelLogger()
    {
        auto exporter = std::unique_ptr<logs_sdk::LogRecordExporter>(
            new logs_exporter::OStreamLogRecordExporter);
        auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(
            std::move(exporter));
        std::shared_ptr<logs_api::LoggerProvider> provider(
            logs_sdk::LoggerProviderFactory::Create(std::move(processor)));

        // Set the global logger provider
        logs_api::Provider::SetLoggerProvider(provider);
    }
    ~OtelLogger()
    {
        std::shared_ptr<logs_api::LoggerProvider> none;
        logs_api::Provider::SetLoggerProvider(none);
    }
    static OtelLogger& globalInstance()
    {
        static OtelLogger instance;
        return instance;
    }
};
struct OtelTracer
{
    OtelTracer()
    {
        // Create ostream span exporter instance
        auto exporter = trace_exporter::OStreamSpanExporterFactory::Create();
        auto processor =
            trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
        std::shared_ptr<trace_api::TracerProvider> provider =
            trace_sdk::TracerProviderFactory::Create(std::move(processor));

        // Set the global trace provider
        trace_api::Provider::SetTracerProvider(provider);
    }
    ~OtelTracer()
    {
        std::shared_ptr<trace_api::TracerProvider> none;
        trace_api::Provider::SetTracerProvider(none);
    }
    static OtelTracer& globalInstance()
    {
        static OtelTracer instance;
        return instance;
    }
};

struct OtelMetrics
{
    struct OtelMetricsBuilder
    {
        std::string url_;
        net::io_context* context{nullptr};
        OtelMetricsBuilder& withContext(net::io_context& c)
        {
            context = &c;
            return *this;
        }
        OtelMetricsBuilder& withUrl(const std::string& url)
        {
            url_ = url;
            return *this;
        }

        OtelMetrics& getMetrics()
        {
            static OtelMetrics metrics(url_, context->get_executor());
            return metrics;
        }
        static OtelMetricsBuilder& globalInstance()
        {
            static OtelMetricsBuilder builder;
            return builder;
        }
    };

    metrics_sdk::MeterProvider* p{nullptr};
    OtelMetrics(const std::string& uri, net::io_context::executor_type ex)
    {
        auto exporter = std::make_unique<PrometheusMetricExporter>(uri, ex);

        // Initialize and set the global MeterProvider
        metrics_sdk::PeriodicExportingMetricReaderOptions options;
        options.export_interval_millis = std::chrono::milliseconds(5000);
        options.export_timeout_millis = std::chrono::milliseconds(500);

        auto reader = metrics_sdk::PeriodicExportingMetricReaderFactory::Create(
            std::move(exporter), options);

        auto u_provider = metrics_sdk::MeterProviderFactory::Create();
        p = static_cast<metrics_sdk::MeterProvider*>(u_provider.get());

        p->AddMetricReader(std::move(reader));

        std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(
            std::move(u_provider));
        metrics_api::Provider::SetMeterProvider(provider);
    }
    void addCounterView(const std::string& name, const std::string& version,
                        const std::string& schema)
    {
        std::string counter_name = name + "_counter";
        std::string unit = "counter-unit";

        auto instrument_selector =
            metrics_sdk::InstrumentSelectorFactory::Create(
                metrics_sdk::InstrumentType::kCounter, counter_name, unit);

        auto meter_selector =
            metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

        auto sum_view = metrics_sdk::ViewFactory::Create(
            name, "description", unit, metrics_sdk::AggregationType::kSum);

        p->AddView(std::move(instrument_selector), std::move(meter_selector),
                   std::move(sum_view));
    }
    void addObservableCounterView(const std::string& name,
                                  const std::string& version,
                                  const std::string& schema)
    {
        std::string unit = "observable-counter-unit";
        // observable counter view
        std::string observable_counter_name = name + "_observable_counter";

        auto observable_instrument_selector =
            metrics_sdk::InstrumentSelectorFactory::Create(
                metrics_sdk::InstrumentType::kObservableCounter,
                observable_counter_name, unit);

        auto observable_meter_selector =
            metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

        auto observable_sum_view = metrics_sdk::ViewFactory::Create(
            name, "test_description", unit, metrics_sdk::AggregationType::kSum);

        p->AddView(std::move(observable_instrument_selector),
                   std::move(observable_meter_selector),
                   std::move(observable_sum_view));
    }
    void addHistogramView(const std::string& name, const std::string& version,
                          const std::string& schema)
    {
        std::string unit = "histogram-unit";
        std::string histogram_name = name + "_histogram";
        auto histogram_instrument_selector =
            metrics_sdk::InstrumentSelectorFactory::Create(
                metrics_sdk::InstrumentType::kHistogram, histogram_name, unit);

        auto histogram_meter_selector =
            metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

        auto histogram_aggregation_config =
            std::unique_ptr<metrics_sdk::HistogramAggregationConfig>(
                new metrics_sdk::HistogramAggregationConfig);

        histogram_aggregation_config->boundaries_ =
            std::vector<double>{0.0,    50.0,   100.0,  250.0,   500.0,  750.0,
                                1000.0, 2500.0, 5000.0, 10000.0, 20000.0};

        std::shared_ptr<metrics_sdk::AggregationConfig> aggregation_config(
            std::move(histogram_aggregation_config));

        auto histogram_view = metrics_sdk::ViewFactory::Create(
            name, "description", unit, metrics_sdk::AggregationType::kHistogram,
            aggregation_config);

        p->AddView(std::move(histogram_instrument_selector),
                   std::move(histogram_meter_selector),
                   std::move(histogram_view));
    }
    auto createDoubleCounter(const std::string& name)
    {
        nostd::shared_ptr<metrics_api::Meter> meter = p->GetMeter(name,
                                                                  "1.2.0");
        return meter->CreateDoubleCounter(name);
    }
    auto createDoubleObservableCounter(const std::string& name)
    {
        nostd::shared_ptr<metrics_api::Meter> meter = p->GetMeter(name,
                                                                  "1.2.0");
        return meter->CreateDoubleObservableCounter(name);
    }
    auto createDoubleHistogram(const std::string& name,
                               const std::string& description,
                               const std::string& unit)
    {
        nostd::shared_ptr<metrics_api::Meter> meter = p->GetMeter(name,
                                                                  "1.2.0");
        return meter->CreateDoubleHistogram(name, description, unit);
    }
    ~OtelMetrics()
    {
        std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
        metrics_api::Provider::SetMeterProvider(none);
        p = nullptr;
    }
};
} // namespace bmctelemetry

#define TRACE_FUNCION                                                          \
    auto func_span = bmctelemetry::trace::Scope(                               \
        bmctelemetry::get_tracer()->StartSpan(__FUNCTION__));
#define START_TRACE(X)                                                         \
    auto X =                                                                   \
        bmctelemetry::trace::Scope(bmctelemetry::get_tracer()->StartSpan(#X));
