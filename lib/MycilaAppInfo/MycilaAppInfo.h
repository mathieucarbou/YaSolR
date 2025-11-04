// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <ArduinoJson.h>

#include <string>

namespace Mycila {
  class AppInfoClass {
    public:
      AppInfoClass();

      void toJson(const JsonObject& root) const;

      bool isOutdated() const { return !latestVersion.empty() && latestVersion != version; }

    public:
      const std::string id;
      const std::string name;
      const std::string model;
      const std::string version;
      const std::string nameModel;
      const std::string nameModelVersion;
      const std::string manufacturer;
      const std::string buildEnv;
      const std::string buildBranch;
      const std::string buildHash;
      const std::string buildDate;
      const std::string buildBoard;
      const std::string defaultHostname;
      const std::string defaultMqttClientId;
      const bool debug;
      const bool trial;
      std::string latestVersion = "";
  };

  extern AppInfoClass AppInfo;
} // namespace Mycila
