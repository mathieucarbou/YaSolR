// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaTaskManager.h>

static const Mycila::TaskPredicate ALWAYS_TRUE = []() {
  return true;
};

////////////////
// TASK MANAGER
////////////////

Mycila::TaskManager::TaskManager(const char* name, const size_t capacity) : _name(name),
                                                                            _capacity(capacity),
                                                                            _tasks(new Task*[capacity]) {
  for (size_t i = 0; i < _capacity; i++)
    _tasks[i] = nullptr;
}
Mycila::TaskManager::~TaskManager() { delete[] _tasks; }

const char* Mycila::TaskManager::getName() const { return _name; }

size_t Mycila::TaskManager::getSize() const {
  size_t count = 0;
  for (size_t i = 0; i < _capacity; i++)
    if (_tasks[i])
      count++;
  return count;
}

void Mycila::TaskManager::loop() {
  for (size_t i = 0; i < _capacity; i++)
    if (_tasks[i] && _tasks[i]->tryRun())
      yield();
}

#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
void Mycila::TaskManager::toJson(const JsonObject& root) const {
  root["name"] = _name;
  for (size_t i = 0; i < _capacity; i++)
    if (_tasks[i])
      _tasks[i]->toJson(root["tasks"][i].to<JsonObject>());
}
#endif

void Mycila::TaskManager::_addTask(Task* task) {
  for (size_t i = 0; i < _capacity; i++)
    if (!_tasks[i]) {
      _tasks[i] = task;
      return;
    }
  assert(false); // full
}

void Mycila::TaskManager::_removeTask(Task* task) {
  for (size_t i = 0; i < _capacity; i++)
    if (_tasks[i] == task) {
      _tasks[i] = nullptr;
      return;
    }
}

////////////////
// TASK
////////////////

Mycila::Task::Task(const char* name, TaskFunction fn) : _name(name), _fn(fn) { assert(_fn); }

Mycila::Task::~Task() {
  if (_manager)
    _manager->_removeTask(this);
}

///////////////////
// task information
///////////////////

const char* Mycila::Task::getName() const { return _name; }
Mycila::TaskType Mycila::Task::getType() const { return _type; }
uint32_t Mycila::Task::getInterval() const { return _intervalSupplier ? _intervalSupplier() : 0; }
bool Mycila::Task::isEnabled() const { return _enabledPredicate && _enabledPredicate(); }
bool Mycila::Task::isRunning() const { return _running; }
bool Mycila::Task::isManaged() const { return _manager; }
bool Mycila::Task::shouldRun() const {
  if (!_enabledPredicate || !_enabledPredicate())
    return false;
  if (_lastEnd == 0 || !_intervalSupplier)
    return true;
  const uint32_t itvl = _intervalSupplier();
  return itvl == 0 || micros() - _lastEnd >= itvl;
}

///////////////////
// task creation
///////////////////

void Mycila::Task::setType(Mycila::TaskType type) { _type = type; }

void Mycila::Task::setEnabled(bool enabled) {
  if (enabled)
    _enabledPredicate = ALWAYS_TRUE;
  else
    _enabledPredicate = nullptr;
}

void Mycila::Task::setEnabledWhen(TaskPredicate predicate) { _enabledPredicate = predicate; }

void Mycila::Task::setInterval(uint32_t intervalMicros) {
  if (intervalMicros == 0)
    _intervalSupplier = nullptr;
  else
    _intervalSupplier = [intervalMicros]() {
      return intervalMicros;
    };
}

void Mycila::Task::setIntervalSupplier(TaskIntervalSupplier supplier) { _intervalSupplier = supplier; }
void Mycila::Task::setCallback(TaskDoneCallback doneCallback) { _onDone = doneCallback; }
void Mycila::Task::setManager(TaskManager* manager) {
  assert(!_manager);
  _manager = manager;
  manager->_addTask(this);
}

///////////////////
// task management
///////////////////

void Mycila::Task::setData(void* params) { _params = params; }
void Mycila::Task::pause() { _enabledPredicate = nullptr; }
void Mycila::Task::resume() { _enabledPredicate = ALWAYS_TRUE; }
bool Mycila::Task::tryRun() {
  if (!_enabledPredicate || !_enabledPredicate())
    return false;
  if (_lastEnd == 0 || !_intervalSupplier) {
    _run(micros());
    return true;
  }
  const uint32_t itvl = _intervalSupplier();
  const uint32_t now = micros();
  if (itvl == 0 || now - _lastEnd >= itvl) {
    _run(now);
    return true;
  }
  return false;
}
void Mycila::Task::forceRun() { _run(micros()); }
void Mycila::Task::requestEarlyRun() { _lastEnd = 0; }

///////////////////
// optional
///////////////////

#ifdef MYCILA_TASK_MANAGER_JSON_SUPPORT
void Mycila::Task::toJson(const JsonObject& root) const {
  root["name"] = _name;
  root["type"] = _type == TaskType::ONCE ? "ONCE" : "FOREVER";
  root["enabled"] = isEnabled();
  root["interval"] = getInterval();
#ifdef MYCILA_TASK_MANAGER_STATS_SUPPORT
  root["count"] = _iterations;
#endif
}
#endif

#ifdef MYCILA_TASK_MANAGER_STATS_SUPPORT
uint32_t Mycila::Task::getIterations() const { return _iterations; }
#endif

#ifdef MYCILA_TASK_MANAGER_DEBUG
bool Mycila::Task::isDebug() const { return _debugPredicate && _debugPredicate(); }
void Mycila::Task::setDebug(bool debug) { _debugPredicate = debug ? ALWAYS_TRUE : nullptr; }
void Mycila::Task::setDebugWhen(TaskPredicate predicate) { _debugPredicate = predicate; }
#endif

///////////////////
// private
///////////////////

void Mycila::Task::_run(const uint32_t now) {
  _running = true;
  _fn(_params);
  _running = false;
  _lastEnd = micros();
  if (_type == TaskType::ONCE)
    _enabledPredicate = nullptr;

  const uint32_t elapsed = _lastEnd - now;

#ifdef MYCILA_TASK_MANAGER_STATS_SUPPORT
  _iterations++;
#endif

#ifdef MYCILA_TASK_MANAGER_DEBUG
  if (_debugPredicate && _debugPredicate())
    ESP_LOGD("TASK", "%s ended in %u us", _name, elapsed);
#endif

  if (_onDone)
    _onDone(*this, elapsed);
}
