// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <string>
#include <algorithm>

#include "mrvCore/mrvOS.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMedia.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPreferences.h"

#include "mrvWidgets/mrvOCIOBrowser.h"

namespace
{
    const char* kModule = "ocio";
}

namespace mrv
{

    OCIOBrowser::OCIOBrowser(int x, int y, int w, int h, const char* l) :
        Fl_Browser(x, y, w, h, l),
        _type(kNone)
    {
        type(FL_HOLD_BROWSER);
        textcolor(FL_BLACK);
        //   when( FL_WHEN_RELEASE );
    }

    OCIOBrowser::~OCIOBrowser() {}

    void OCIOBrowser::fill_view()
    {
#ifdef TLRENDER_OCIO
        OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
        const char* display = Preferences::OCIO_Display.c_str();
        std::vector< std::string > views;
        int numViews = config->getNumViews(display);
        for (int i = 0; i < numViews; i++)
        {
            std::string view = config->getView(display, i);
            views.push_back(view);
        }

        value(1);
        std::sort(views.begin(), views.end());
        for (size_t i = 0; i < views.size(); ++i)
        {
            add(views[i].c_str());
            if (views[i] == _sel)
            {
                value(i + 1);
            }
        }
#endif
    }

    void OCIOBrowser::fill_display()
    {
#ifdef TLRENDER_OCIO
        OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
        std::vector< std::string > displays;
        for (int i = 0; i < config->getNumDisplays(); ++i)
        {
            std::string display = config->getDisplay(i);
            displays.push_back(display);
        }

        value(1);
        std::sort(displays.begin(), displays.end());
        for (size_t i = 0; i < displays.size(); ++i)
        {
            add(displays[i].c_str());
            if (displays[i] == _sel)
            {
                value(i + 1);
            }
        }
#endif
    }

    void OCIOBrowser::fill_display_and_view()
    {
#ifdef TLRENDER_OCIO
        OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
        std::vector< std::string > views;
        for (int i = 0; i < config->getNumDisplays(); ++i)
        {
            const std::string& display = config->getDisplay(i);
            int numViews = config->getNumViews(display.c_str());
            for (int i = 0; i < numViews; i++)
            {
                std::string view = config->getView(display.c_str(), i);
                views.push_back(display + "/" + view);
            }
        }

        value(1);
        std::sort(views.begin(), views.end());
        for (size_t i = 0; i < views.size(); ++i)
        {
            add(views[i].c_str());
            if (views[i] == _sel)
            {
                value(i + 1);
            }
        }
#endif
    }
    void OCIOBrowser::fill_look()
    {
#ifdef TLRENDER_OCIO
        OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();

        std::vector<std::string> looks;
        int numLooks = config->getNumLooks();
        for (int i = 0; i < numLooks; ++i)
        {
            looks.push_back(config->getLookNameByIndex(i));
        }

        value(1);
        std::sort(looks.begin(), looks.end());
        for (size_t i = 0; i < looks.size(); ++i)
        {
            add(looks[i].c_str());
            if (looks[i] == _sel)
            {
                value(i + 1);
            }
        }
#endif
    }

    void OCIOBrowser::fill_input_color_space()
    {
#ifdef TLRENDER_OCIO
        OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
        std::vector< std::string > spaces;
        for (int i = 0; i < config->getNumColorSpaces(); ++i)
        {
            std::string csname = config->getColorSpaceNameByIndex(i);
            spaces.push_back(csname);
        }

        if (std::find(spaces.begin(), spaces.end(), OCIO::ROLE_SCENE_LINEAR) ==
            spaces.end())
        {
            spaces.push_back(OCIO::ROLE_SCENE_LINEAR);
        }

        std::sort(spaces.begin(), spaces.end());
        value(1);
        for (size_t i = 0; i < spaces.size(); ++i)
        {
            const char* space = spaces[i].c_str();
            OCIO::ConstColorSpaceRcPtr cs = config->getColorSpace(space);
            add(space); // was w = add( space ) @TODO: fltk1.4 impossible
            // w->tooltip( strdup( cs->getDescription() ) );
            if (spaces[i] == _sel)
            {
                value(i + 1);
            }
        }
#endif
    }

    int OCIOBrowser::handle(int event)
    {
        return Fl_Browser::handle(event);
    }

    void OCIOBrowser::fill()
    {
        this->clear();

        locale::SetAndRestore saved;

        switch (_type)
        {
        case kInputColorSpace:
            fill_input_color_space();
            break;
        case kView:
            fill_view();
            break;
        case kDisplay:
            fill_display();
            break;
        case kDisplay_And_View:
            fill_display_and_view();
            break;
        case kLook:
            fill_look();
            break;
        default:
            LOG_ERROR(_("Unknown type for mrvOCIOBrowser"));
        }
    }

} // namespace mrv
