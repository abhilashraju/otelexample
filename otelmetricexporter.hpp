// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "opentelemetry/common/spin_lock_mutex.h"
#include "opentelemetry/sdk/metrics/data/metric_data.h"
#include "opentelemetry/sdk/metrics/export/metric_producer.h"
#include "opentelemetry/sdk/metrics/instruments.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/version.h"

#include "client/http/http_subscriber.hpp"
#include "common_utils.hpp"

#include <iostream>
#include <string>

using namespace reactor;
using namespace opentelemetry;
namespace bmctelemetry
{
inline std::string
    timeToString(opentelemetry::common::SystemTimestamp time_stamp)
{
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(time_stamp);

    struct tm* tm_ptr = nullptr;
#if defined(_MSC_VER)
    struct tm buf_tm;
    if (!gmtime_s(&buf_tm, &epoch_time))
    {
        tm_ptr = &buf_tm;
    }
#else
    tm_ptr = std::gmtime(&epoch_time);
#endif

    char buf[100];
    char* date_str = nullptr;
    if (tm_ptr == nullptr)
    {
        OTEL_INTERNAL_LOG_ERROR("[OStream Metric] gmtime failed for "
                                << epoch_time);
    }
    else if (std::strftime(buf, sizeof(buf), "%c", tm_ptr) > 0)
    {
        date_str = buf;
    }
    else
    {
        OTEL_INTERNAL_LOG_ERROR("[OStream Metric] strftime failed for "
                                << epoch_time);
    }

    return std::string{date_str};
}
template <typename Container>
inline void printVec(std::ostream& os, Container& vec)
{
    using T = typename std::decay<decltype(*vec.begin())>::type;
    os << '[';
    if (vec.size() > 1)
    {
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, ", "));
    }
    os << ']';
}
inline void
    printPointData(std::ostream& sout_,
                   const opentelemetry::sdk::metrics::PointType& point_data)
{
    if (opentelemetry::nostd::holds_alternative<sdk::metrics::SumPointData>(
            point_data))
    {
        auto sum_point_data =
            nostd::get<sdk::metrics::SumPointData>(point_data);
        sout_ << "\n  type\t\t: SumPointData";
        sout_ << "\n  value\t\t: ";
        if (opentelemetry::nostd::holds_alternative<double>(
                sum_point_data.value_))
        {
            sout_ << opentelemetry::nostd::get<double>(sum_point_data.value_);
        }
        else if (opentelemetry::nostd::holds_alternative<int64_t>(
                     sum_point_data.value_))
        {
            sout_ << opentelemetry::nostd::get<int64_t>(sum_point_data.value_);
        }
    }
    else if (opentelemetry::nostd::holds_alternative<
                 sdk::metrics::HistogramPointData>(point_data))
    {
        auto histogram_point_data =
            opentelemetry::nostd::get<sdk::metrics::HistogramPointData>(
                point_data);
        sout_ << "\n  type     : HistogramPointData";
        sout_ << "\n  count     : " << histogram_point_data.count_;
        sout_ << "\n  sum     : ";
        if (opentelemetry::nostd::holds_alternative<double>(
                histogram_point_data.sum_))
        {
            sout_ << opentelemetry::nostd::get<double>(
                histogram_point_data.sum_);
        }
        else if (opentelemetry::nostd::holds_alternative<int64_t>(
                     histogram_point_data.sum_))
        {
            sout_ << opentelemetry::nostd::get<int64_t>(
                histogram_point_data.sum_);
        }

        if (histogram_point_data.record_min_max_)
        {
            if (opentelemetry::nostd::holds_alternative<int64_t>(
                    histogram_point_data.min_))
            {
                sout_ << "\n  min     : "
                      << opentelemetry::nostd::get<int64_t>(
                             histogram_point_data.min_);
            }
            else if (opentelemetry::nostd::holds_alternative<double>(
                         histogram_point_data.min_))
            {
                sout_ << "\n  min     : "
                      << opentelemetry::nostd::get<double>(
                             histogram_point_data.min_);
            }
            if (opentelemetry::nostd::holds_alternative<int64_t>(
                    histogram_point_data.max_))
            {
                sout_ << "\n  max     : "
                      << opentelemetry::nostd::get<int64_t>(
                             histogram_point_data.max_);
            }
            if (opentelemetry::nostd::holds_alternative<double>(
                    histogram_point_data.max_))
            {
                sout_ << "\n  max     : "
                      << opentelemetry::nostd::get<double>(
                             histogram_point_data.max_);
            }
        }

        sout_ << "\n  buckets     : ";
        printVec(sout_, histogram_point_data.boundaries_);

        sout_ << "\n  counts     : ";
        printVec(sout_, histogram_point_data.counts_);
    }
    else if (opentelemetry::nostd::holds_alternative<
                 sdk::metrics::LastValuePointData>(point_data))
    {
        auto last_point_data =
            opentelemetry::nostd::get<sdk::metrics::LastValuePointData>(
                point_data);
        sout_ << "\n  type     : LastValuePointData";
        sout_ << "\n  timestamp     : "
              << std::to_string(
                     last_point_data.sample_ts_.time_since_epoch().count())
              << std::boolalpha
              << "\n  valid     : " << last_point_data.is_lastvalue_valid_;
        sout_ << "\n  value     : ";
        if (opentelemetry::nostd::holds_alternative<double>(
                last_point_data.value_))
        {
            sout_ << opentelemetry::nostd::get<double>(last_point_data.value_);
        }
        else if (opentelemetry::nostd::holds_alternative<int64_t>(
                     last_point_data.value_))
        {
            sout_ << opentelemetry::nostd::get<int64_t>(last_point_data.value_);
        }
    }
}
inline void printPointAttributes(
    std::ostream& sout_,
    const opentelemetry::sdk::metrics::PointAttributes& point_attributes)
{
    sout_ << "\n  attributes\t\t: ";
    for (const auto& kv : point_attributes)
    {
        sout_ << "\n\t" << kv.first << ": ";
        bmctelemetry::print_value(kv.second, sout_);
    }
}
inline void printAttributes(
    std::ostream& sout_,
    const std::map<std::string, sdk::common::OwnedAttributeValue>& map,
    const std::string prefix)
{
    for (const auto& kv : map)
    {
        sout_ << prefix << kv.first << ": ";
        bmctelemetry::print_value(kv.second, sout_);
    }
}
inline void
    printResources(std::ostream& sout_,
                   const opentelemetry::sdk::resource::Resource& resources)
{
    auto attributes = resources.GetAttributes();
    if (attributes.size())
    {
        // Convert unordered_map to map for printing so that iteration
        // order is guaranteed.
        std::map<std::string, opentelemetry::sdk::common::OwnedAttributeValue>
            attr_map;
        for (auto& kv : attributes)
            attr_map[kv.first] = std::move(kv.second);
        printAttributes(sout_, attr_map, "\n\t");
    }
}

inline void printInstrumentationInfoMetricData(
    std::ostream& sout_,
    const opentelemetry::sdk::metrics::ScopeMetrics& info_metric,
    const opentelemetry::sdk::metrics::ResourceMetrics& data)
{
    // sout_ is shared
    sout_ << "{";
    sout_ << "\n  scope name\t: " << info_metric.scope_->GetName()
          << "\n  schema url\t: " << info_metric.scope_->GetSchemaURL()
          << "\n  version\t: " << info_metric.scope_->GetVersion();
    for (const auto& record : info_metric.metric_data_)
    {
        sout_ << "\n  start time\t: " << timeToString(record.start_ts)
              << "\n  end time\t: " << timeToString(record.end_ts)
              << "\n  instrument name\t: " << record.instrument_descriptor.name_
              << "\n  description\t: "
              << record.instrument_descriptor.description_
              << "\n  unit\t\t: " << record.instrument_descriptor.unit_;

        for (const auto& pd : record.point_data_attr_)
        {
            if (!opentelemetry::nostd::holds_alternative<
                    opentelemetry::sdk::metrics::DropPointData>(pd.point_data))
            {
                printPointData(sout_, pd.point_data);
                printPointAttributes(sout_, pd.attributes);
            }
        }

        sout_ << "\n  resources\t:";
        printResources(sout_, *data.resource_);
    }
    sout_ << "\n}\n";
}

/**
 * The OtelMetricExporter exports record data through an ostream
 */
class OtelMetricExporter final :
    public opentelemetry::sdk::metrics::PushMetricExporter
{
  public:
    /**
     * Create an OtelMetricExporter. This constructor takes in a reference to an
     * ostream that the export() function will send metrics data into. The
     * default ostream is set to stdout
     */

    explicit OtelMetricExporter(
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
                   data) noexcept override
    {
        if (isShutdown())
        {
            return opentelemetry::sdk::common::ExportResult::kFailure;
        }
        std::stringstream sout_;
        for (auto& record : data.scope_metric_data_)
        {
            printInstrumentationInfoMetricData(sout_, record, data);
        }
        subscriber.sendEvent(sout_.str());
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
