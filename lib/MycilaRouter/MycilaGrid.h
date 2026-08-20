// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <MycilaExpiringValue.h>
#include <MycilaMetrics.h>

#include <optional>
#include <utility>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class Grid : public metric::MetricSupport {
    public:
      bool isConnected() const {
        return _metrics.isPresent() && _metrics.get().voltage > 0;
      }

      std::optional<float> getPower() const {
        if (_metrics.isPresent() && !std::isnan(_metrics.get().power)) {
          return _metrics.get().power;
        }
        return std::nullopt;
      }

      std::optional<float> getVoltage() const {
        if (_metrics.isPresent() && _metrics.get().voltage > 0) {
          return _metrics.get().voltage;
        }
        return std::nullopt;
      }

      std::optional<float> getFrequency() const {
        if (_metrics.isPresent() && _metrics.get().frequency > 0) {
          return _metrics.get().frequency;
        }
        return std::nullopt;
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["online"] = isConnected();
        root["metrics"]["source"] = getSourceString();
        root["metrics"]["time"] = _metrics.getLastUpdateTime();
        if (_metrics.isPresent()) {
          metric::Metrics::toJson(root["metrics"].as<JsonObject>(), _metrics.get());
        }
      }
#endif
  };
} // namespace Mycila
