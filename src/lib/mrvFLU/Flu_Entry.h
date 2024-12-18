// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Time.h>
#include <tlCore/Context.h>

#include <FL/Fl_Input.H>

extern Fl_Pixmap preview_img, file_list_img, file_listwide_img, fileDetails,
    desktop, folder_closed, default_file, my_computer, computer, disk_drive,
    cd_drive, floppy_drive, removable_drive, ram_drive, network_drive,
    documents, little_favorites, little_desktop, reel, picture;
extern Fl_Image *usd, *music;

class Fl_RGB_Image;
class Flu_File_Chooser;
using namespace tl;

class Flu_Entry : public Fl_Input
{
public:
    Flu_Entry(
        const char* name, int t, bool d, Flu_File_Chooser* c,
        const std::shared_ptr<system::Context>& context);
    Flu_Entry(const char* name, int t, bool d, Flu_File_Chooser* c);
    virtual ~Flu_Entry();

    int handle(int event) FL_OVERRIDE;
    void draw() FL_OVERRIDE;

    void bind_image(Fl_RGB_Image*);

    void set_colors();

    void updateSize();
    void updateIcon();

    void startRequest();
    void cancelRequest();

    static void timerEvent_cb(void* self);
    void timerEvent();

    inline static void _inputCB(Fl_Widget* w, void* arg)
    {
        ((Flu_Entry*)arg)->inputCB();
    }
    void inputCB();

    inline static void _editCB(void* arg) { ((Flu_Entry*)arg)->editCB(); }
    void editCB();

    //! Convert our internal information to a tlRender friendly filename.
    std::string toTLRender();

public:
    std::string filename, date, filesize, shortname, owner, description,
        shortDescription, toolTip, altname;
    std::string permissions;
    unsigned char pU, pG, pO; // 3-bit unix style permissions
    unsigned int type;
    time_t idate;
    int64_t isize;
    bool selected;
    int editMode;
    Flu_File_Chooser* chooser = nullptr;
    Fl_Image* icon = nullptr;

protected:
    void _init(const char* name, int t, bool d, Flu_File_Chooser* c);
    void updateSize(int& W, int& H, int& iW, int& iH, int& tW, int& tH);

    bool isPicture = false;
    bool details;
    int nameW, typeW, sizeW, dateW;

    TLRENDER_PRIVATE();
};
