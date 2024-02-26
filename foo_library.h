// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once
constexpr const char* libraryname = "mylibrary";
#include "otelapi.hpp"
void fooFunc();
class foo_library
{
public:
  static void counter_example(const std::string &name);
  static void histogram_example(const std::string &name);
  static void observable_counter_example(const std::string &name);
};