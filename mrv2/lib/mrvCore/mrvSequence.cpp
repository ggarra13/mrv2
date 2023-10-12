// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/fl_utf8.h>

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "seq";
}

namespace mrv
{

    bool is_valid_frame(const std::string& framespec)
    {
        if (framespec.size() > 17)
            return false;

        const char* c = framespec.c_str();
        if (*c == '.')
            ++c;

        for (; *c != 0; ++c)
        {
            if (*c == '+' || *c == '-' || (*c >= '0' && *c <= '9'))
                continue;

            return false;
        }

        return true;
    }

    bool is_valid_frame_spec(std::string& framespec)
    {
        const char* c;
        if (framespec.substr(0, 1) == ".")
            c = framespec.c_str() + 1;
        else
            c = framespec.c_str();

        if (framespec.size() > 17)
            return false;

        if (*c == '%')
        {
            bool d = false;
            bool l = false;
            for (++c; *c != 0; ++c)
            {
                if ((*c >= '0' && *c <= '9'))
                    continue;
                if (!d && *c == 'd')
                {
                    d = true;
                    continue;
                }
                if (!l && *c == 'l')
                {
                    l = true;
                    continue;
                }
                return false;
            }
            return true;
        }
        else if (*c == '#' || *c == '@')
        {
            char t = *c;
            for (++c; *c != 0; ++c)
            {
                if (*c != t)
                    return false;
            }
            return true;
        }

        int idx = 0;
        bool range_found = false;
        for (++c; *c != 0; ++c)
        {
            if (!range_found)
            {
                ++idx;
                if (*c == '-')
                {
                    range_found = true;
                    continue;
                }
            }
            if (*c != '+' && *c >= '0' && *c <= '9')
                continue;

            return false;
        }

        framespec = framespec.substr(0, idx);

        return range_found;
    }

    std::string get_short_view(bool left)
    {
        const char* pairs = fl_getenv("MRV_STEREO_CHAR_PAIRS");
        if (!pairs)
            pairs = "L:R";

        std::string view = pairs;
        size_t idx = view.find(':');
        if (idx == std::string::npos)
        {
            // LOG_ERROR( "MRV_STEREO_CHAR_PAIRS does not have two letters
            // separated by colon" );
            if (left)
                return "L";
            else
                return "R";
        }

        if (left)
            return view.substr(0, idx);
        else
            return view.substr(idx + 1, view.size());
    }

    std::string get_long_view(bool left)
    {
        const char* pairs = fl_getenv("MRV_STEREO_NAME_PAIRS");
        if (!pairs)
            pairs = "left:right";

        std::string view = pairs;
        size_t idx = view.find(':');
        if (idx == std::string::npos)
        {
            LOG_ERROR("MRV_STEREO_NAME_PAIRS does not have two names separated "
                      "by colon");
            if (left)
                return "left";
            else
                return "right";
        }

        if (left)
            return view.substr(0, idx);
        else
            return view.substr(idx + 1, view.size());
    }

    bool replace_view(std::string& view)
    {
        if (view.empty())
            return false;

        if (view.substr(view.size() - 1, view.size()) == ".")
            view = view.substr(0, view.size() - 1);

        if (view == "%v" || view == get_short_view(true) ||
            view == get_short_view(false))
        {
            view = "%v";
            return true;
        }

        if (view == "%V" || view == get_long_view(true) ||
            view == get_long_view(false))
        {
            view = "%V";
            return true;
        }
        return false;
    }

    bool is_valid_view(std::string view)
    {
        return replace_view(view);
    }

    std::string parse_view(const std::string& root, bool left)
    {
        size_t idx = root.find("%V");
        std::string tmp = root;
        if (idx != std::string::npos)
        {
            tmp = root.substr(0, idx);
            tmp += get_long_view(left);
            tmp += root.substr(idx + 2, root.size());
        }
        else
        {
            idx = root.find("%v");
            if (idx != std::string::npos)
            {
                tmp = root.substr(0, idx);
                tmp += get_short_view(left);
                tmp += root.substr(idx + 2, root.size());
            }
        }
        return tmp;
    }

} // namespace mrv
