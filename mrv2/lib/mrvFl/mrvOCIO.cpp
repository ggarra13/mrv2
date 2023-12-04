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
        std::string ocioIcs()
        {
            auto uiICS = App::ui->uiICS;
            int idx = uiICS->value();
            if (idx < 0 || idx >= uiICS->children())
                return "";

            const Fl_Menu_Item* item = uiICS->child(idx);
            if (!item || !item->label() || item->flags & FL_SUBMENU)
                return "";

            char pathname[1024];
            int ret = uiICS->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return "";

            return pathname;
        }

        void setOcioIcs(const std::string& name)
        {
            auto uiICS = App::ui->uiICS;
            int value = -1;
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiICS->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname || name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Ics '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiICS->value(value);
            uiICS->do_callback();
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }

        std::string ocioLook()
        {
            auto OCIOLook = App::ui->OCIOLook;
            int idx = OCIOLook->value();
            if (idx < 0 || idx >= OCIOLook->children())
                return "";

            const Fl_Menu_Item* item = OCIOLook->child(idx);
            return item->label();
        }

        void setOcioLook(const std::string& name)
        {
            auto OCIOLook = App::ui->OCIOLook;
            int value = -1;
            for (int i = 0; i < OCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = OCIOLook->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = OCIOLook->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname || name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Look '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            OCIOLook->value(value);
            OCIOLook->do_callback();
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }

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
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }
    } // namespace image
} // namespace mrv
