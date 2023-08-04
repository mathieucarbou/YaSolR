// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <functional>
#include <vector>

#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
#include <ArduinoJson.h>
#endif

#define MYCILA_TASK_MANAGER_VERSION "1.0.0"
#define MYCILA_TASK_MANAGER_VERSION_MAJOR 1
#define MYCILA_TASK_MANAGER_VERSION_MINOR 0
#define MYCILA_TASK_MANAGER_VERSION_REVISION 0

namespace Mycila {
  namespace TimeUnit {
    constexpr uint32_t MICROSECONDS = 1UL;
    constexpr uint32_t MILLISECONDS = 1000UL;
    constexpr uint32_t SECONDS = 1000000UL;
    constexpr uint32_t MINUTES = 60 * SECONDS;
    constexpr uint32_t HOURS = 60 * MINUTES;
    constexpr uint32_t DAYS = 24 * HOURS;
  } // namespace TimeUnit

  enum class TaskType {
    // once enabled, the task will run once and then disable itself
    ONCE,
    // the task will run at the specified interval as long as it is enabled
    FOREVER
  };

  class Task;

  typedef std::function<void(void* params)> TaskFunction;
  typedef std::function<bool()> TaskPredicate;
  typedef std::function<uint32_t()> TaskIntervalSupplier;
  typedef std::function<void(const Task& me)> TaskDoneCallback;

  class Task {
    public:
      Task(const char* name, TaskFunction fn, TaskType type = TaskType::FOREVER, uint32_t intervalMicros = 0, bool enabled = false) : _name(name),
                                                                                                                                      _fn(fn),
                                                                                                                                      _type(type) {
        assert(_fn);
        setInterval(intervalMicros);
        setEnabled(enabled);
      }

      const char* getName() const { return _name; }
      TaskType getType() const { return _type; }
      uint32_t getInterval() const { return _intervalSupplier ? _intervalSupplier() : 0; }
      bool isEnabled() const { return _enabledPredicate && _enabledPredicate(); }
      bool isRunning() const { return _running; }
      bool isEarlyRunRequested() const { return _requestEarlyRun; }
      bool isDebug() const { return _debugPredicate && _debugPredicate(); }
      uint32_t getLastStart() const { return _lastStart; }
      uint32_t getLastEnd() const { return _lastEnd; }
      uint32_t getLastRuntime(uint32_t unit = TimeUnit::MICROSECONDS) const { return _lastRuntime / unit; }
      bool shouldRun() const {
        if (!isEnabled())
          return false;
        const uint32_t itvl = getInterval();
        return itvl == 0 || isEarlyRunRequested() || micros() - getLastEnd() >= itvl;
      }

      Task& setEnabled(bool enabled) { return enabled ? resume() : pause(); }
      Task& pause() {
        _enabledPredicate = nullptr;
        return *this;
      }
      Task& resume() {
        _enabledPredicate = []() {
          return true;
        };
        return *this;
      }
      Task& when(TaskPredicate predicate) {
        _enabledPredicate = predicate;
        return *this;
      }

      Task& setInterval(uint32_t intervalMicros) {
        if (intervalMicros == 0) {
          _intervalSupplier = nullptr;
        }
        _intervalSupplier = [intervalMicros]() {
          return intervalMicros;
        };
        return *this;
      }
      Task& interval(TaskIntervalSupplier supplier) {
        _intervalSupplier = supplier;
        return *this;
      }

      Task& setData(void* params) {
        _params = params;
        return *this;
      }

      Task& debug() {
        _debugPredicate = []() {
          return true;
        };
        return *this;
      }
      Task& debugIf(TaskPredicate predicate) {
        _debugPredicate = predicate;
        return *this;
      }

      bool tryRun() {
        if (!isEnabled())
          return false;
        const uint32_t itvl = getInterval();
        const uint32_t now = micros();
        if (itvl != 0 && !isEarlyRunRequested() && now - getLastEnd() < itvl)
          return false;
        _run(now);
        return true;
      }

      Task& forceRun() {
        _run(micros());
        return *this;
      }

      Task& requestEarlyRun() {
        _requestEarlyRun = true;
        return *this;
      }

      Task& onDone(TaskDoneCallback doneCallback) {
        _onDone = doneCallback;
        return *this;
      }

#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["debug"] = isDebug();
        root["earlyRunRequested"] = isEarlyRunRequested();
        root["enabled"] = isEnabled();
        root["interval"] = getInterval();
        root["lastEnd"] = getLastEnd();
        root["lastRuntime"] = getLastRuntime();
        root["lastStart"] = getLastStart();
        root["name"] = getName();
        root["running"] = isRunning();
        root["type"] = getType() == TaskType::ONCE ? "ONCE" : "FOREVER";
      }
#endif

    private:
      const char* _name;
      const TaskFunction _fn;
      bool _requestEarlyRun = false;
      bool _running = false;
      TaskDoneCallback _onDone = nullptr;
      TaskIntervalSupplier _intervalSupplier = nullptr;
      TaskPredicate _debugPredicate = nullptr;
      TaskPredicate _enabledPredicate = nullptr;
      TaskType _type;
      uint32_t _lastEnd = 0;
      uint32_t _lastRuntime = 0;
      uint32_t _lastStart = 0;
      void* _params = nullptr;

    private:
      void _run(uint32_t now) {
        _lastStart = now;
        _running = true;
        _fn(_params);
        _running = false;
        _requestEarlyRun = false;
        const uint32_t end = micros();
        _lastEnd = end;
        _lastRuntime = end - now;
        if (_type == TaskType::ONCE)
          pause();
        if (isDebug())
          ESP_LOGD("TASK", "Finished '%s' in %u us", _name, (_lastEnd - _lastStart));
        if (_onDone)
          _onDone(*this);
      }
  };

  class TaskManagerClass {
    public:
      void loop();

    private:
      std::vector<Mycila::Task> _tasks;
  };

  extern TaskManagerClass TaskManager;
} // namespace Mycila
