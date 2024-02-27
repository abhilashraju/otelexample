// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <iostream>
#include <string>

#include "opentelemetry/common/spin_lock_mutex.h"
#include "opentelemetry/sdk/metrics/data/metric_data.h"
#include "opentelemetry/sdk/metrics/export/metric_producer.h"
#include "opentelemetry/sdk/metrics/instruments.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/version.h"
#include "http_subscriber.hpp"

namespace sdk
{
namespace resource
{
class Resource;
}  // namespace resource
}  // namespace sdk
using namespace reactor;
namespace bmctelemetry
{

/**
 * The OtelMetricExporter exports record data through an ostream
 */
class OtelMetricExporter final : public opentelemetry::sdk::metrics::PushMetricExporter
{
public:
  /**
   * Create an OtelMetricExporter. This constructor takes in a reference to an ostream that the
   * export() function will send metrics data into.
   * The default ostream is set to stdout
   */
  
  
  explicit OtelMetricExporter(const std::string& url,net::io_context::executor_type ex,
                                 opentelemetry::sdk::metrics::AggregationTemporality aggregation_temporality =
                                 opentelemetry::sdk::metrics::AggregationTemporality::kCumulative) noexcept
                                     :subscriber(ex,url),aggregation_temporality_(aggregation_temporality)
    {

        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_verify_mode(ssl::verify_none);
        subscriber.withSslContext(std::move(ctx));
        subscriber.withPoolSize(5);
        subscriber.withPolicy({.maxRetries = 1});
        subscriber.withSuccessHandler(
            [](const HttpSubscriber::Request&, const HttpSubscriber::Response&) {
            // Handle the response
            std::cout << "Success: " << std::endl;
        });
        
    }

  /**
   * Export
   * @param data metrics data
   */
  opentelemetry::sdk::common::ExportResult Export(const opentelemetry::sdk::metrics::ResourceMetrics &data) noexcept override
  {
    if (isShutdown())
    {
      return opentelemetry::sdk::common::ExportResult::kFailure;
    }
     
    subscriber.sendEvent("metrics data");
    return opentelemetry::sdk::common::ExportResult::kSuccess;
  }

  /**
   * Get the AggregationTemporality for ostream exporter
   *
   * @return AggregationTemporality
   */
  opentelemetry::sdk::metrics::AggregationTemporality GetAggregationTemporality(
      opentelemetry::sdk::metrics::InstrumentType instrument_type) const noexcept override{
    return aggregation_temporality_;
      }

  /**
   * Force flush the exporter.
   */
  bool ForceFlush(
      std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override
      {
        return true;
      }

  /**
   * Shut down the exporter.
   * @param timeout an optional timeout.
   * @return return the status of this operation
   */
  bool Shutdown(
      std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override
      {
        is_shutdown_=true;
        return true;
      
      }

private:
  
  
  HttpSubscriber subscriber;

  bool is_shutdown_ = false;
  opentelemetry::sdk::metrics::AggregationTemporality aggregation_temporality_;
  bool isShutdown() const noexcept
  {
    return is_shutdown_;
  }
};

}  // namespace exporter

