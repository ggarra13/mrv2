// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cmath>
#include <cstdio>
#include <cinttypes>

#include <opentime/rationalTime.h>
namespace otime = opentime::OPENTIME_VERSION;

#include <tlCore/Path.h>
#include <tlCore/Context.h>

#include "mrvCore/mrvFile.h"

namespace mrv
{

    /**
     * Given a frame string like "0020", return the number of
     * padded digits
     *
     * @param frame a string like "0020" or "14".
     *
     * @return number of padded digits (4 for 0020, 1 for 14 ).
     */
    int padded_digits(const std::string& frame);

    /**
     * Utility function to print a float value with 8 digits
     *
     * @param x float number
     *
     * @return a new 9 character buffer
     */
    inline const char* float_printf(char* buf, float x) noexcept
    {
        if (std::isnan(x))
        {
            return "   NAN  ";
        }
        else if (!std::isfinite(x))
        {
            return "  INF.  ";
        }
        else
        {
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
    inline const char* hex_printf(char* buf, float x) noexcept
    {
        if (std::isnan(x))
        {
            return "        ";
        }
        else
        {
            unsigned h = 0;
            if (x > 0.0f)
                h = unsigned(x * 255.0f);
            snprintf(buf, 24, " %7x", h);
            return buf + strlen(buf) - 8;
        }
    }

    /**
     * Utility function to print a float as a decimal value [0-255] with 8
     * digits
     *
     * @param x float number
     *
     * @return a new 9 character buffer
     */
    inline const char* dec_printf(char* buf, float x) noexcept
    {
        if (std::isnan(x))
        {
            return "        ";
        }
        else
        {
            unsigned h = 0;
            if (x > 0.0f)
                h = unsigned(x * 255.0f);
            snprintf(buf, 24, " %7d", h);
            return buf + strlen(buf) - 8;
        }
    }

    //! Create a basename+number+extension from a path and a time
    inline std::string createStringFromPathAndTime(
        const tl::file::Path& path, const otime::RationalTime& time) noexcept
    {
        const auto& name = path.getBaseName();
        int64_t frame = time.to_frames();
        const auto& num = path.getNumber();
        const auto& extension = path.getExtension();
        if (mrv::file::isMovie(extension))
            frame = atoi(num.c_str());

        char buf[256];
        buf[0] = 0;
        if (!num.empty())
        {
            auto padding = path.getPadding();
            snprintf(buf, 256, "%0*" PRId64, padding, frame);
        }

        return name + buf + extension;
    }

    /**
     * Return a string with all the 'match' characters commented out with
     * with backslashes.
     */
    std::string
    commentCharacter(const std::string& input, const char match = '/');

    //! Parse a directory and return all movies, sequences and audios found
    //! there
    void parse_directory(
        const std::string& directory, std::vector<std::string>& movies,
        std::vector<std::string>& sequences, std::vector<std::string>& audios);

} // namespace mrv
