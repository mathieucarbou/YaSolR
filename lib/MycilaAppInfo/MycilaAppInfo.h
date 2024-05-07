// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>

namespace Mycila {
  class AppInfoClass {
    public:
      AppInfoClass();

      void toJson(const JsonObject& root) const;

    public:
      const String id;
      const String name;
      const String model;
      const String version;
      const String nameModel;
      const String nameModelVersion;
      const String manufacturer;
      const String firmware;
      const String buildBranch;
      const String buildHash;
      const String buildDate;
      const String defaultHostname;
      const String defaultMqttClientId;
      const String defaultSSID;
      const bool debug;
      const bool trial;

    private:
      static String _getEspId();
      static String _lower(const String& s);
  };

  extern AppInfoClass AppInfo;
} // namespace Mycila
