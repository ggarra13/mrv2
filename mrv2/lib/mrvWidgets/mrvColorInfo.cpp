// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath> // for std::isnan, std::isfinite

#include <FL/Fl_Menu.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>

#include "mrViewer.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvString.h"
#include "mrvGL/mrvGLViewport.h"
#include "mrvColorInfo.h"

#include "mrvFl/mrvIO.h"

namespace
{

    void copy_color_cb(void*, mrv::Browser* w)
    {
        if (w->value() < 2 || w->value() > 11)
            return;

        if (w->text(w->value()) == NULL)
            return;

        size_t last;
        std::string line(w->text(w->value()));
        size_t start = line.find('\t', 0);
        line = line.substr(start + 1, line.size() - 1);
        while ((start = line.find('@', 0)) != std::string::npos)
        {
            last = line.find('c', start);
            if (last == std::string::npos)
                break;

            if (start > 0)
                line =
                    (line.substr(0, start - 1) +
                     line.substr(last + 1, line.size() - 1));
            else
                line = line.substr(last + 1, line.size() - 1);
        }

        std::string copy = " ";
        last = 0;
        while ((start = line.find_first_not_of("\t ", last)) !=
               std::string::npos)
        {
            last = line.find_first_of("\t ", start);
            if (last == std::string::npos)
                last = line.size();

            copy += line.substr(start, last - start) + " ";
        }

        // Copy text to both the clipboard and to X's XA_PRIMARY
        Fl::copy(copy.c_str(), unsigned(copy.size()), true);
        Fl::copy(copy.c_str(), unsigned(copy.size()), false);
    }

} // namespace

namespace mrv
{
    ViewerUI* ColorInfo::ui = NULL;

    class ColorBrowser : public mrv::Browser
    {
        int _value;
        ViewerUI* ui;

    public:
        ColorBrowser(int X, int Y, int W, int H, const char* L = 0) :
            mrv::Browser(X, Y, W, H, L),
            _value(-1)
        {
        }

        void main(ViewerUI* v) { ui = v; }

        void resize(int X, int Y, int W, int H)
        {
            mrv::Browser::resize(X, Y, W, H);
            int* widths = column_widths();
            int WL, WH;
            fl_font(FL_HELVETICA, 14);
            fl_measure(_("Maximum:"), WL, WH);
            WL += 8;
            int width = W - WL;
            int w5 = width / 4;
            widths[0] = WL;
            for (int i = 1; i < 5; ++i)
                widths[i] = w5;
            redraw();
        }

        int mousePush(int X, int Y)
        {
            if (value() < 0)
                return 0;

            Fl_Menu_Button menu(X, Y, 0, 0);

            menu.add(
                _("Copy/Color"), FL_COMMAND + 'C', (Fl_Callback*)copy_color_cb,
                (void*)this, 0);

            menu.popup();
            return 1;
        }

        bool valid_value()
        {
            int line = value();
            if ((line < 2 || line > 11) || (line > 5 && line < 8))
            {
                value(-1);
                return false;
            }
            _value = line;
            return true;
        }

        void draw()
        {
            value(_value);
            mrv::Browser::draw();
        }

        int handle(int event)
        {
            int ok = 0;
            switch (event)
            {
            case FL_PUSH:
            {
                if (Fl::event_button() == 3)
                    return mousePush(Fl::event_x(), Fl::event_y());
                ok = Fl_Browser::handle(event);
                if (valid_value())
                    return 1;
                return ok;
            }
            case FL_ENTER:
                return 1;
            case FL_FOCUS:
                return 1;
            default:
                ok = Fl_Browser::handle(event);
                if (valid_value())
                    return 1;
                return ok;
            }
        }
    };

    class ColorWidget : public Fl_Box
    {
        Fl_Browser* color_browser_;

    public:
        ColorWidget(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Box(X, Y, W, H, L)
        {
            box(FL_FRAME_BOX);
        }

        int mousePush(int X, int Y)
        {
            color_browser_->value(4);

            Fl_Menu_Button menu(X, Y, 0, 0);

            menu.add(
                _("Copy/Color"), FL_COMMAND + 'C', (Fl_Callback*)copy_color_cb,
                (void*)color_browser_, 0);

            menu.popup();
            return 1;
        }

        void color_browser(Fl_Browser* b) { color_browser_ = b; }

        int handle(int event)
        {
            switch (event)
            {
            case FL_PUSH:
                if (Fl::event_button() == FL_RIGHT_MOUSE)
                    return mousePush(Fl::event_x(), Fl::event_y());
            default:
                return Fl_Box::handle(event);
            }
        }
    };

    ColorInfo::ColorInfo(int X, int Y, int W, int h, const char* l) :
        Fl_Group(X, Y, W, h, l)
    {
        tooltip(_("Mark an area in the image with SHIFT + the left mouse "
                  "button"));

        Fl_Group* g = new Fl_Group(X + 8, Y + 10, 32, 32);
        dcol = new ColorWidget(X + 8, Y + 10, 32, 32);
        g->resizable(0);
        g->end();

        area = new Fl_Box(X + 40, Y, W - 40, 50);
        area->box(FL_FLAT_BOX);
        area->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        area->labelsize(12);

        int WL, WH;
        fl_font(FL_HELVETICA, 14);
        fl_measure(_("Maximum:"), WL, WH);
        WL += 8;
        int width = W - WL;
        int w5 = width / 4;
        int* col_widths = new int[6];
        col_widths[0] = WL;
        for (int i = 1; i < 5; ++i)
            col_widths[i] = w5;
        col_widths[5] = 0;
        browser = new ColorBrowser(X, Y + 10 + area->h(), W, h - area->h());
        browser->column_widths(col_widths);
        browser->showcolsep(1);
        browser->type(FL_HOLD_BROWSER);
        browser->resizable(browser);
        resizable(this);

        dcol->color_browser(browser);
    }

    void ColorInfo::main(ViewerUI* m)
    {
        ui = m;
        browser->main(m);
    }

    int ColorInfo::handle(int event)
    {
        return Fl_Group::handle(event);
    }

    void ColorInfo::update(const area::Info& info)
    {

        if (info.box.min.x == info.box.max.x ||
            info.box.min.y == info.box.max.y)
        {
            area->copy_label("");
            area->redraw();
            browser->clear();
            browser->redraw();
            return;
        }

        unsigned numPixels = info.box.w() * info.box.h();

        std::ostringstream text;
        text << std::endl
             << _("Area") << ": (" << info.box.min.x << ", " << info.box.min.y
             << ") - (" << info.box.max.x << ", " << info.box.max.y << ")"
             << std::endl
             << _("Size") << ": [ " << info.box.w() << "x" << info.box.h()
             << " ] = " << numPixels << " "
             << (numPixels == 1 ? _("pixel") : _("pixels")) << std::endl;
        area->copy_label(text.str().c_str());

        PixelToolBarClass* c = ui->uiPixelWindow;
        mrv::BrightnessType brightness_type =
            (mrv::BrightnessType)c->uiLType->value();

        static const char* kR = "@C4286611456@c";
        static const char* kG = "@C1623228416@c";
        static const char* kB = "@C2155937536@c";
        static const char* kA = "@C2964369408@c";

        static const char* kH = "@C2964324352@c";
        static const char* kS = "@C2964324352@c";
        static const char* kV = "@C2964324352@c";
        static const char* kL = "@C2964324352@c";

        Fl_Color col;
        {
            float r = info.rgba.mean.r;
            float g = info.rgba.mean.g;
            float b = info.rgba.mean.b;

            if (r > 1.F)
                r = 1.F;
            if (g > 1.F)
                g = 1.F;
            if (b > 1.F)
                b = 1.F;

            if (r < 0.F)
                r = 0.F;
            if (g < 0.F)
                g = 0.F;
            if (b < 0.F)
                b = 0.F;

            if (r <= 0.01f && g <= 0.01f && b <= 0.01f)
                col = FL_BLACK;
            else
            {
                col = fl_rgb_color(
                    (uchar)(r * 255), (uchar)(g * 255), (uchar)(b * 255));
            }
        }

        dcol->color(col);
        dcol->redraw();

        text.str("");
        text.str().reserve(1024);
        const char* locale = setlocale(LC_NUMERIC, NULL);
        text.imbue(std::locale(locale));
        text << "@b\t" << std::fixed << std::setw(7) << std::setprecision(2)
             << kR << "R"
             << "\t" << kG << "G"
             << "\t" << kB << "B"
             << "\t" << kA << "A"
             << "\n"
             << _("Maximum") << ":\t@c" << info.rgba.max.r << "\t@c"
             << info.rgba.max.g << "\t@c" << info.rgba.max.b << "\t@c"
             << info.rgba.max.a << "\n"
             << _("Minimum") << ":\t@c" << info.rgba.min.r << "\t@c"
             << info.rgba.min.g << "\t@c" << info.rgba.min.b << "\t@c"
             << info.rgba.min.a << "\n";

        text << _("Range") << ":\t@c" << info.rgba.diff.r << "\t@c"
             << info.rgba.diff.g << "\t@c" << info.rgba.diff.b << "\t@c"
             << info.rgba.diff.a << "\n"
             << "@b" << _("Mean") << ":\t@c" << kR << info.rgba.mean.r << "\t@c"
             << kG << info.rgba.mean.g << "\t@c" << kB << info.rgba.mean.b
             << "\t@c" << kA << info.rgba.mean.a << "\n"
             << "\n"
             << "@b\t";

        switch (c->uiBColorType->value() + 1)
        {
        case color::kITU_709:
            text << kH << "7"
                 << "\t@c" << kS << "0"
                 << "\t@c" << kL << "9";
            break;
        case color::kITU_601:
            text << kH << "6"
                 << "\t@c" << kS << "0"
                 << "\t@c" << kL << "1";
            break;
        case color::kYIQ:
            text << kH << "Y"
                 << "\t@c" << kS << "I"
                 << "\t@c" << kL << "Q";
            break;
        case color::kYDbDr:
            text << kH << "Y"
                 << "\t@c" << kS << "Db"
                 << "\t@c" << kL << "Dr";
            break;
        case color::kYUV:
            text << kH << "Y"
                 << "\t@c" << kS << "U"
                 << "\t@c" << kL << "V";
            break;
        case color::kCIE_Luv:
            text << kH << "L"
                 << "\t@c" << kS << "u"
                 << "\t@c" << kL << "v";
            break;
        case color::kCIE_Lab:
            text << kH << "L"
                 << "\t@c" << kS << "a"
                 << "\t@c" << kL << "b";
            break;
        case color::kCIE_xyY:
            text << kH << "x"
                 << "\t@c" << kS << "y"
                 << "\t@c" << kL << "Y";
            break;
        case color::kCIE_XYZ:
            text << kH << "X"
                 << "\t@c" << kS << "Y"
                 << "\t@c" << kL << "Z";
            break;
        case color::kHSL:
            text << kH << "H"
                 << "\t@c" << kS << "S"
                 << "\t@c" << kL << "L";
            break;
        case color::kHSV:
        default:
            text << kH << "H"
                 << "\t@c" << kS << "S"
                 << "\t@c" << kV << "V";
            break;
        }

        text << "\t" << kL;

        switch (brightness_type)
        {
        case kAsLuminance:
            text << "Y";
            break;
        case kAsLumma:
            text << "Y'";
            break;
        case kAsLightness:
            text << "L";
            break;
        }

        text << "\n"
             << _("Maximum") << ":\t@c" << info.hsv.max.r << "\t@c"
             << info.hsv.max.g << "\t@c" << info.hsv.max.b << "\t@c"
             << info.hsv.max.a << "\n"
             << _("Minimum") << ":\t@c" << info.hsv.min.r << "\t@c"
             << info.hsv.min.g << "\t@c" << info.hsv.min.b << "\t@c"
             << info.hsv.min.a << "\n"
             << _("Range") << ":\t@c" << info.hsv.diff.r << "\t@c"
             << info.hsv.diff.g << "\t@c" << info.hsv.diff.b << "\t@c"
             << info.hsv.diff.a << "\n"
             << "@b" << _("Mean") << ":\t@c" << kH << info.hsv.mean.r << "\t@c"
             << kS << info.hsv.mean.g << "\t@c" << kV << info.hsv.mean.b
             << "\t@c" << kL << info.hsv.mean.a << "\n";

        stringArray lines;
        mrv::split_string(lines, text.str(), "\n");
        stringArray::iterator i = lines.begin();
        stringArray::iterator e = lines.end();
        area->redraw_label();

        browser->clear();
        for (; i != e; ++i)
        {
            browser->add((*i).c_str());
        }

        browser->redraw();
    }

} // namespace mrv
