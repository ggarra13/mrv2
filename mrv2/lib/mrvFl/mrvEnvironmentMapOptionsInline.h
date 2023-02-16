// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

namespace mrv {
inline bool
EnvironmentMapOptions::operator==(const EnvironmentMapOptions &b) const {
  return (type == b.type && horizontalAperture == b.horizontalAperture &&
          verticalAperture == b.verticalAperture &&
          focalLength == b.focalLength && rotateX == b.rotateX &&
          rotateY == b.rotateY && subdivisionX == b.subdivisionX &&
          subdivisionY == b.subdivisionY && spin == b.spin);
}

inline bool
EnvironmentMapOptions::operator!=(const EnvironmentMapOptions &b) const {
  return !(*this == b);
}
} // namespace mrv
