// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace mrv {

class ColorSchemes {
protected:
  std::string filename;
  std::string remove_comments(std::string line);

public:
  std::string name;

  struct Theme {
    Theme(std::string &n) : name(n) {}
    Theme(const Theme &t) : name(t.name), colormaps(t.colormaps) {}

    std::string name;
    typedef std::map<int, int> colorMap;
    colorMap colormaps;
  };

  typedef std::vector<Theme> themeArray;
  themeArray themes;

  ColorSchemes();
  bool read_themes(const char *file);
  bool read_colors(FILE *f, Theme &scheme);
  void apply_colors(std::string name);
  void reload_theme(std::string name);

  void debug();
};

} // namespace mrv
