// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Table.H>

namespace mrv {

class Table : public Fl_Table {
public:
  Table(int x, int y, int w, int h, const char *l = 0);
  virtual ~Table();

  int handle(int event) override;
  void resize(int X, int Y, int W, int H) override;
  void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0,
                 int Y = 0, int W = 0, int H = 0) override;

  inline void column_labels(const char **h) { headers = h; }
  inline void column_separator(bool t = true) { _column_separator = t; }
  inline void auto_resize(bool t = true) { _auto_resize = t; }

  void add(Fl_Widget *w);
  void layout();

protected:
  void DrawHeader(const char *s, int X, int Y, int W, int H);

  const char **headers;
  bool _column_separator;
  bool _auto_resize;
};

} // namespace mrv
