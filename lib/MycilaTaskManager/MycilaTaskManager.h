// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <functional>
#include <vector>

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

  typedef std::function<void(void* params)> TaskFunction;
  typedef std::function<bool()> TaskEnablePredicate;
  typedef std::function<uint32_t()> TaskIntervalSupplier;

  class Task {
    public:
      Task(const char* name, TaskFunction fn, TaskType type = TaskType::FOREVER, uint32_t intervalMicros = 0, bool enabled = false) : _name(name),
                                                                                                                                      _fn(fn),
                                                                                                                                      _type(type) {
        setInterval(intervalMicros);
        setEnabled(enabled);
      }

      const char* getName() const { return _name; }
      TaskType getType() const { return _type; }
      uint32_t getInterval() const { return _intervalSupplier(); }
      bool isEnabled() const { return _enabledPredicate(); }
      bool isRunning() const { return _running; }
      bool isEarlyRunRequested() const { return _requestEarlyRun; }
      uint32_t getLastStart() const { return _lastStart; }
      uint32_t getLastEnd() const { return _lastEnd; }
      uint32_t getLastRuntime(uint32_t unit = TimeUnit::MICROSECONDS) const { return (_lastEnd - _lastStart) / unit; }
      bool shouldRun() const {
        if (_running || !_enabledPredicate())
          return false;
        const uint32_t itvl = _intervalSupplier();
        return itvl == 0 || _requestEarlyRun || micros() - _lastEnd >= itvl;
      }

      Task pause() {
        return when([]() { return false; });
      }
      Task resume() {
        return when([]() { return true; });
      }

      Task setEnabled(bool enabled) { return enabled ? resume() : pause(); }
      Task when(TaskEnablePredicate enabledPredicate) {
        _enabledPredicate = enabledPredicate;
        return *this;
      }

      Task setInterval(uint32_t intervalMillis) {
        return interval([intervalMillis]() { return intervalMillis; });
      }
      Task interval(TaskIntervalSupplier intervalSupplier) {
        _intervalSupplier = intervalSupplier;
        return *this;
      }

      Task setData(void* params) {
        _params = params;
        return *this;
      }

      Task debug() {
        _debug = true;
        return *this;
      }

      bool tryRun() {
        if (_running || !_enabledPredicate())
          return false;
        const uint32_t itvl = _intervalSupplier();
        const uint32_t now = micros();
        if (itvl != 0 && !_requestEarlyRun && now - _lastEnd < itvl)
          return false;
        _run(now);
        return true;
      }

      void requestEarlyRun() {
        _requestEarlyRun = true;
      }

    private:
      const char* _name;
      const TaskFunction _fn;
      TaskType _type;
      TaskIntervalSupplier _intervalSupplier;
      TaskEnablePredicate _enabledPredicate;
      uint32_t _lastStart = 0;
      uint32_t _lastEnd = 0;
      bool _running = false;
      bool _requestEarlyRun = false;
      void* _params = nullptr;
      bool _debug = false;

    private:
      void _run(uint32_t now) {
        _running = true;
        _fn(_params);
        _running = false;
        _requestEarlyRun = false;
        _lastStart = now;
        _lastEnd = micros();
        if (_type == TaskType::ONCE)
          pause();
        if (_debug)
          ESP_LOGD("TASK", "Finished '%s' in %u us", _name, (_lastEnd - _lastStart));
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
