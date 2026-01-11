// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


// #define DEBUG_REQUESTS 1

#include "mrvApp/mrvApp.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvUtil.h"

#include "mrvFLU/flu_pixmaps.h"
#include "mrvFLU/flu_file_chooser_pixmaps.h"
#include "mrvFLU/Flu_Enumerations.h"
#include "mrvFLU/Flu_Entry.h"
#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"


#include <tlCore/Path.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Timeline.h>

#include <FL/fl_utf8.h>


#include <filesystem>
namespace fs = std::filesystem;
#include <random>
#include <mutex>


namespace
{
    const float kTimeout = 0.05;
}

Fl_Pixmap preview_img((char* const*)monalisa_xpm),
    file_list_img((char* const*)filelist_xpm),
    file_listwide_img((char* const*)filelistwide_xpm),
    fileDetails((char* const*)filedetails_xpm),
    desktop((char* const*)desktop_xpm),
    folder_closed((char* const*)folder_closed_xpm),
    default_file((char* const*)textdoc_xpm),
    my_computer((char* const*)my_computer_xpm),
    computer((char* const*)computer_xpm),
    disk_drive((char* const*)disk_drive_xpm),
    cd_drive((char* const*)cd_drive_xpm),
    floppy_drive((char* const*)floppy_drive_xpm),
    removable_drive((char* const*)removable_drive_xpm),
    ram_drive((char* const*)ram_drive_xpm),
    network_drive((char* const*)network_drive_xpm),
    documents((char* const*)filled_folder_xpm),
    little_favorites((char* const*)mini_folder_favorites_xpm),
    little_desktop((char* const*)mini_desktop_xpm),
    reel((char* const*)reel_xpm), picture((char* const*)image_xpm);

Fl_Image* usd = nullptr;
Fl_Image* music = nullptr;

static const int kColorOne = fl_rgb_color(200, 200, 200);
static const int kColorTwo = fl_rgb_color(180, 180, 180);

namespace
{

    std::string shortenString(const std::string& filename, int maxW)
    {
        int H = 0;
        int W = 0;

        std::string shortened = filename;
        fl_measure(shortened.c_str(), W, H);

        const char* start = filename.c_str();
        const char* end = start + filename.size();
        const char* pos = end;

        // Loop until target length is reached or beginning is reached
        while (W > maxW && pos > start)
        {
            // Move back one character (considering UTF-8 encoding)
            if (pos <= start)
                break;
            pos = fl_utf8back(pos - 1, start, pos);
            shortened = std::string(start, pos) + "...";
            W = 0;
            fl_measure(shortened.c_str(), W, H);
        }

        return shortened;
    }
} // namespace

struct Flu_Entry::Private
{
#ifdef OPENGL_BACKEND
    std::weak_ptr<timelineui::ThumbnailSystem> thumbnailSystem;

    struct ThumbnailData
    {
        bool init = true;
        timelineui::ThumbnailRequest request;
    };
#endif

#ifdef VULKAN_BACKEND
    std::weak_ptr<timelineui_vk::ThumbnailSystem> thumbnailSystem;

    struct ThumbnailData
    {
        bool init = true;
        timelineui_vk::ThumbnailRequest request;
    };
#endif
    
    ThumbnailData thumbnail;

    bool bind_image = false;
};

Flu_Entry::Flu_Entry(
    const char* name, int t, bool d, Flu_File_Chooser* c,
    const std::shared_ptr<system::Context>& context) :
    Fl_Input(0, 0, 0, 0),
    _p(new Private)
{
    TLRENDER_P();

#ifdef OPENGL_BACKEND
    p.thumbnailSystem = context->getSystem<timelineui::ThumbnailSystem>();
#endif

#ifdef VULKAN_BACKEND
    p.thumbnailSystem = context->getSystem<timelineui_vk::ThumbnailSystem>();
#endif

    _init(name, t, d, c);
}

Flu_Entry::Flu_Entry(const char* name, int t, bool d, Flu_File_Chooser* c) :
    Fl_Input(0, 0, 0, 0),
    _p(new Private)
{
    _init(name, t, d, c);
}

void Flu_Entry::_init(const char* name, int t, bool d, Flu_File_Chooser* c)
{
    resize(0, 0, DEFAULT_ENTRY_WIDTH, 20);
    textsize(12);
    box(FL_BORDER_BOX);
    when(FL_WHEN_RELEASE_ALWAYS | FL_WHEN_ENTER_KEY_ALWAYS);
    callback(_inputCB, this);
    filename = name;
    selected = false;
    chooser = c;
    details = d;
    type = t;
    icon = nullptr;
    editMode = 0;
    description = "";

    if (type == ENTRY_FILE && (c->type() &
                               static_cast<int>(ChooserType::DEACTIVATE_FILES)))
    {
        textcolor(FL_GRAY);
        deactivate();
    }

    updateSize();
    updateIcon();
}

void Flu_Entry::timerEvent_cb(void* d)
{
    Flu_Entry* self = static_cast<Flu_Entry*>(d);
    self->timerEvent();
}

void Flu_Entry::timerEvent()
{
    TLRENDER_P();

    if (p.thumbnail.request.future.valid() &&
        p.thumbnail.request.future.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready)
    {
        if (auto image = p.thumbnail.request.future.get())
        {
            if (image::PixelType::RGBA_U8 == image->getPixelType())
            {
                const int w = image->getWidth();
                const int h = image->getHeight();
                const int depth = 4;

                uint8_t* pixelData = new uint8_t[w * h * depth];

                auto rgbImage = new Fl_RGB_Image(pixelData, w, h, depth);
                rgbImage->alloc_array = true;

                uint8_t* d = pixelData;
                const uint8_t* s = image->getData();

#ifdef OPENGL_BACKEND
                for (int y = 0; y < h; ++y)
                {
                    memcpy(d + (h - 1 - y) * w * 4, s + y * w * 4, w * 4);
                }
#endif

#ifdef VULKAN_BACKEND
                std::memcpy(d, s, w * h * 4);
#endif
                bind_image(rgbImage);
                updateSize();
                redraw();
                Fl_Group* g = chooser->getEntryGroup();
                g->redraw();
            }
            else
            {
                isPicture = false;
            }
        }
        else
        {
            isPicture = false;
        }
    }
    else
    {
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }
}

void Flu_Entry::updateIcon()
{
    Flu_File_Chooser::FileTypeInfo* tt = nullptr;
    if (type == ENTRY_MYCOMPUTER)
    {
        icon = &computer;
        description = _(Flu_File_Chooser::myComputerTxt.c_str());
    }
    else if (type == ENTRY_MYDOCUMENTS)
    {
        icon = &documents;
        description = _(Flu_File_Chooser::myDocumentsTxt.c_str());
    }
    else if (type == ENTRY_DRIVE)
    {
        // icon = &disk_drive;
        // description = "";
    }
    else if (type == ENTRY_DIR || type == ENTRY_FAVORITE)
        tt = Flu_File_Chooser::find_type(nullptr);
    else
    {
        const char* dot = strrchr(filename.c_str(), '.');
        if (dot)
        {
            tt = Flu_File_Chooser::find_type(dot + 1);
            if (!tt)
                description = dot + 1;
        }
    }
    if (tt)
    {
        icon = tt->icon;
        description = tt->type;
    }
    // if there is no icon, assign a default one
    if (!icon && type == ENTRY_FILE &&
        !(static_cast<int>(chooser->type()) &
          static_cast<int>(ChooserType::DEACTIVATE_FILES)))
        icon = chooser->defaultFileIcon;
    if (type == ENTRY_FAVORITE)
        icon = &little_favorites;

    shortname = filename;
    size_t pos = 0;

    while ((pos = shortname.find('@', pos)) != std::string::npos)
    {
        shortname = shortname.substr(0, pos + 1) + '@' +
                    shortname.substr(pos + 1, shortname.size());
        pos += 2;
    }

    toolTip = _(Flu_File_Chooser::detailTxt[0].c_str());
    toolTip += ": " + shortname;

    shortname = filename;

    if (type == ENTRY_FILE)
    {
        toolTip += "\n";
        toolTip += _(Flu_File_Chooser::detailTxt[1].c_str());
        toolTip += ": " + filesize;
    }
    if (type == ENTRY_SEQUENCE)
    {
        toolTip += "\n";
        toolTip += _(Flu_File_Chooser::detailTxt[4].c_str());
        toolTip += ": " + filesize;
    }
    toolTip += "\n";
    toolTip += _(Flu_File_Chooser::detailTxt[3].c_str());
    toolTip += ": " + description;
    toolTip += "\n";
    toolTip += _(Flu_File_Chooser::detailTxt[5].c_str());
    toolTip += ": " + owner;
    toolTip += "\n";
    toolTip += _(Flu_File_Chooser::detailTxt[6].c_str());
    toolTip += ": " + permissions;
    tooltip(toolTip.c_str());

    set_colors();

    redraw();
}

void Flu_Entry::set_colors()
{
    Fl_Group* g = chooser->getEntryGroup();
    if (!g)
        return;
    if (selected)
    {
        color(FL_DARK_BLUE);
        return;
    }
    unsigned num = g->children();
    for (unsigned i = 0; i < num; ++i)
    {
        Flu_Entry* e = (Flu_Entry*)g->child(i);
        if (e != this)
            continue;

        if (i % 2 == 0)
        {
            color(kColorOne);
        }
        else
        {
            color(kColorTwo);
        }
        redraw();
        return;
    }
}

std::string Flu_Entry::toTLRender() const
{
    std::string fullname = Flu_File_Chooser::currentDir + filename;

    if (type == ENTRY_SEQUENCE)
    {
        std::string number = filesize;
        std::size_t pos = number.find(' ');
        number = number.substr(0, pos);
        int frame = std::stoi(number);
        char tmp[1024];
        // Note: fullname is a valid C format sequence, like picture.%04d.exr
        snprintf(tmp, 1024, fullname.c_str(), frame);
        fullname = tmp;
    }

    return fullname;
}

int Flu_Entry::handle(int event)
{
    TLRENDER_P();

    if (editMode)
    {
        // if user hits 'Escape' while in edit mode, restore the original name
        // and turn off edit mode
        if (event == FL_KEYDOWN && Fl::event_key(FL_Escape))
        {
            editMode = 0;
            redraw();
            if (selected)
                chooser->trashBtn->activate();
            return 1;
        }
        return Fl_Input::handle(event);
    }

    if (event == FL_FOCUS || event == FL_UNFOCUS)
        return 1;

    if (event == FL_ENTER)
    {
        // if user enters an entry cell, color it yellow
        if (!selected)
        {
            color(FL_YELLOW);
            redraw();
        }
        return 1;
    }
    if (event == FL_LEAVE)
    {
        // if user leaves an entry cell, color it gray or blue
        set_colors();
        redraw();
        chooser->redraw();
        return 1;
    }

    Fl_Group* g = chooser->getEntryGroup();
    if (event == FL_PUSH)
    {
        if (Fl::event_button1())
        {
            // double-clicking a directory cd's to it or single travel too
            if ((Flu_File_Chooser::singleButtonTravelDrawer ||
                 Fl::event_clicks() > 0) &&
                (type != ENTRY_FILE && type != ENTRY_SEQUENCE))
            {
                Fl::event_clicks(0);
#ifdef _WIN32
                if (filename[1] == ':')
                    chooser->delayedCd = filename;
                else
#endif
                    chooser->delayedCd = chooser->currentDir + filename + "/";
                if (type == ENTRY_FAVORITE)
                    chooser->delayedCd = filename;
                Fl::add_timeout(0.1f, Flu_File_Chooser::delayedCdCB, chooser);
            }
            // double-clicking a favorite cd's to it
            if (Fl::event_clicks() > 0)
            {
                if (type == ENTRY_FAVORITE)
                {
                    Fl::event_clicks(0);
                    chooser->delayedCd = filename;
                    Fl::add_timeout(
                        0.1f, Flu_File_Chooser::delayedCdCB, chooser);
                }
                // double-clicking a file chooses it if we are in file selection
                // mode
                else if (
                    (static_cast<int>(chooser->type()) &
                     static_cast<int>(ChooserType::DIRECTORY)) ||
                    (static_cast<int>(chooser->type()) &
                     static_cast<int>(ChooserType::STDFILE)))
                {
                    Fl::event_clicks(0);
                    Flu_File_Chooser::selectCB(chooser);
                }

                if (selected)
                    chooser->trashBtn->activate();
                return 1;
            }
        }
        if (static_cast<int>(chooser->type()) &
            static_cast<int>(ChooserType::MULTI))
        {
            if (Fl::event_state(FL_CTRL))
            {
                selected = !selected; // toggle this item
                chooser->lastSelected = this;
                chooser->redraw();
                chooser->getEntryContainer()->take_focus();
            }
            else if (Fl::event_state(FL_SHIFT))
            {
                // toggle all items from the last selected item to this one
                if (chooser->lastSelected == nullptr)
                {
                    selected = true;
                    chooser->lastSelected = this;
                    chooser->redraw();
                    chooser->getEntryContainer()->take_focus();
                }
                else
                {
                    // get the index of the last selected item and this item
                    int lastindex = -1, thisindex = -1;
                    int i;
                    for (i = 0; i < g->children(); ++i)
                    {
                        if (g->child(i) == chooser->lastSelected)
                            lastindex = i;
                        if (g->child(i) == this)
                            thisindex = i;
                        if (lastindex >= 0 && thisindex >= 0)
                            break;
                    }
                    if (lastindex >= 0 && thisindex >= 0)
                    {
                        // loop from this item to the last item, toggling each
                        // item except the last
                        int inc;
                        if (thisindex > lastindex)
                            inc = -1;
                        else
                            inc = 1;
                        Flu_Entry* e;
                        for (i = thisindex; i != lastindex; i += inc)
                        {
                            e = (Flu_Entry*)g->child(i);
                            e->selected = !e->selected;
                            e->redraw();
                        }
                        chooser->lastSelected = this;
                        chooser->redraw();
                        chooser->getEntryContainer()->take_focus();
                    }
                }
            }
            else
            {
                chooser->unselect_all();
                selected = true;
                chooser->lastSelected = this;
                chooser->redraw();
                chooser->getEntryContainer()->take_focus();
            }

            if (!((static_cast<int>(chooser->type()) &
                   static_cast<int>(ChooserType::DIRECTORY)) ||
                  (static_cast<int>(chooser->type()) &
                   static_cast<int>(ChooserType::STDFILE))) &&
                (Fl::event_state(FL_CTRL) || Fl::event_state(FL_SHIFT)))
            {
                // if we are only choosing multiple files, don't allow a
                // directory to be selected
                Fl_Group* g = chooser->getEntryGroup();
                for (int i = 0; i < g->children(); ++i)
                {
                    Flu_Entry* e = (Flu_Entry*)g->child(i);
                    if (e->type == ENTRY_DIR)
                        e->selected = false;
                }
            }
        }
        else
        {
            chooser->unselect_all();
            selected = true;
            chooser->lastSelected = this;
            chooser->redraw();
            chooser->getEntryContainer()->take_focus();
        }

        // g->take_focus();

        redraw();
        if (selected)
            chooser->trashBtn->activate();

        if (Fl::event_button3())
            return chooser->popupContextMenu(this);

        // don't put the filename into the box if we are a directory but we are
        // not choosing directories or if we are in SAVING mode
        if ((static_cast<int>(chooser->type()) &
             static_cast<int>(ChooserType::DIRECTORY)) ||
            (static_cast<int>(chooser->type()) &
             static_cast<int>(ChooserType::STDFILE)) ||
            type == ENTRY_FILE || type == ENTRY_SEQUENCE)
            chooser->filename.value(filename.c_str());
        else if (!(static_cast<int>(chooser->type()) &
                   static_cast<int>(ChooserType::SAVING)))
            chooser->filename.value("");
        chooser->filename.insert_position(
            chooser->filename.size(), chooser->filename.size());

        return 1;
    }
    else if (event == FL_DRAG)
    {
        if (static_cast<int>(chooser->type()) &
            static_cast<int>(ChooserType::MULTI))
        {
            // toggle all items from the last selected item to this one
            if (chooser->lastSelected != nullptr)
            {
                selected = true;
                // get the index of the last selected item and this item
                int lastindex = -1, thisindex = -1;
                int i;
                for (i = 0; i < g->children(); ++i)
                {
                    if (g->child(i) == chooser->lastSelected)
                        lastindex = i;
                    if (g->child(i) == this)
                        thisindex = i;
                    if (lastindex >= 0 && thisindex >= 0)
                        break;
                }
                if (lastindex >= 0 && thisindex >= 0)
                {
                    // loop from this item to the last item, toggling each item
                    // except the last
                    int inc = (thisindex > lastindex) ? -1 : 1;
                    Flu_Entry* e;
                    for (i = thisindex; i != lastindex; i += inc)
                    {
                        e = (Flu_Entry*)g->child(i);
                        e->selected = !e->selected;
                        e->redraw();
                    }
                    chooser->lastSelected = this;
                    chooser->redraw();
                }
                redraw();
                chooser->getEntryContainer()->take_focus();
                if (selected)
                    chooser->trashBtn->activate();
                return 1;
            }
        }
    }
    return Fl_Widget::handle(event);
}

void Flu_Entry::editCB()
{
    // if already selected, switch to input mode
    editMode = 2;
    value(filename.c_str());
    take_focus();
    // select the text up to but not including the extension
    const char* dot = strrchr(filename.c_str(), '.');
    if (dot)
        insert_position(0, (int)(dot - filename.c_str()));
    else
        insert_position(0, (int)filename.size());
    chooser->trashBtn->deactivate();
    redraw();
}

void Flu_Entry::updateSize(int& W, int& H, int& iW, int& iH, int& tW, int& tH)
{
    TLRENDER_P();

    // Some constants
    const int marginW = 2;
    const int marginH = 4;

    // Measure the name in width and height
    tW = 0;
    tH = 0;
    fl_font(textfont(), textsize());

    // measure the name and see if we need a truncated version
    fl_measure(filename.c_str(), tW, tH);

    // how big is the icon?
    iW = 22, iH = marginH;
    bool wide = chooser->fileListWideBtn->value();
    if (icon)
    {
        if (chooser->previewBtn->value() && (icon == &reel || icon == &picture))
        {
            iW = icon->w() + marginW;
            iH = 64 + marginH;
        }
        else
        {
            iW = icon->w() + marginW;
            iH = icon->h() + marginH;
        }
    }

    if (!isPicture || wide)
        tH = 0;

    W = tW;
    H = tH + iH;
}

void Flu_Entry::updateSize()
{
    int W, H, iW, iH, tW, tH;
    updateSize(W, H, iW, iH, tW, tH);

    if (type == ENTRY_FAVORITE || chooser->fileListWideBtn->value())
    {
        resize(x(), y(), chooser->filelist->w() - 4, H);
    }
    else
        resize(x(), y(), DEFAULT_ENTRY_WIDTH, H);

    details = chooser->fileDetailsBtn->value() && (type != ENTRY_FAVORITE);

    if (details)
    {
        nameW = chooser->detailNameBtn->w();
        typeW = chooser->detailTypeBtn->w();
        sizeW = chooser->detailSizeBtn->w();
        dateW = chooser->detailDateBtn->w();
        resize(x(), y(), chooser->filedetails->w(), H);
    }
    else
        nameW = w();

    const int maxW = nameW;
    if (W > maxW)
    {
        // progressively strip characters off the end of the name until
        // it fits with "..." at the end
        if (!altname.empty())
            shortname = altname;
        else
            shortname = filename;

        shortname = shortenString(shortname, maxW);
    }
    else
        shortname = "";

    // measure the description and see if we need a truncated version
    shortDescription = "";
    if (details)
    {
        W = 0;
        H = 0;
        fl_measure(description.c_str(), W, H);
        const int maxW = typeW - 4;
        if (W > maxW)
        {
            // progressively strip characters off the end of the description
            // until it fits with "..." at the end
            shortDescription = shortenString(description, maxW);
        }
    }

    redraw();
}

Flu_Entry::~Flu_Entry()
{
    TLRENDER_P();

    if (p.bind_image)
        delete icon;
}

void Flu_Entry::inputCB()
{
    redraw();

    // if the user tried to change the string to nothing, restore the original
    // name and turn off edit mode
    if (strlen(value()) == 0)
    {
        editMode = 0;
        return;
    }

    // if input text is different from filename, try to change the filename
    if (strcmp(value(), filename.c_str()) != 0)
    {
        // build the total old filename and new filename
        std::string oldName = chooser->currentDir + filename,
                    newName = chooser->currentDir + value();
        // see if new name already exists
        int result = fs::exists(newName);
        if (result == 0)
        {
            mrv::fl_alert(
                Flu_File_Chooser::fileExistsErrTxt.c_str(), newName.c_str());
            return; // leave editing on
        }

        fs::rename(oldName, newName);

        if (!fs::exists(newName))
        {
            mrv::fl_alert(
                Flu_File_Chooser::renameErrTxt.c_str(), oldName.c_str(),
                newName.c_str());
        }
        else
        {
            filename = value();
            updateSize();
            updateIcon();
        }
    }

    // only turn off editing if we have a successful name change
    editMode = 0;

    if (type == ENTRY_DIR)
    {
        chooser->sortCB(this);
    }
}

void Flu_Entry::draw()
{
    TLRENDER_P();

    if (editMode)
    {
        if (editMode == 2)
        {
            editMode--;
            fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);
            redraw();
        }
        textcolor(fl_rgb_color(0, 0, 0));
        Fl_Input::draw();
        return;
    }

    if (selected)
    {
        fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), Fl_Color(0x8f8f0000));
        fl_color(FL_BLACK);
    }
    else
    {
        fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), color());
        fl_color(FL_BLACK);
    }

    int W, H, iW, iH, tW, tH;
    updateSize(W, H, iW, iH, tW, tH);

    bool below = (!chooser->fileListWideBtn->value()) && isPicture;
    int X = x() + 4;
    int Y = y();
    int tY = Y;

    if (icon)
    {
        icon->draw(X, y() + (H - iH - tH) / 2 + 2);
        if (!below)
        {
            X += icon->w() + 2;
            tH = H;
        }
        else
        {
            tY += icon->h() + 2;
        }
    }

    fl_font(textfont(), textsize());
    fl_measure(filename.c_str(), W, H);
    int maxW = nameW - iW * !below;
    if (W > maxW)
    {
        // progressively strip characters off the end of the name until
        // it fits with "..." at the end
        if (!altname.empty())
            shortname = altname;
        else
            shortname = filename;

        shortname = shortenString(shortname, maxW);
    }
    else
        shortname = filename;

    size_t pos = 0;
    while ((pos = shortname.find('@', pos)) != std::string::npos)
    {
        shortname = shortname.substr(0, pos + 1) + '@' +
                    shortname.substr(pos + 1, shortname.size());
        pos += 2;
    }

    fl_draw(shortname.c_str(), X, tY, nameW, tH, FL_ALIGN_LEFT);

    X = x() + 4 + nameW;

    if (details)
    {
        if (!shortDescription.empty())
            fl_draw(
                shortDescription.c_str(), X, y(), typeW - 4, h(),
                Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_CLIP));
        else
            fl_draw(
                description.c_str(), X, y(), typeW - 4, h(),
                Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_CLIP));

        X += typeW;

        fl_draw(
            filesize.c_str(), X, y(), sizeW - 4, h(),
            Fl_Align(FL_ALIGN_RIGHT | FL_ALIGN_CLIP));

        X += sizeW + 4;

        fl_draw(
            date.c_str(), X, y(), dateW - 4, h(),
            Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_CLIP));
    }
}

void Flu_Entry::bind_image(Fl_RGB_Image* image)
{
    TLRENDER_P();
    if (p.bind_image)
        delete icon;
    icon = image;
    p.bind_image = true;
}

void Flu_Entry::startRequest()
{
    TLRENDER_P();

    if (!p.thumbnailSystem.lock() || icon == music || !p.thumbnail.init)
        return;

    file::Path path(toTLRender());
    if (mrv::file::isDirectory(path.get()))
        return;

    auto extension = tl::string::toLower(path.getExtension());

    bool requestIcon = chooser->previewBtn->value() &&
                       mrv::file::isValidType(path) &&
                       !mrv::file::isAudio(path) && extension != ".ndi";

    if (!Flu_File_Chooser::thumbnailsUSD)
    {
        if (extension == ".usd" || extension == ".usda" ||
            extension == ".usc" || extension == ".usz")
        {
            requestIcon = false;
        }
    }

    if (!requestIcon)
    {
        cancelRequest();
        return;
    }

    image::Size size(128, 64);
    otime::RationalTime time = time::invalidTime;

    // Needed to change icon when user saved over the same image name.

    if (auto thumbnailSystem = p.thumbnailSystem.lock())
    {
        if (extension == ".otio" || extension == ".otioz")
        {
            const auto& timeline =
                timeline::Timeline::create(path, mrv::App::app->getContext());
            const auto& timeRange = timeline->getTimeRange();
            if (time::isValid(timeRange))
            {
                time = timeRange.start_time();
            }
        }

        io::Options options;

        std::random_device rd;
        options["clearCache"] = string::Format("{0}").arg(rd());
        p.thumbnail.request = thumbnailSystem->getThumbnail(path, size.h, time,
                                                            options);
        p.thumbnail.init = false;
        isPicture = true;

        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }
}

void Flu_Entry::cancelRequest()
{
    TLRENDER_P();

    if (!p.thumbnailSystem.lock() || p.thumbnail.init ||
        !p.thumbnail.request.future.valid())
        return;

    if (auto thumbnailSystem = p.thumbnailSystem.lock())
    {
        thumbnailSystem->cancelRequests({p.thumbnail.request.id});

        p.thumbnail.init = true;
        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
    }
}
