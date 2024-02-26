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
#include "web_client.h"

namespace sdk
{
namespace resource
{
class Resource;
}  // namespace resource
}  // namespace sdk

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
  explicit OtelMetricExporter(std::ostream &sout = std::cout,
                                 opentelemetry::sdk::metrics::AggregationTemporality aggregation_temporality =
                                 opentelemetry::sdk::metrics::AggregationTemporality::kCumulative) noexcept
                                     : sout_(sout), aggregation_temporality_(aggregation_temporality)
                                     {}

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
    sout_ << "ResourceMetrics: " << std::endl;
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
  std::ostream &sout_;
  bool is_shutdown_ = false;
  opentelemetry::sdk::metrics::AggregationTemporality aggregation_temporality_;
  bool isShutdown() const noexcept
  {
    return is_shutdown_;
  }
};

}  // namespace exporter

