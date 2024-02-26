// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0


# include "foo_library.h"

using namespace bmctelemetry;
using namespace reactor;
int main()
{
  OtelLogger::globalInstance();
  OtelTracer::globalInstance();

  fooFunc();
  auto& metric=OtelMetrics::globalInstance();
  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
  metric.addCounterView("my_metric", version, schema);
  metric.addObservableCounterView("my_metric", version, schema);
  metric.addHistogramView("my_metric", version, schema);
  foo_library::counter_example("my_metric");
  foo_library::observable_counter_example("my_metric");
  foo_library::histogram_example("my_metric");
  net::io_context ioc;
  auto ex = net::make_strand(ioc);
  http::string_body::value_type body = "test value";

  auto flux = WebClient<AsyncTcpStream, http::string_body>::builder()
                  .withSession(ex)
                  .withEndpoint("https://127.0.0.1:8081/testpost")
                  .create()
                  .post()
                  .withContentType(ContentType{"plain/text"})
                  .withBody(std::move(body))
                  .toFlux();
  std::vector<std::string> actual;
  flux->subscribe([&actual, i = 0](auto v, auto reqNext) mutable {
      if (!v.isError())
      {
          actual.push_back(v.response().body());
          reqNext(i++ < 2);
          return;
      }
      reqNext(false);
  });
  ioc.run();
}