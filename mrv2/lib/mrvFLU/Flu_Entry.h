#pragma once

#include <tlCore/Time.h>

#include <FL/Fl_Input.H>

namespace mrv
{
    class ThumbnailCreator;
}

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
        std::shared_ptr<mrv::ThumbnailCreator> thumbnailGenerator = nullptr);
    virtual ~Flu_Entry();

    int handle(int event) FL_OVERRIDE;
    void draw() FL_OVERRIDE;

    void bind_image(Fl_RGB_Image*);

    void set_colors();

    void updateSize();
    void updateIcon();

    void createdThumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails);

    void startRequest();
    void cancelRequest();

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
    Flu_File_Chooser* chooser;
    Fl_Image* icon;

protected:
    void updateSize(int& W, int& H, int& iW, int& iH, int& tW, int& tH);

    bool isPicture = false;
    bool details;
    int nameW, typeW, sizeW, dateW;

    TLRENDER_PRIVATE();
};
