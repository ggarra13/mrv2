// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include "mrvApp/App.h"
#include "tlGL/Util.h"

int main(int argc, char *argv[]) {
  int r = 0;
  try {
    auto context = tl::system::Context::create();
    tl::gl::init(context);
    mrv::App app(argc, argv, context);
    if (0 == app.getExit()) {
      r = app.run();
    }
  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return r;
}
