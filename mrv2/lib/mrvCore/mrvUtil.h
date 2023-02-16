// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <opentime/rationalTime.h>
#include <tlCore/Path.h>

#include <cinttypes>
#include <cmath>
#include <cstdio>
namespace otime = opentime::OPENTIME_VERSION;

#include "mrvI8N.h"

namespace mrv {

/**
 * Given a filename with %hex characters, return string in ascii.
 */
std::string hex_to_char_filename(std::string &f);

/**
 * Given a filename extension, return whether the extension is
 * from a movie format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible movie, false if not.
 */
bool is_valid_movie(const char *ext);

/**
 * Given a filename extension, return whether the extension is
 * from an audio format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible audio file, false if not.
 */
bool is_valid_audio(const char *ext);

/**
 * Given a filename extension, return whether the extension is
 * from a subtitle format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible subtitle file, false if not.
 */
bool is_valid_subtitle(const char *ext);

/**
 * Given a filename extension, return whether the extension is
 * from a picture format.
 *
 * @param ext Filename extension
 *
 * @return true if a possible picture, false if not.
 */
bool is_valid_picture(const char *ext);

/**
 * Given a single image filename, return whether the image is
 * a sequence on disk (ie. there are several images named with a
 * similar convention)
 *
 * @param file Filename of image
 *
 * @return true if a possible sequence, false if not.
 */
bool is_valid_sequence(const char *file);

/**
 * Given a single filename, return whether the file is
 * a directory on disk
 *
 * @param file Filename or Directory
 *
 * @return true if a directory, false if not.
 */
bool is_directory(const char *file);

/**
 * Given a frame string like "0020", return the number of
 * padded digits
 *
 * @param frame a string like "0020" or "14".
 *
 * @return number of padded digits (4 for 0020, 1 for 14 ).
 */
int padded_digits(const std::string &frame);

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
inline const char *float_printf(char *buf, float x) noexcept {
  if (std::isnan(x)) {
    return "   NAN  ";
  } else if (!std::isfinite(x)) {
    return "  INF.  ";
  } else {
    snprintf(buf, 24, " %7.4f", x);
    return buf + strlen(buf) - 8;
  }
}

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
inline const char *hex_printf(char *buf, float x) noexcept {
  if (std::isnan(x)) {
    return "        ";
  } else {
    unsigned h = 0;
    if (x > 0.0f)
      h = unsigned(x * 255.0f);
    snprintf(buf, 24, " %7x", h);
    return buf + strlen(buf) - 8;
  }
}

/**
 * Utility function to print a float as a decimal value [0-255] with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
inline const char *dec_printf(char *buf, float x) noexcept {
  if (std::isnan(x)) {
    return "        ";
  } else {
    unsigned h = 0;
    if (x > 0.0f)
      h = unsigned(x * 255.0f);
    snprintf(buf, 24, " %7d", h);
    return buf + strlen(buf) - 8;
  }
}

//! Create a basename+number+extension from a path and a time
inline std::string
createStringFromPathAndTime(const tl::file::Path &path,
                            const otime::RationalTime &time) noexcept {
  const auto &name = path.getBaseName();
  int64_t frame = time.to_frames();
  const auto &num = path.getNumber();
  const auto &extension = path.getExtension();
  if (mrv::is_valid_movie(extension.c_str()))
    frame = atoi(num.c_str());

  char buf[256];
  buf[0] = 0;
  if (!num.empty()) {
    auto padding = path.getPadding();
    snprintf(buf, 256, "%0*" PRId64, padding, frame);
  }

  return name + buf + extension;
}
} // namespace mrv
