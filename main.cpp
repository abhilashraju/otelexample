// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "foo_library.h"

using namespace bmctelemetry;
using namespace reactor;
int main()
{
    OtelLogger::globalInstance();
    OtelTracer::globalInstance();

    fooFunc();
    net::io_context ioContext;
    auto& metric =
        OtelMetrics::OtelMetricsBuilder::globalInstance()
            .withUrl("http://127.0.0.1:9091/metrics/job/sample_client")
            .withContext(ioContext)
            .getMetrics();
    std::string version{"1.2.0"};
    std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
    metric.addCounterView("my_metric", version, schema);
    metric.addObservableCounterView("my_metric", version, schema);
    metric.addHistogramView("my_metric", version, schema);
    foo_library::counter_example("my_metric");
    foo_library::observable_counter_example("my_metric");
    foo_library::histogram_example("my_metric");

    ioContext.run();
}
