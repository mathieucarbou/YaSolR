// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <functional>

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
  typedef std::function<void(const Task& me, const uint32_t elapsed)> TaskDoneCallback;

  ///////////////////
  // TaskManager
  ///////////////////

  class TaskManager {
    public:
      explicit TaskManager(const char* name, const size_t capacity = 64);
      ~TaskManager();

      const char* getName() const;

      // number of tasks
      size_t getSize() const;

      // must be called from main loop and will loop over all registered tasks
      void loop();

      // json output of the task manager
#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
      void toJson(const JsonObject& root) const;
#endif

    private:
      const char* _name;
      const size_t _capacity;
      Task** _tasks = nullptr;

    private:
      void _addTask(Task* task);
      void _removeTask(Task* task);

    private:
      friend class Task;
  };

  ///////////////////
  // Task
  ///////////////////

  class Task {
    public:
      Task(const char* name, TaskFunction fn);
      ~Task();

      ///////////////////
      // task information
      ///////////////////

      const char* getName() const;
      TaskType getType() const;
      uint32_t getInterval() const;
      bool isEnabled() const;
      bool isRunning() const;
      bool isEarlyRunRequested() const;
      bool shouldRun() const;
      bool isManaged() const;

      ///////////////////
      // task creation
      ///////////////////

      // change task type. Once will run once and then disable itself. Forever will run at the specified interval
      void setType(TaskType type);

      // change the enabled state
      void setEnabled(bool enabled);

      // enable the task if the predicate returns true
      void setEnabledWhen(TaskPredicate predicate);

      // change the interval of execution
      void setInterval(uint32_t intervalMicros);

      // dynamically provide the interval
      void setIntervalSupplier(TaskIntervalSupplier supplier);

      // callback when the task is done
      void setCallback(TaskDoneCallback doneCallback);

      // have this task managed by a task manager
      void setManager(TaskManager* manager);

      ///////////////////
      // task management
      ///////////////////

      // pass some data to the task
      void setData(void* params);

      // pause a task
      void pause();

      // resume a paused task
      void resume();

      // try to run the task if it should run
      bool tryRun();

      // force the task to run
      void forceRun();

      // request an early run of the task and do not wait for the interval to be reached
      void requestEarlyRun();

      ///////////////////
      // optional
      ///////////////////

#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
      // json output
      void toJson(const JsonObject& root) const;
#endif

#ifdef MYCILA_TASK_MANAGER_STATS_SUPPORT
      uint32_t getIterations() const;
#endif

#ifdef MYCILA_TASK_MANAGER_DEBUG
      // debug
      bool isDebug() const;
      // activate some debug features, like output the task elapsed time at the end
      void setDebug(bool debug);
      // activate some debug features, like output the task elapsed time at the end if the predicate returns true
      void setDebugWhen(TaskPredicate predicate);
#endif

      ///////////////////
      // private
      ///////////////////

    private:
      const char* _name;
      const TaskFunction _fn;

      TaskType _type = TaskType::FOREVER;
      TaskManager* _manager = nullptr;
      TaskPredicate _enabledPredicate = nullptr;
      TaskIntervalSupplier _intervalSupplier = nullptr;
      TaskDoneCallback _onDone = nullptr;
      bool _running = false;
      uint32_t _lastEnd = 0;
      void* _params = nullptr;

      void _run(const uint32_t now);

#ifdef MYCILA_TASK_MANAGER_STATS_SUPPORT
      uint32_t _iterations = 0;
      void _updateStats(const uint32_t now);
#endif

#ifdef MYCILA_TASK_MANAGER_DEBUG
      TaskPredicate _debugPredicate = nullptr;
#endif
  };
} // namespace Mycila
