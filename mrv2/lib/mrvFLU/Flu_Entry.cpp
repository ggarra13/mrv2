

#include <mutex>

#include <tlCore/Path.h>
#include <tlCore/String.h>

#include <FL/Fl_Pixmap.H>

#include "mrvFLU/flu_pixmaps.h"
#include "mrvFLU/flu_file_chooser_pixmaps.h"
#include "mrvFLU/Flu_Enumerations.h"
#include "mrvFLU/Flu_Entry.h"
#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvUtil.h"

#ifdef TLRENDER_GL
#    include "mrvGL/mrvThumbnailCreator.h"
#endif

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
    reel((char* const*)reel_xpm), picture((char* const*)image_xpm),
    music((char* const*)music_xpm);

Fl_Image* usd = mrv::load_svg("USD.svg");

static const int kColorOne = fl_rgb_color(200, 200, 200);
static const int kColorTwo = fl_rgb_color(180, 180, 180);

struct Flu_Entry::Private
{
    std::shared_ptr<mrv::ThumbnailCreator> thumbnailCreator;
    std::mutex thumbnailMutex;
    int64_t id = -1;
};

static void createdThumbnail_cb(
    const int64_t id,
    const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
        thumbnails,
    void* opaque)
{
    Flu_Entry* entry = static_cast< Flu_Entry* >(opaque);
    entry->createdThumbnail(id, thumbnails);
}

Flu_Entry::Flu_Entry(
    const char* name, int t, bool d, Flu_File_Chooser* c,
    std::shared_ptr<mrv::ThumbnailCreator> thumbnailCreator) :
    Fl_Input(0, 0, 0, 0),
    _p(new Private)
{
    TLRENDER_P();

    p.thumbnailCreator = thumbnailCreator;

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

    if (type == ENTRY_FILE && (static_cast<int>(c->selectionType) &
                               static_cast<int>(ChooserType::DEACTIVATE_FILES)))
    {
        textcolor(FL_GRAY);
        deactivate();
    }

    updateSize();
    updateIcon();
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
        !(static_cast<int>(chooser->selectionType) &
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

std::string Flu_Entry::toTLRender()
{
    std::string fullname = Flu_File_Chooser::currentDir + filename;

    if (type == ENTRY_SEQUENCE)
    {
        std::string number = filesize;
        std::size_t pos = number.find(' ');
        number = number.substr(0, pos);
        int frame = atoi(number.c_str());
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
        if (Fl::event_button() == FL_LEFT_MOUSE)
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
                    !(static_cast<int>(chooser->selectionType) &
                      static_cast<int>(ChooserType::DIRECTORY)) ||
                    (static_cast<int>(chooser->selectionType) &
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

        /*
          if( selected && !Fl::event_button3() && !Fl::event_state(FL_CTRL) &&
          !Fl::event_state(FL_SHIFT) )
          {
          // only allow editing of certain files and directories
          if( chooser->fileEditing && ( type == ENTRY_FILE || type == ENTRY_DIR
          ) )
          {
          // if already selected, switch to input mode
          Fl::add_timeout( 1.0, _editCB, this );
          return 1;
          }
          }

          else*/
        if (static_cast<int>(chooser->selectionType) &
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
                    for (i = 0; i < g->children(); i++)
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

            if (!((static_cast<int>(chooser->selectionType) &
                   static_cast<int>(ChooserType::DIRECTORY)) ||
                  (static_cast<int>(chooser->selectionType) &
                   static_cast<int>(ChooserType::STDFILE))) &&
                (Fl::event_state(FL_CTRL) || Fl::event_state(FL_SHIFT)))
            {
                // if we are only choosing multiple files, don't allow a
                // directory to be selected
                Fl_Group* g = chooser->getEntryGroup();
                for (int i = 0; i < g->children(); i++)
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
        if ((static_cast<int>(chooser->selectionType) &
             static_cast<int>(ChooserType::DIRECTORY)) ||
            (static_cast<int>(chooser->selectionType) &
             static_cast<int>(ChooserType::STDFILE)) ||
            type == ENTRY_FILE || type == ENTRY_SEQUENCE)
            chooser->filename.value(filename.c_str());
        else if (!(static_cast<int>(chooser->selectionType) &
                   static_cast<int>(ChooserType::SAVING)))
            chooser->filename.value("");
        chooser->filename.insert_position(
            chooser->filename.size(), chooser->filename.size());

        return 1;
    }
    else if (event == FL_DRAG)
    {
        if (static_cast<int>(chooser->selectionType) &
            static_cast<int>(ChooserType::MULTI))
        {
            // toggle all items from the last selected item to this one
            if (chooser->lastSelected != nullptr)
            {
                selected = true;
                // get the index of the last selected item and this item
                int lastindex = -1, thisindex = -1;
                int i;
                for (i = 0; i < g->children(); i++)
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
void Flu_Entry::updateSize()
{
    int H = 20;
    if (icon)
    {
        if (chooser->previewBtn->value() &&
            (icon == &reel || icon == &picture) &&
            Flu_File_Chooser::thumbnailsFileReq)
        {
            H = 68;
        }
        else
        {
            H = icon->h() + 4;
        }
    }
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

    // how big is the icon?
    int iW = 22;
    if (icon)
    {
        iW = icon->w() + 2;
    }

    fl_font(textfont(), textsize());

    // measure the name and see if we need a truncated version
    int W = 0;
    H = 0;
    fl_measure(filename.c_str(), W, H);
    if (W > nameW - iW)
    {
        // progressively strip characters off the end of the name until
        // it fits with "..." at the end
        if (altname[0] != '\0')
            shortname = altname;
        else
            shortname = filename;

        size_t len = shortname.size();
        while (W > (nameW - iW) && len > 3)
        {
            shortname[len - 3] = '.';
            shortname[len - 2] = '.';
            shortname[len - 1] = '.';
            shortname[len] = '\0';
            len--;
            W = 0;
            fl_measure(shortname.c_str(), W, H);
        }
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
        if (W > typeW - 4)
        {
            // progressively strip characters off the end of the description
            // until it fits with "..." at the end
            shortDescription = description;
            size_t len = shortDescription.size();
            while (W > typeW - 4 && len > 3)
            {
                shortDescription[len - 3] = '.';
                shortDescription[len - 2] = '.';
                shortDescription[len - 1] = '.';
                shortDescription[len] = '\0';
                len--;
                W = 0;
                fl_measure(shortDescription.c_str(), W, H);
            }
        }
    }

    redraw();
}

Flu_Entry::~Flu_Entry()
{
    TLRENDER_P();
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
        struct stat s;
        int result = ::stat(newName.c_str(), &s);
        if (result == 0)
        {
            mrv::fl_alert(
                Flu_File_Chooser::fileExistsErrTxt.c_str(), newName.c_str());
            return; // leave editing on
        }

        if (rename(oldName.c_str(), newName.c_str()) == -1)
        {
            mrv::fl_alert(
                Flu_File_Chooser::renameErrTxt.c_str(), oldName.c_str(),
                newName.c_str());
            // return;  // leave editing on
        }
        else
        {
            filename = value();
            updateSize();
            updateIcon();
        }
        // QUESTION: should we set the chooser filename to the modified name?
        // chooser->filename.value( filename.c_str() );
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
    
    int X = x() + 4;
    int Y = y();
    int iH = 0;
    if (icon)
    {
        {
            icon->draw(X, y() + h() / 2 - icon->h() / 2);
            X += icon->w() + 2;
        }
    }

    int iW = 0, W = 0, H = 0;
    if (icon)
    {
        iW = icon->w() + 2;
    }

    fl_font(textfont(), textsize());
    // fl_color( textcolor() );

    fl_measure(filename.c_str(), W, H);
    if (W > nameW - iW)
    {
        // progressively strip characters off the end of the name until
        // it fits with "..." at the end
        if (altname[0] != '\0')
            shortname = altname;
        else
            shortname = filename;
        size_t len = shortname.size();
        while (W > (nameW - iW) && len > 3)
        {
            shortname[len - 3] = '.';
            shortname[len - 2] = '.';
            shortname[len - 1] = '.';
            shortname[len] = '\0';
            len--;
            W = 0;
            fl_measure(shortname.c_str(), W, H);
        }
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

    fl_draw(shortname.c_str(), X, Y, nameW, h() - iH, FL_ALIGN_LEFT);

    shortname = filename;

    X = x() + 4 + nameW;

    if (details)
    {
        if (shortDescription[0] != '\0')
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
    redraw();
}

void Flu_Entry::startRequest()
{
    TLRENDER_P();
    if (!p.thumbnailCreator || p.id != -1 || icon == &music)
        return;

    file::Path path(filename);
    if (mrv::file::isDirectory(path.get()))
        return;
    
    auto extension = tl::string::toLower(path.getExtension());

    bool requestIcon =
        mrv::file::isValidType(path) && !mrv::file::isAudio(path) &&
        extension != ".ndi" && Flu_File_Chooser::thumbnailsFileReq;

    if (!Flu_File_Chooser::thumbnailsUSD)
    {
        if (extension == ".usd" || extension == ".usda" ||
            extension == ".usc" || extension == ".usz")
        {
            cancelRequest();
            return;
        }
    }

    if (!requestIcon)
    {
        cancelRequest();
        return;
    }
    
    std::cerr << "\tSTART REQUEST " << filename << std::endl;
    p.thumbnailCreator->initThread();

    // Show the frame at the beginning
    const otio::RationalTime& time = time::invalidTime;

    image::Size size(128, 64);

    p.thumbnailCreator->clearCache();

    const std::string& fullname = toTLRender();
    auto id = p.thumbnailCreator->request(
        fullname, time, size, createdThumbnail_cb, (void*)this);
    p.id = id;
}

void Flu_Entry::cancelRequest()
{
    TLRENDER_P();
    if (!p.thumbnailCreator || p.id == -1)
        return;
    
    const std::lock_guard<std::mutex> lock(p.thumbnailMutex);
    p.thumbnailCreator->cancelRequests(p.id);
    p.thumbnailCreator->stopThread();
    p.id = -1;
}

void Flu_Entry::createdThumbnail(
    const int64_t id,
    const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
        thumbnails)
{
    TLRENDER_P();
    std::lock_guard<std::mutex> lock(p.thumbnailMutex);
    if (id == p.id)
    {
        for (const auto& i : thumbnails)
        {
            icon = i.second;
            redraw();
            parent()->redraw();
        }
    }
}
