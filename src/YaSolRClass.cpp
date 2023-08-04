// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <YaSolRWebsite.h>

void YaSolR::YaSolRClass::_initHTTPd() {
  Mycila::HTTPd.init(&webServer, YASOLR_ADMIN_USERNAME, Mycila::Config.get(KEY_ADMIN_PASSWORD));
}

void YaSolR::YaSolRClass::_initWebsite() {
  YaSolR::Website.init();
}

void YaSolR::YaSolRClass::updateWebsite() {
  YaSolR::Website.update();
}

namespace YaSolR {
  YaSolRClass YaSolR;
} // namespace YaSolR
