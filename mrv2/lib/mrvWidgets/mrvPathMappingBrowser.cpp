// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/names.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include "mrvCore/mrvString.h"

#include "mrvFl/mrvPathMapping.h"

#include "mrvWidgets/mrvPathMappingBrowser.h"

#include "PathMappingUI.h"

namespace mrv
{
    int PathMappingBrowser::handle(int e)
    {
        int ret = Browser::handle(e);

        switch (e)
        {
        case FL_ENTER:
        case FL_LEAVE:
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_PUSH:
        {
            if (Fl::event_clicks() > 0)
            {
                Fl::event_clicks(0);
                int idx = value();
                if (idx >= 2)
                {
                    change_path_mapping(this, idx);
                    ret = 1;
                }
            }
        }
        case FL_KEYUP:
        {
            int idx = value();
            if (idx < 2)
                return ret;

            std::string orig = text(idx);

            int key = Fl::event_key();
            switch (key)
            {
            case FL_Down:
            {
                if (idx == size())
                    return ret;
                std::string newline = text(idx + 1);
                replace(idx, newline.c_str());
                replace(idx + 1, orig.c_str());
                select(idx + 1);
                return 1;
                break;
            }
            case FL_Up:
            {
                if (idx == 2)
                    return ret;
                std::string newline = text(idx - 1);
                replace(idx, newline.c_str());
                replace(idx - 1, orig.c_str());
                select(idx - 1);
                return 1;
                break;
            }
            }
        }
        }
        return ret;
    }
} // namespace mrv
