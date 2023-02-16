// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvDraw/Shape.h>

#include <memory>
#include <vector>

namespace tl {
namespace draw {

class Annotation {
public:
  Annotation(const int64_t frame, const bool allFrames);
  ~Annotation();

  bool allFrames() const;

  int64_t frame() const;

  bool empty() const;

  void push_back(const std::shared_ptr<Shape> &);

  const std::vector<std::shared_ptr<Shape>> &shapes() const;
  const std::vector<std::shared_ptr<Shape>> &undo_shapes() const;

  std::shared_ptr<Shape> lastShape() const;

  void undo();

  void redo();

protected:
  TLRENDER_PRIVATE();
};

} // namespace draw

} // namespace tl
