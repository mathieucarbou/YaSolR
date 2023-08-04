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
      const String version;
      const String model;
      const String manufacturer;
      const String firmware;
      const String buildBranch;
      const String buildHash;
      const String buildDate;
      const bool debug;
      const bool trial;

    private:
      String _getEspId() const;
  };

  extern AppInfoClass AppInfo;
} // namespace Mycila
