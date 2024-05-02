#pragma once
#include "opentelemetry/common/spin_lock_mutex.h"
#include "opentelemetry/sdk/metrics/data/metric_data.h"
#include "opentelemetry/sdk/metrics/export/metric_producer.h"
#include "opentelemetry/sdk/metrics/instruments.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/version.h"
#include "prometheus/text_serializer.h"

#include "client/http/http_subscriber.hpp"
#include "exporter_utils.hpp"
namespace bmctelemetry
{
namespace
{
std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Gauge& g)
{
    os << "Value: " << g.value << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Info& i)
{
    os << "Value: " << i.value << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Quantile& q)
{
    os << "Quantile: " << q.quantile << std::endl;
    os << "Value: " << q.value << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Summary& s)
{
    os << "SampleCount: " << s.sample_count << std::endl;
    os << "SampleSum: " << s.sample_sum << std::endl;
    for (auto& quantile : s.quantile)
    {
        os << quantile;
    }
    return os;
}
std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Label& l)
{
    os << "Name: " << l.name << std::endl;
    os << "Value: " << l.value << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const prometheus::ClientMetric::Counter& c)
{
    os << "Value: " << c.value << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const prometheus_client::ClientMetric& cm)
{
    os << "Label: " << std::endl;
    for (auto& label : cm.label)
    {
        os << label;
    }
    return os;
}
std::ostream& operator<<(std::ostream& os,
                         const prometheus_client::MetricFamily& mf)
{
    os << "Name: " << mf.name << std::endl;
    os << "Help: " << mf.help << std::endl;
    os << "Type: " << int(mf.type) << std::endl;
    for (auto& metric : mf.metric)
    {
        os << "Metric: " << std::endl;
        os << metric;
    }
    return os;
}
} // namespace

class PrometheusMetricExporter final :
    public opentelemetry::sdk::metrics::PushMetricExporter
{
  public:
    /**
     * Create an OtelMetricExporter. This constructor takes in a reference to an
     * ostream that the export() function will send metrics data into. The
     * default ostream is set to stdout
     */

    explicit PrometheusMetricExporter(
        const std::string& url, net::io_context::executor_type ex,
        opentelemetry::sdk::metrics::AggregationTemporality
            aggregation_temporality = opentelemetry::sdk::metrics::
                AggregationTemporality::kCumulative) noexcept :
        subscriber(ex, url),
        aggregation_temporality_(aggregation_temporality)
    {
        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_verify_mode(ssl::verify_none);
        subscriber.withSslContext(std::move(ctx));
        subscriber.withPoolSize(1);
        subscriber.withPolicy({.maxRetries = 1});
        subscriber.withSuccessHandler([](const HttpSubscriber::Request&,
                                         const HttpSubscriber::Response&) {
            // Handle the response
            std::cout << "Success: " << std::endl;
        });
    }

    /**
     * Export
     * @param data metrics data
     */
    opentelemetry::sdk::common::ExportResult
        Export(const opentelemetry::sdk::metrics::ResourceMetrics&
                   metric_data) noexcept override
    {
        if (isShutdown())
        {
            return opentelemetry::sdk::common::ExportResult::kFailure;
        }
        std::vector<prometheus_client::MetricFamily> result;
        auto prometheus_metric_data =
            PrometheusExporterUtils::TranslateToPrometheus(metric_data, false,
                                                           false);
        for (auto& data : prometheus_metric_data)
            result.emplace_back(data);

        const auto serializer = prometheus::TextSerializer{};

        subscriber.sendEvent(serializer.Serialize(result));

        return opentelemetry::sdk::common::ExportResult::kSuccess;
    }

    /**
     * Get the AggregationTemporality for ostream exporter
     *
     * @return AggregationTemporality
     */
    opentelemetry::sdk::metrics::AggregationTemporality
        GetAggregationTemporality(opentelemetry::sdk::metrics::InstrumentType
                                      instrument_type) const noexcept override
    {
        return aggregation_temporality_;
    }

    /**
     * Force flush the exporter.
     */
    bool ForceFlush(std::chrono::microseconds timeout =
                        (std::chrono::microseconds::max)()) noexcept override
    {
        return true;
    }

    /**
     * Shut down the exporter.
     * @param timeout an optional timeout.
     * @return return the status of this operation
     */
    bool Shutdown(std::chrono::microseconds timeout =
                      (std::chrono::microseconds::max)()) noexcept override
    {
        is_shutdown_ = true;
        return true;
    }

  private:
    HttpSubscriber subscriber;

    bool is_shutdown_ = false;
    opentelemetry::sdk::metrics::AggregationTemporality
        aggregation_temporality_;
    bool isShutdown() const noexcept
    {
        return is_shutdown_;
         
    }
};

} // namespace bmctelemetry
