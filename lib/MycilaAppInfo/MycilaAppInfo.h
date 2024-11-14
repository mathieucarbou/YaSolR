// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <ArduinoJson.h>

#include <string>

namespace Mycila {
  class AppInfoClass {
    public:
      AppInfoClass();

      void toJson(const JsonObject& root) const;

    public:
      const std::string id;
      const std::string name;
      const std::string model;
      const std::string version;
      const std::string nameModel;
      const std::string nameModelVersion;
      const std::string manufacturer;
      const std::string firmware;
      const std::string buildBranch;
      const std::string buildHash;
      const std::string buildDate;
      const std::string defaultHostname;
      const std::string defaultMqttClientId;
      const std::string defaultSSID;
      const bool debug;
      const bool trial;
  };

  extern AppInfoClass AppInfo;
} // namespace Mycila
