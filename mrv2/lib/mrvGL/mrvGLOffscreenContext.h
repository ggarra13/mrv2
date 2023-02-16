// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <memory>

namespace mrv {
//
// This class implements an offscreen OpenGL context
//
class OffscreenContext {
public:
  OffscreenContext();
  ~OffscreenContext();

  void init();
  void make_current();
  void release();

  void create_gl_window();

protected:
  TLRENDER_PRIVATE();
};
} // namespace mrv
