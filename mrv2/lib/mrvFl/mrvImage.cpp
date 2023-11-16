// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvI8N.h"

#include "mrViewer.h"

namespace mrv
{
    namespace image
    {

        std::string ocioView()
        {
            auto uiOCIOView = App::ui->OCIOView;
            int idx = uiOCIOView->value();
            if (idx < 0 || idx >= uiOCIOView->children())
                return "";

            const Fl_Menu_Item* item = uiOCIOView->child(idx);

            char pathname[1024];
            int ret = uiOCIOView->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return "";
            return pathname;
            ;
        }

        void setOcioView(const std::string& name)
        {
            auto uiOCIOView = App::ui->OCIOView;
            int value = -1;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname)
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Display/View '{0}'."))
                        .arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiOCIOView->value(value);
            uiOCIOView->do_callback();
            uiOCIOView->redraw();
        }
    } // namespace image
} // namespace mrv
