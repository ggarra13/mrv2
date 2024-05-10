#pragma once


#include <tlUI/ThumbnailSystem.h>

#include <FL/Fl_Input.H>


extern Fl_Pixmap preview_img,
    file_list_img,
    file_listwide_img,
    fileDetails,
    desktop,
    folder_closed,
    default_file,
    my_computer,
    computer,
    disk_drive,
    cd_drive,
    floppy_drive,
    removable_drive,
    ram_drive,
    network_drive,
    documents,
    little_favorites,
    little_desktop, reel, picture,
    music;

class Flu_File_Chooser;
using namespace tl::ui;

class Flu_Entry : public Fl_Input
{
public:
    Flu_Entry(
        const char* name, int t, bool d, Flu_File_Chooser* c,
        const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator =
            nullptr);
    ~Flu_Entry();

    int handle(int event);
    void draw();

    void set_colors();

    void updateSize();
    void updateIcon();

    inline static void _inputCB(Fl_Widget* w, void* arg)
        {
            ((Flu_Entry*)arg)->inputCB();
        }
    void inputCB();

    inline static void _editCB(void* arg) { ((Flu_Entry*)arg)->editCB(); }
    void editCB();

    std::string filename, date, filesize, shortname, owner, description,
        shortDescription, toolTip, altname;
    std::string permissions;
    unsigned char pU, pG, pO; // 3-bit unix style permissions
    unsigned int type;
    time_t idate;
    int64_t isize;
    bool selected;
    int editMode;
    Flu_File_Chooser* chooser;
    Fl_Image* icon;

    //! Convert our internal information to a tlRender friendly filename.
    std::string toTLRender();
        
    bool details;
    int nameW, typeW, sizeW, dateW;

    TLRENDER_PRIVATE();
};
