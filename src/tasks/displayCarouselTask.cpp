// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task displayCarouselTask("Carousel", [](void* params) {
  void* data = displayTask.getData();
  if (data == nullptr) {
    displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
  } else {
    switch ((DisplayKind) reinterpret_cast<int>(data)) {
      case DisplayKind::DISPLAY_HOME:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_NETWORK));
        break;
      case DisplayKind::DISPLAY_NETWORK:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_ROUTER));
        break;
      case DisplayKind::DISPLAY_ROUTER:
        if (config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
          displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_OUTPUT1));
          break;
        } else {
          [[fallthrough]];
        }
      case DisplayKind::DISPLAY_OUTPUT1:
        if (config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
          displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_OUTPUT2));
          break;
        } else {
          [[fallthrough]];
        }
      case DisplayKind::DISPLAY_OUTPUT2:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
        break;
      default:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
        break;
    }
  }
});
