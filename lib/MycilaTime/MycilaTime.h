// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <WString.h>
#include <time.h>

namespace Mycila {
  class Time {
    public:
      static String toISO8601Str(time_t unixTime) {
        if (unixTime == 0)
          return emptyString;
        char buffer[21];
        strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&unixTime));
        return buffer;
      }

      static String toLocalStr(time_t unixTime) {
        if (unixTime == 0)
          return emptyString;
        struct tm timeInfo;
        localtime_r(&unixTime, &timeInfo);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%F %T", &timeInfo);
        return buffer;
      }

      static time_t getUnixTime() {
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 5))
          return 0;
        time(&now);
        return now;
      }

      static String getISO8601Str() { return toISO8601Str(getUnixTime()); }

      static String getLocalStr() { return toLocalStr(getUnixTime()); }

      static int toMinutes(const String& time, char sep = ':') {
        int i = time.indexOf(sep);
        if (i < 0)
          return i;
        return time.substring(0, i).toInt() * 60 + time.substring(i + 1).toInt();
      }

      static int timeInRange(const struct tm& timeInfo, const String& start, const String& end, char sep = ':') {
        const int startMinutes = toMinutes(start, sep);
        if (startMinutes == -1)
          return -1;

        const int stopMinutes = toMinutes(end, sep);
        if (stopMinutes == -1)
          return -1;

        if (startMinutes == stopMinutes)
          return false; // range is empty

        const uint16_t timeMinutes = timeInfo.tm_hour * 60 + timeInfo.tm_min;

        // cases:
        // startMinutes < stopMinutes : i.e. 06:00 -> 22:00
        // startMinutes > stopMinutes  : i.e. 22:00 -> 06:00
        return (startMinutes < stopMinutes && timeMinutes >= startMinutes && timeMinutes < stopMinutes) || (startMinutes > stopMinutes && (timeMinutes >= startMinutes || timeMinutes < stopMinutes));
      }

      static const String toDHHMMSS(uint32_t seconds) {
        const uint8_t days = seconds / 86400;
        seconds = seconds % (uint32_t)86400;
        const uint8_t hh = seconds / 3600;
        seconds = seconds % (uint32_t)3600;
        const uint8_t mm = seconds / 60;
        const uint8_t ss = seconds % (uint32_t)60;
        char buffer[14];
        snprintf(buffer, sizeof(buffer), "%" PRIu8 "d %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8, days % 1000, hh % 100, mm % 100, ss % 100);
        return buffer;
      }
  };

} // namespace Mycila
