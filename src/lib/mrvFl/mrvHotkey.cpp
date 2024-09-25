// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

#include <FL/Enumerations.H>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvString.h"

#include "mrvWidgets/mrvBrowser.h"

#include "mrvUI/mrvAsk.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvFileRequester.h"

#include "keyboard_ui.h"
#include "mrvHotkeyUI.h"
#include "mrViewer.h"

namespace
{
    const char* kModule = "keys";
}

namespace
{
    using mrv::Hotkey;

    std::string addHotkeys(
        const Hotkey& hotkey_less, const Hotkey& hotkey_more,
        const Hotkey& hotkey_reset)
    {
        std::string out;
        const std::string& less = hotkey_less.to_s();
        const std::string& more = hotkey_more.to_s();
        const std::string& reset = hotkey_reset.to_s();
        if (!less.empty() && !more.empty())
        {
            out = _("\nHotkeys:   ");
            out += less;
            out += _("   and   ");
            out += more;
        }
        if (!reset.empty())
        {
            out += _("\nReset:   ");
            out += reset;
        }
        return out;
    }

    std::string addHotkey(const Hotkey& hotkey)
    {
        std::string out;
        const std::string& key = hotkey.to_s();
        if (!key.empty())
        {
            out = _("\nHotkey: ");
            out += key;
        }
        return out;
    }
} // namespace

namespace mrv
{

    void fill_ui_hotkeys(mrv::Browser* b)
    {
        int r = b->vposition();
        b->type(FL_SELECT_BROWSER);
        b->clear();

        int w2 = b->w() / 4;
        int w1 = w2 * 3;
        static int widths[] = {w1 - 20, w2, 0};
        b->column_widths(widths);
        b->column_char('\t'); // tabs as column delimiters

        // Labels
        b->add(_("@B12@C7@b@.Function\t@B12@C7@b@.Hotkey"));

        for (int i = 0; hotkeys[i].hotkey; ++i)
        {
            const HotkeyEntry& h = hotkeys[i];
            const std::string hotkey = h.hotkey->to_s();

            std::string row(_(h.name.c_str()));
            row += "\t" + hotkey;

            b->add(row.c_str());
        }

        b->vposition(r);
    }

    void select_hotkey(HotkeyUI* b)
    {
        int idx =
            b->uiFunction->value() - 2; // 1 for browser offset, 1 for title
        if (idx < 0)
            return;

        const std::string& name = hotkeys[idx].name;
        Hotkey* hotkey = hotkeys[idx].hotkey;

        ChooseHotkey* h = new ChooseHotkey(hotkey);
        h->make_window(name);
        h->fill();

        Fl_Window* window = h->uiMain;
        Fl_Group::current(0);
        window->show();

        while (window->visible())
            Fl::check();

        for (int i = 0; hotkeys[i].hotkey; ++i)
        {
            if (h->hk == *(hotkeys[i].hotkey) && idx != i &&
                hotkeys[i].hotkey->to_s() != "[" &&
                hotkeys[i].hotkey->to_s() != "]")
            {
                int ok = fl_choice(
                    _("Hotkey \"%s\" already used in \"%s\".\n"
                      "Do you want to override it?"),
                    _("Yes"), _("No"), 0L, h->hk.to_s().c_str(),
                    _(hotkeys[i].name.c_str()));
                if (ok)
                {
                    delete h;
                    return select_hotkey(b);
                }
                else
                {
                    hotkeys[i].hotkey->clear();
                }
            }
        }

        *hotkey = h->hk;

        delete h;

        fill_ui_hotkeys(b->uiFunction);
    }

    void searchFunction(const std::string& searchText, mrv::Browser* o)
    {
        if (searchText.empty())
        {
            o->select(o->value(), 0);
            o->topline(0);
            o->redraw();
            return;
        }
        try
        {
            std::regex regex{searchText, std::regex_constants::icase};
            int start = o->value() + 1;
            for (int i = start; i <= o->size(); ++i)
            {
                std::string function = o->text(i);
                std::size_t pos = function.find('\t');
                function = function.substr(0, pos);
                if (std::regex_search(function, regex))
                {
                    o->topline(i);
                    o->select(i);
                    o->redraw();
                    return;
                }
            }
        }
        catch (const std::exception& e)
        {
        }
    }

    void searchHotkey(const std::string& text, mrv::Browser* o)
    {
        if (text.empty())
        {
            o->select(o->value(), 0);
            o->topline(0);
            o->redraw();
            return;
        }
        try
        {
            std::string prepareForRegEx = string::commentCharacter(text, '+');
            std::regex regex{text, std::regex_constants::icase};
            int start = o->value() + 1;
            for (int i = start; i <= o->size(); ++i)
            {
                std::string hotkey = o->text(i);
                std::size_t pos = hotkey.find('\t');
                hotkey = hotkey.substr(pos + 1, hotkey.size());
                if (std::regex_search(hotkey, regex))
                {
                    o->topline(i);
                    o->select(i);
                    o->redraw();
                    return;
                }
            }
        }
        catch (const std::exception& e)
        {
        }
    }

    void save_hotkeys(Fl_Preferences& keys)
    {
        keys.set("version", 11);
        for (int i = 0; hotkeys[i].hotkey; ++i)
        {
            keys.set(
                (hotkeys[i].name + " ctrl").c_str(), hotkeys[i].hotkey->ctrl);
            keys.set(
                (hotkeys[i].name + " alt").c_str(), hotkeys[i].hotkey->alt);
            keys.set(
                (hotkeys[i].name + " meta").c_str(), hotkeys[i].hotkey->meta);
            keys.set(
                (hotkeys[i].name + " shift").c_str(), hotkeys[i].hotkey->shift);
            keys.set(
                (hotkeys[i].name + " key").c_str(),
                (int)hotkeys[i].hotkey->key);
            keys.set(
                (hotkeys[i].name + " text").c_str(),
                hotkeys[i].hotkey->text.c_str());
        }
    }

    void load_hotkeys(Fl_Preferences* keys)
    {
        int version = 0;
        keys->get("version", version, 11);
        int tmp = 0;
        char tmpS[2048];

        for (int i = 0; hotkeys[i].hotkey; ++i)
        {
            if (hotkeys[i].force == false)
                continue;
            hotkeys[i].hotkey->shift = hotkeys[i].hotkey->ctrl =
                hotkeys[i].hotkey->alt = hotkeys[i].hotkey->meta = false;
            hotkeys[i].hotkey->key = 0;
            hotkeys[i].hotkey->text.clear();
        }

        for (int i = 0; hotkeys[i].hotkey; ++i)
        {
            Hotkey saved(*hotkeys[i].hotkey);

            keys->get(
                (hotkeys[i].name + " key").c_str(), tmp,
                (int)hotkeys[i].hotkey->key);
            if (tmp)
                hotkeys[i].force = false;
            hotkeys[i].hotkey->key = unsigned(tmp);

            keys->get(
                (hotkeys[i].name + " text").c_str(), tmpS,
                hotkeys[i].hotkey->text.c_str(), 16);
            if (strlen(tmpS) > 0)
            {
                hotkeys[i].force = false;
                hotkeys[i].hotkey->text = tmpS;
            }
            else
                hotkeys[i].hotkey->text.clear();

            if (hotkeys[i].force)
            {
                *(hotkeys[i].hotkey) = saved;
                continue;
            }
            keys->get(
                (hotkeys[i].name + " ctrl").c_str(), tmp,
                (int)hotkeys[i].hotkey->ctrl);
            if (tmp)
                hotkeys[i].hotkey->ctrl = true;
            else
                hotkeys[i].hotkey->ctrl = false;
            keys->get(
                (hotkeys[i].name + " alt").c_str(), tmp,
                (int)hotkeys[i].hotkey->alt);
            if (tmp)
                hotkeys[i].hotkey->alt = true;
            else
                hotkeys[i].hotkey->alt = false;

            keys->get(
                (hotkeys[i].name + " meta").c_str(), tmp,
                (int)hotkeys[i].hotkey->meta);
            if (tmp)
                hotkeys[i].hotkey->meta = true;
            else
                hotkeys[i].hotkey->meta = false;

            keys->get(
                (hotkeys[i].name + " shift").c_str(), tmp,
                (int)hotkeys[i].hotkey->shift);
            if (tmp)
                hotkeys[i].hotkey->shift = true;
            else
                hotkeys[i].hotkey->shift = false;

            for (int j = 0; hotkeys[j].hotkey; ++j)
            {
                if (j != i && hotkeys[j].hotkey == hotkeys[i].hotkey)
                {
                    if (hotkeys[j].force != true)
                    {
                        std::string err =
                            tl::string::Format(
                                _("Corruption in hotkeys preferences. "
                                  "Hotkey '{0}' for {1} will not be "
                                  "available.  "
                                  "Already used in {2}."))
                                .arg(hotkeys[j].hotkey->to_s())
                                .arg(_(hotkeys[j].name.c_str()))
                                .arg(_(hotkeys[i].name.c_str()));
                        LOG_ERROR(err);
                    }
                    hotkeys[j].hotkey->ctrl = false;
                    hotkeys[j].hotkey->shift = false;
                    hotkeys[j].hotkey->meta = false;
                    hotkeys[j].hotkey->alt = false;
                    hotkeys[j].hotkey->text.clear();
                    hotkeys[j].hotkey->key = 0;
                }
            }
        }
    }

    void load_hotkeys()
    {
        Fl_Preferences* keys = new Fl_Preferences(
            prefspath().c_str(), "filmaura", Preferences::hotkeys_file.c_str(),
            (Fl_Preferences::Root)0);
        load_hotkeys(keys);
        delete keys;
    }

    void update_hotkey_tooltips()
    {
        ViewerUI* ui = App::ui;

        std::string tooltip;

        tooltip = _("Allows you to select different image channels or layers.");
        tooltip += addHotkeys(kPreviousChannel, kNextChannel, Hotkey());
        ui->uiColorChannel->copy_tooltip(tooltip.c_str());

        tooltip = _("Reduce gain 1/4 stop (divide by sqrt(sqrt(2))).");
        tooltip += addHotkey(kExposureLess);
        ui->uiExposureLess->copy_tooltip(tooltip.c_str());

        tooltip = _("Reduce gain 1/4 stop (divide by sqrt(sqrt(2))).");
        tooltip += addHotkey(kExposureLess);
        ui->uiExposureLess->copy_tooltip(tooltip.c_str());

        tooltip = _("Increase gain 1/4 stop (multiply by sqrt(sqrt(2))).");
        tooltip += addHotkey(kExposureMore);
        ui->uiExposureMore->copy_tooltip(tooltip.c_str());

        tooltip = _("Allows you to adjust the gain or exposure of the image.");
        tooltip += addHotkeys(kExposureLess, kExposureMore, kResetChanges);
        ui->uiGain->copy_tooltip(tooltip.c_str());
        ui->uiGainInput->copy_tooltip(tooltip.c_str());

        tooltip = _("Allows you to adjust the saturation of the image.");
        tooltip += addHotkeys(kSaturationLess, kSaturationMore, kResetChanges);
        ui->uiSaturation->copy_tooltip(tooltip.c_str());
        ui->uiSaturationInput->copy_tooltip(tooltip.c_str());

        tooltip = _("Allows you to adjust gamma curve for display.\nValue is:  "
                    "pow( 2, 1/x ).");
        tooltip += addHotkeys(kGammaLess, kGammaMore, kResetChanges);
        ui->uiGamma->copy_tooltip(tooltip.c_str());
        ui->uiGammaInput->copy_tooltip(tooltip.c_str());
        
        tooltip = _("Toggle Edit Mode.");
        tooltip += addHotkey(kToggleEditMode);
        ui->uiEdit->copy_tooltip(tooltip.c_str());

        //
        // Action tools.
        //
        tooltip = _("Scrubbing Tool");
        tooltip += addHotkey(kScrubMode);
        ui->uiScrub->copy_tooltip(tooltip.c_str());

        tooltip = _("Area Select Tool");
        tooltip += addHotkey(kAreaMode);
        ui->uiSelection->copy_tooltip(tooltip.c_str());

        tooltip = _("Freehand Drawing Tool");
        tooltip += addHotkey(kDrawMode);
        ui->uiDraw->copy_tooltip(tooltip.c_str());

        tooltip = _("Eraser Tool");
        tooltip += addHotkey(kEraseMode);
        ui->uiErase->copy_tooltip(tooltip.c_str());
        
        tooltip = _("Polygon Tool");
        tooltip += addHotkey(kPolygonMode);
        ui->uiPolygon->copy_tooltip(tooltip.c_str());

        tooltip = _("Circle Tool");
        tooltip += addHotkey(kCircleMode);
        ui->uiCircle->copy_tooltip(tooltip.c_str());

        tooltip = _("Rectangle Tool");
        tooltip += addHotkey(kRectangleMode);
        ui->uiRectangle->copy_tooltip(tooltip.c_str());

        tooltip = _("Arrow Tool");
        tooltip += addHotkey(kArrowMode);
        ui->uiArrow->copy_tooltip(tooltip.c_str());

        tooltip = _("Text Tool.   "
                    "Right click to edit previously stamped text.");
        tooltip += addHotkey(kTextMode);
        ui->uiText->copy_tooltip(tooltip.c_str());
        
        tooltip = _("Undo Draw");
        tooltip += addHotkey(kUndoDraw);
        ui->uiUndoDraw->copy_tooltip(tooltip.c_str());
        
        tooltip = _("Redo Draw");
        tooltip += addHotkey(kRedoDraw);
        ui->uiRedoDraw->copy_tooltip(tooltip.c_str());

        // Timebar Window Toolbar
        TimelineClass* c = ui->uiTimeWindow;

        tooltip = _("Go to the beginning of the sequence.");
        tooltip += addHotkey(kFirstFrame);
        c->uiPlayStart->copy_tooltip(tooltip.c_str());

        tooltip = _("Play sequence backwards.");
        tooltip += addHotkey(kPlayBack);
        c->uiPlayBackwards->copy_tooltip(tooltip.c_str());

        tooltip = _("Go back one frame.");
        tooltip += addHotkey(kFrameStepBack);
        c->uiStepBackwards->copy_tooltip(tooltip.c_str());

        tooltip = _("Stop playback.");
        tooltip += addHotkey(kStop);
        c->uiStop->copy_tooltip(tooltip.c_str());

        tooltip = _("Go back one frame.");
        tooltip += addHotkey(kFrameStepFwd);
        c->uiStepForwards->copy_tooltip(tooltip.c_str());

        tooltip = _("Play sequence forwards.");
        tooltip += addHotkey(kPlayFwd);
        c->uiPlayForwards->copy_tooltip(tooltip.c_str());

        tooltip = _("Go to the end of the sequence.");
        tooltip += addHotkey(kLastFrame);
        c->uiPlayEnd->copy_tooltip(tooltip.c_str());

        tooltip = _("Set In Point");
        tooltip += addHotkey(kSetInPoint);
        c->uiStartButton->copy_tooltip(tooltip.c_str());

        tooltip = _("Set Out Point");
        tooltip += addHotkey(kSetOutPoint);
        c->uiEndButton->copy_tooltip(tooltip.c_str());
        
        //
        // Pixel Tool Bar.
        //
        PixelToolBarClass* p = ui->uiPixelWindow;
        tooltip = _("Image zoom setting.");
        tooltip += addHotkeys(kZoomOut, kZoomIn, kFitScreen);
        tooltip += _("\n\nMousewheel to zoom in and out.");
        tooltip += _("\nAlt + Right Mouse Button to zoom in and out.");
        tooltip += _("\n\n0 to 9 to zoom to a specific level.");
        p->uiZoom->copy_tooltip(tooltip.c_str());
    }

} // namespace mrv
