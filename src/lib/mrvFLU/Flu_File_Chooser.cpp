/***************************************************************
 *                FLU - FLTK Utility Widgets
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 *
 * License is like FLTK.
 ***************************************************************/

#ifdef _WIN32
#    define strcasecmp _stricmp
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <random>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <set>

#define FLU_USE_REGISTRY

#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#    include <shlobj.h>
#    include <shellapi.h>
#    include <lmcons.h>
#endif

#if defined _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#    include <pwd.h>
#endif

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/math.h>
#include <FL/filename.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Shared_Image.H>

#include <tlIO/System.h>

#include <tlCore/Path.h>
#include <tlCore/String.h>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvString.h"

#include "mrvIcons/Documents.h"
#include "mrvIcons/FavoritesButton.h"
#include "mrvIcons/FavoritesFolders.h"
#include "mrvIcons/Home.h"
#include "mrvIcons/NewFolder.h"
#include "mrvIcons/Music.h"
#include "mrvIcons/RefreshDir.h"
#include "mrvIcons/TemporaryButton.h"
#include "mrvIcons/Trashcan.h"
#include "mrvIcons/UpFolder.h"
#include "mrvIcons/USD.h"


#include "mrvFl/mrvPreferences.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvUtil.h"

#include "mrvFLU/flu_pixmaps.h"
#include "mrvFLU/flu_file_chooser_pixmaps.h"
#include "mrvFLU/Flu_Label.h"
#include "mrvFLU/Flu_Scroll.h"
#include "mrvFLU/Flu_Separator.h"
#include "mrvFLU/Flu_Enumerations.h"
#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "flu";
}

// set default language strings
std::string Flu_File_Chooser::favoritesTxt = _("Favorites");
#ifdef _WIN32
std::string Flu_File_Chooser::myComputerTxt = _("My Computer");
std::string Flu_File_Chooser::myDocumentsTxt = _("My Documents");
std::string Flu_File_Chooser::desktopTxt = _("Desktop");
#else
std::string Flu_File_Chooser::myComputerTxt = _("Home");
std::string Flu_File_Chooser::myDocumentsTxt = _("Temporary");
std::string Flu_File_Chooser::desktopTxt = _("Desktop");
#endif

std::string Flu_File_Chooser::detailTxt[7] = {
    _("Name"), _("Size"), _("Date"), _("Type"), _("Frames"),
    _("Owner"), _("Permissions")};
std::string Flu_File_Chooser::contextMenuTxt[3] = {
    _("New Folder"), _("Rename"), _("Delete")};
std::string Flu_File_Chooser::diskTypesTxt[6] = {
    "Floppy Disk",  "Removable Disk", "Local Disk",
    "Compact Disk", "Network Disk",   "RAM Disk"};

std::string Flu_File_Chooser::filenameTxt = _("Filename");
std::string Flu_File_Chooser::okTxt = _("Ok");
std::string Flu_File_Chooser::cancelTxt = _("Cancel");
std::string Flu_File_Chooser::locationTxt = _("Location");
std::string Flu_File_Chooser::showHiddenTxt = _("Show Hidden Files");
std::string Flu_File_Chooser::fileTypesTxt = _("File Types");
std::string Flu_File_Chooser::directoryTxt = _("Directory");
std::string Flu_File_Chooser::allFilesTxt = _("All Files (*)");
std::string Flu_File_Chooser::defaultFolderNameTxt = _("New Folder");

std::string Flu_File_Chooser::backTTxt = _("Go back one directory in the history");
std::string Flu_File_Chooser::forwardTTxt =
    _("Go forward one directory in the history");
std::string Flu_File_Chooser::upTTxt = _("Go to the parent directory");
std::string Flu_File_Chooser::reloadTTxt = _("Refresh this directory");
std::string Flu_File_Chooser::trashTTxt = _("Delete file(s)");
std::string Flu_File_Chooser::newDirTTxt = _("Create new directory");
std::string Flu_File_Chooser::addFavoriteTTxt =
    _("Add this directory to my favorites");
std::string Flu_File_Chooser::previewTTxt = _("Preview files");
std::string Flu_File_Chooser::listTTxt = _("List mode");
std::string Flu_File_Chooser::wideListTTxt = _("Wide list mode");
std::string Flu_File_Chooser::detailTTxt = _("Detail mode");

std::string Flu_File_Chooser::createFolderErrTxt =
    _("Could not create directory '%s'. You may not have permission to "
    "perform this operation.");
std::string Flu_File_Chooser::deleteFileErrTxt =
    _("An error occurred while trying to delete '%s'. %s");
std::string Flu_File_Chooser::fileExistsErrTxt = _("File '%s' already exists!");
std::string Flu_File_Chooser::renameErrTxt = _("Unable to rename '%s' to '%s'");

// just a string that no file could probably ever be called
#define FAVORITES_UNIQUE_STRING "\t!@#$%^&*(Favorites)-=+"

#define DEFAULT_ENTRY_WIDTH 235

#define streq(a, b) (strcmp(a, b) == 0)

Flu_File_Chooser::FileTypeInfo* Flu_File_Chooser::types = nullptr;
int Flu_File_Chooser::numTypes = 0;
int Flu_File_Chooser::typeArraySize = 0;
Flu_File_Chooser::ContextHandlerVector Flu_File_Chooser::contextHandlers;
int (*Flu_File_Chooser::customSort)(const char*, const char*) = 0;
std::string Flu_File_Chooser::dArrow[4];
std::string Flu_File_Chooser::uArrow[4];

bool Flu_File_Chooser::thumbnailsUSD = true;
bool Flu_File_Chooser::thumbnailsFileReq = true;
bool Flu_File_Chooser::singleButtonTravelDrawer = true;
Flu_File_Chooser* Flu_File_Chooser::window = nullptr;
std::string Flu_File_Chooser::currentDir;

#ifdef _WIN32
// Internationalized windows folder name access
// Fix suggested by Fabien Costantini
/*
  CSIDL_DESKTOPDIRECTORY -- desktop
  CSIDL_PERSONAL -- my documents
  CSIDL_PERSONAL and strip back to last "/" -> home
*/
static std::string flu_get_special_folder(int csidl)
{
    static char path[MAX_PATH + 1];

#    ifdef FLU_USE_REGISTRY
    HKEY key;
    DWORD size = MAX_PATH;
    const char* keyQuery = "";
    switch (csidl)
    {
    case CSIDL_DESKTOPDIRECTORY:
        keyQuery = "Desktop";
        break;
    case CSIDL_PERSONAL:
        keyQuery = "Personal";
        break;
    }

    if (RegOpenKeyEx(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell "
            "Folders",
            0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return "";

    if (RegQueryValueEx(key, keyQuery, 0, 0, (LPBYTE)path, &size) !=
        ERROR_SUCCESS)
        return "";

    RegCloseKey(key);

    return path;

#    else

    path[0] = '\0';
    if (SUCCEEDED(SHGetSpecialFolderPath(nullptr, path, csidl, FALSE)))
    // if( SUCCEEDED( SHGetFolderPath( nullptr, csidl, nullptr, 0, path ) ) )
    {
        int len = strlen(path);
        if (len > 0 && path[len - 1] != '/' && path[len - 1] != '\\')
            strcat(path, "/");
        return path;
    }
    return "";
#    endif
}
#endif

struct Flu_File_Chooser::Private
{
    std::shared_ptr<system::Context> context;
};

void Flu_File_Chooser::previewCB()
{
    TLRENDER_P();
    bool inFavorites = (currentDir == FAVORITES_UNIQUE_STRING);
    if (inFavorites)
    {
        cancelThumbnailRequests();
        return;
    }

    thumbnailsFileReq = previewBtn->value();

    updateEntrySizes();

    Fl_Group* g = getEntryGroup();
    unsigned num = g->children();
    for (unsigned i = 0; i < num; ++i)
    {
        Flu_Entry* c = (Flu_Entry*)g->child(i);
        c->set_colors();
    }
}

void Flu_File_Chooser::add_context_handler(
    int type, const char* ext, const char* name,
    void (*cb)(const char*, int, void*), void* cbd)
{
    if (cb == nullptr)
        return;
    ContextHandler h;
    h.ext = ext ? ext : "";
    std::transform(
        h.ext.begin(), h.ext.end(), h.ext.begin(), (int (*)(int))tolower);
    h.type = type;
    h.name = name;
    h.callback = cb;
    h.callbackData = cbd;
    Flu_File_Chooser::contextHandlers.push_back(h);
}

// extensions == nullptr implies directories
void Flu_File_Chooser::add_type(
    const char* extensions, const char* short_description, Fl_Image* icon)
{
    std::string ext;
    if (extensions)
        ext = extensions;
    else
        ext = "\t"; // indicates a directory
    std::transform(ext.begin(), ext.end(), ext.begin(), (int (*)(int))toupper);

    // are we overwriting an existing type?
    for (int i = 0; i < numTypes; i++)
    {
        if (types[i].extensions == ext)
        {
            types[i].icon = icon;
            types[i].type = short_description;
            return;
        }
    }

    if (numTypes == typeArraySize)
    {
        int newSize =
            (typeArraySize == 0)
                ? 1
                : typeArraySize * 2; // double the size of the old list
                                     // (same behavior as STL vector)
        // allocate the new list
        FileTypeInfo* newTypes = new FileTypeInfo[newSize];
        // copy the old list to the new list
        for (int i = 0; i < numTypes; i++)
        {
            newTypes[i].icon = types[i].icon;
            newTypes[i].extensions = types[i].extensions;
            newTypes[i].type = types[i].type;
        }
        // delete the old list and replace it with the new list
        delete[] types;
        types = newTypes;
        typeArraySize = newSize;
    }

    types[numTypes].icon = icon;
    types[numTypes].extensions = ext;
    types[numTypes].type = short_description;

    numTypes++;
}

Flu_File_Chooser::FileTypeInfo*
Flu_File_Chooser::find_type(const char* extension)
{
    std::string ext;
    if (extension)
        ext = extension;
    else
        ext = "\t"; // indicates a directory
    std::transform(ext.begin(), ext.end(), ext.begin(), (int (*)(int))toupper);

    // lookup the type based on the extension
    for (int i = 0; i < numTypes; i++)
    {
        // check extension against every token
        std::string e = types[i].extensions;
        char* tok = strtok((char*)e.c_str(), " ,");
        while (tok)
        {
            if (ext == tok)
                return &(types[i]);
            tok = strtok(nullptr, " ,");
        }
    }

    return nullptr;
}

Flu_File_Chooser::Flu_File_Chooser(
    const char* pathname, const char* pat, int type, const char* title,
    const std::shared_ptr<system::Context>& context, const bool compact) :
    Fl_Double_Window(900, 600, title),
    _p(new Private),
    filename(70, h() - 60, w() - 70 - 85 - 10, 25, "", this),
    ok(w() - 90, h() - 60, 85, 25),
    cancel(w() - 90, h() - 30, 85, 25),
    wingrp(new Fl_Group(0, 0, 900, 600))
{
    TLRENDER_P();

    p.context = context;

    _compact = compact;

    int oldNormalSize = FL_NORMAL_SIZE;
    FL_NORMAL_SIZE = 12;

    _callback = 0;
    _userdata = 0;

    Fl_Double_Window::callback(_hideCB, this);

    Fl_Double_Window::size_range(900, 600);

    wingrp->box(FL_UP_BOX);
    resizable(wingrp);

    Fl_Group* g;

    filename.labelcolor(FL_WHITE);
    filename.textcolor(FL_BLACK);
    filename.cursor_color(FL_BLACK);
    filename.label(_(filenameTxt.c_str()));
    ok.label(_(okTxt.c_str()));
    ok.labelsize(FL_NORMAL_SIZE);
    cancel.label(_(cancelTxt.c_str()));
    cancel.labelsize(FL_NORMAL_SIZE);

    add_type(nullptr, _(directoryTxt.c_str()), &folder_closed);
    add_type("3gp", _("3GP Movie"), &reel);
    add_type("asf", _("Advanced Systems Format Media"), &reel);
    add_type("avc", _("AVCHD Video"), &reel);
    add_type("avchd", _("AVCHD Video"), &reel);
    add_type("avi", _("AVI Movie"), &reel);
    add_type("divx", _("DIVX Movie"), &reel);
    add_type("dv", _("Digital Video"), &reel);
    add_type("flv", _("Flash Movie"), &reel);
    add_type("m2ts", _("AVCHD Video"), &reel);
    add_type("m2t", _("AVCHD Video"), &reel);
    add_type("m4v", _("Apple's M4V Movie"), &reel);
    add_type("mkv", _("Matroska Movie"), &reel);
    add_type("mov", _("Quicktime Movie"), &reel);
    add_type("mp4", _("MP4 Movie"), &reel);
    add_type("mpeg", _("MPEG Movie"), &reel);
    add_type("mpg", _("MPEG Movie"), &reel);
    add_type("mxf", _("MXF Movie"), &reel);
    add_type("ogm", _("Ogg Movie"), &reel);
    add_type("ogv", _("Ogg Video"), &reel);
    add_type("otio", _("OpenTimelineIO EDL"), &reel);
    add_type("otioz", _("OpenTimelineIO Zipped EDL"), &reel);
    add_type("qt", _("Quicktime Movie"), &reel);
    add_type("rm", _("Real Media Movie"), &reel);
    add_type("ts", _("AVCHD Video"), &reel);
    add_type("vob", _("VOB Movie"), &reel);
    add_type("webm", _("WebM Movie"), &reel);
    add_type("wmv", _("WMV Movie"), &reel);

    add_type("3fr", _("RAW Picture"), &picture);
    add_type("arw", _("RAW Picture"), &picture);
    add_type("bay", _("RAW Picture"), &picture);
    add_type("bmp", _("Bitmap Picture"), &picture);
    add_type("bmq", _("RAW Picture"), &picture);
    add_type("cap", _("RAW Picture"), &picture);
    add_type("cin", _("Cineon Picture"), &picture);
    add_type("cine", _("RAW Picture"), &picture);
    add_type("cap", _("RAW Picture"), &picture);
    add_type("cr2", _("Canon Raw Picture"), &picture);
    add_type("cr3", _("Canon Raw Picture"), &picture);
    add_type("crw", _("Canon Raw Picture"), &picture);
    add_type("cs1", _("RAW Picture"), &picture);
    add_type("dcr", _("RAW Picture"), &picture);
    add_type("dng", _("Kodak Digital Negative"), &picture);
    add_type("dpx", _("DPX Picture"), &picture);
    add_type("drf", _("RAW Picture"), &picture);
    add_type("dsc", _("RAW Picture"), &picture);
    add_type("erf", _("RAW Picture"), &picture);
    add_type("exr", _("EXR Picture"), &picture);
    add_type("fff", _("RAW Picture"), &picture);
    add_type("gif", _("GIF Picture"), &picture);
    add_type("hdr", _("HDRI Picture"), &picture);
    add_type("ia", _("RAW Picture"), &picture);
    add_type("iiq", _("RAW Picture"), &picture);
    add_type("jpg", _("JPEG Picture"), &picture);
    add_type("jpeg", _("JPEG Picture"), &picture);
    add_type("jfif", _("JPEG Picture"), &picture);
    add_type("kdc", _("RAW Picture"), &picture);
    add_type("mdc", _("RAW Picture"), &picture);
    add_type("mef", _("RAW Picture"), &picture);
    add_type("mos", _("RAW Picture"), &picture);
    add_type("mrw", _("RAW Picture"), &picture);
    add_type("nef", _("RAW Picture"), &picture);
    add_type("nrw", _("RAW Picture"), &picture);
    add_type("orf", _("RAW Picture"), &picture);
    add_type("pef", _("RAW Picture"), &picture);
    add_type("pic", _("Softimage Picture"), &picture);
    add_type("png", _("Portable Network Graphics Picture"), &picture);
    add_type("ppm", _("Portable Pixmap"), &picture);
    add_type("psd", _("Photoshop Picture"), &picture);
    add_type("pxn", _("RAW Picture"), &picture);
    add_type("qtk", _("RAW Picture"), &picture);
    add_type("raf", _("RAW Picture"), &picture);
    add_type("raw", _("RAW Picture"), &picture);
    add_type("rgb", _("SGI Picture"), &picture);
    add_type("rgba", _("SGI Picture"), &picture);
    add_type("rw2", _("RAW Picture"), &picture);
    add_type("rwl", _("RAW Picture"), &picture);
    add_type("rwz", _("RAW Picture"), &picture);
    add_type("sgi", _("SGI Picture"), &picture);
    add_type("sr2", _("RAW Picture"), &picture);
    add_type("srf", _("RAW Picture"), &picture);
    add_type("srw", _("RAW Picture"), &picture);
    add_type("sti", _("RAW Picture"), &picture);
    add_type("sxr", _("Stereo OpenEXR Picture"), &picture);
    add_type("tga", _("TGA Picture"), &picture);
    add_type("tif", _("TIFF Picture"), &picture);
    add_type("tiff", _("TIFF Picture"), &picture);
    add_type("x3f", _("RAW Picture"), &picture);

    if (!music)
        music = MRV2_LOAD_SVG(Music);

    add_type("mp3", _("MP3 music"), music);
    add_type("ogg", _("OGG Vorbis music"), music);
    add_type("wav", _("Wave music"), music);

    if (!usd)
        usd = MRV2_LOAD_SVG(USD);
    add_type("usd", _("OpenUSD Asset"), usd);
    add_type("usdz", _("OpenUSD Zipped Asset"), usd);
    add_type("usdc", _("OpenUSD Compressed Asset"), usd);
    add_type("usda", _("OpenUSD ASCII Asset"), usd);

    for (int j = 0; j < 4; j++)
    {
        std::string text = _(detailTxt[j].c_str());
        dArrow[j] = "@-12DnArrow " + text;
        uArrow[j] = "@-18UpArrow " + text;
    }

    history = currentHist = nullptr;
    walkingHistory = false;
    fileEditing = false;
    caseSort = false;
#ifdef _WIN32
    refreshDrives = true;
#endif

    // get home area by stripping off to the last '/' from docs
    userHome = mrv::homepath();

#ifdef _WIN32
    userDesktop = flu_get_special_folder(CSIDL_DESKTOPDIRECTORY);
    userDocs = flu_get_special_folder(CSIDL_PERSONAL);

    {
        for (size_t i = userHome.size() - 1; i > 0; i--)
        {
            if (userHome[i] == '/')
            {
                userHome[i] = '\0';
                break;
            }
        }
    }

    // construct the user desktop path
    // userDesktop = userHome + "/" + desktopTxt;

    win2unix(userDesktop);
    win2unix(userDocs);

    // make sure they don't end in '/'
    if (userDesktop[userDesktop.size() - 1] == '/')
        userDesktop[userDesktop.size() - 1] = '\0';
    if (userDocs[userDocs.size() - 1] == '/')
        userDocs[userDocs.size() - 1] = '\0';

    // get the actual name of the "My Documents" folder by pulling off the last
    // name in the field we do this because the actual name may vary from
    // country to country
    {
        size_t slash = userDesktop.rfind('/');
        if (slash != std::string::npos)
            desktopTxt = userDesktop.c_str() + slash + 1;
        slash = userDocs.rfind('/');
        if (slash != std::string::npos)
            myDocumentsTxt = userDocs.c_str() + slash + 1;
    }

    // make sure they end in '/'
    userHome += "/";
    userDesktop += "/";
    userDocs += "/";

#else
    {
        userDesktop = userHome;
        userDesktop += "/";
        userDesktop += _(desktopTxt.c_str());
        userDesktop += "/";
        userDocs = "/tmp/";
    }
#endif
    configFilename = userHome + "/.filmaura/mrv2.favorites";

    selectionType = type;
    filenameEnterCallback = filenameTabCallback = false;
    sortMethod = SORT_NAME;

    lastSelected = nullptr;
    filename.labelsize(12);
    filename.when(FL_WHEN_ENTER_KEY_ALWAYS | FL_WHEN_CHANGED);
    filename.callback(_filenameCB, this);
    filename.value("");

    Fl_Group* quickIcons = new Fl_Group(5, 5, 100, h() - 10 - 60);
    quickIcons->box(FL_DOWN_BOX);
    quickIcons->color(FL_DARK2);

    Flu_Button* desktopBtn = new Flu_Button(30, 18, 50, 48);
    desktopBtn->box(FL_FLAT_BOX);
    desktopBtn->image(desktop);
    desktopBtn->enter_box(FL_THIN_UP_BOX);
    desktopBtn->color(FL_DARK3);
    desktopBtn->callback(_desktopCB, this);
    {
        Flu_Label* l = new Flu_Label(5, 62, 100, 20, _(desktopTxt.c_str()));
        l->labelcolor(fl_contrast(FL_WHITE, l->color()));
        l->align(FL_ALIGN_CENTER);
    }

    Flu_Button* homeBtn = new Flu_Button(30, 98, 50, 48);
    homeBtn->box(FL_FLAT_BOX);
    homeBtn->enter_box(FL_THIN_UP_BOX);
    homeBtn->color(FL_DARK3);
    homeBtn->callback(_homeCB, this);
    {
#ifdef _WIN32
        Flu_Label* l = new Flu_Label(5, 142, 100, 20, _(myComputerTxt.c_str()));
        homeBtn->image(my_computer);
#else
        Flu_Label* l = new Flu_Label(5, 142, 100, 20, _(myComputerTxt.c_str()));
        homeBtn->image(MRV2_LOAD_SVG(Home));
#endif
        l->labelcolor(fl_contrast(FL_WHITE, l->color()));
        l->align(FL_ALIGN_CENTER);
    }

    Flu_Button* documentsBtn = new Flu_Button(30, 178, 50, 48);
    documentsBtn->box(FL_FLAT_BOX);
    documentsBtn->enter_box(FL_THIN_UP_BOX);
    documentsBtn->labelcolor(fl_contrast(FL_WHITE, documentsBtn->color()));
    documentsBtn->color(FL_DARK3);
    documentsBtn->callback(_documentsCB, this);
    {
#ifdef _WIN32
        Flu_Label* l =
            new Flu_Label(5, 222, 100, 20, _(myDocumentsTxt.c_str()));
        documentsBtn->image(MRV2_LOAD_SVG(Documents));
#else
        Flu_Label* l =
            new Flu_Label(5, 222, 100, 20, _(myDocumentsTxt.c_str()));
        documentsBtn->image(MRV2_LOAD_SVG(TemporaryButton));
#endif
        l->labelcolor(fl_contrast(FL_WHITE, l->color()));
        l->align(FL_ALIGN_CENTER);
    }

    Flu_Button* favoritesBtn = new Flu_Button(30, 258, 50, 48);
    favoritesBtn->box(FL_FLAT_BOX);
    favoritesBtn->image(MRV2_LOAD_SVG(FavoritesButton));
    favoritesBtn->enter_box(FL_THIN_UP_BOX);
    favoritesBtn->color(FL_DARK3);
    favoritesBtn->callback(_favoritesCB, this);
    {
        Flu_Label* l = new Flu_Label(5, 302, 100, 20, _(favoritesTxt.c_str()));
        l->labelcolor(fl_contrast(FL_WHITE, l->color()));
        l->align(FL_ALIGN_CENTER);
    }

    favoritesList = new Fl_Browser(0, 0, 0, 0);
    favoritesList->hide();

    {
        Fl_Group* dummy = new Fl_Group(5, h() - 10 - 61, 100, 1);
        quickIcons->resizable(dummy);
    }
    quickIcons->end();

    Fl_Group* dummy = new Fl_Group(110, 0, w() - 110, 70);

    locationQuickJump = new Fl_Group(166, 5, w() - 171, 8);
    locationQuickJump->box(FL_NO_BOX);
    locationQuickJump->end();

    location =
        new Flu_Combo_Tree(166, 15, w() - 171, 22, _(locationTxt.c_str()));
    location->labelcolor(FL_WHITE);
    location->pop_height(200);
    // location->tree.all_branches_always_open( true );
    location->tree.showroot(false);
    location->tree.connectorstyle(FL_TREE_CONNECTOR_SOLID);
    // location->tree.horizontal_gap( -10 );
    // location->tree.show_leaves( false );
    location->callback(_locationCB, this);

    ////////////////////////////////////////////////////////////////

    g = new Fl_Group(
        110, 40, w() - 110, 30); // group enclosing all the buttons at top

    hiddenFiles =
        new Fl_Check_Button(110, 43, 130, 25, _(showHiddenTxt.c_str()));
    hiddenFiles->labelcolor(FL_WHITE);
    hiddenFiles->callback(reloadCB, this);
#ifdef _WIN32
    hiddenFiles->hide();
#endif

    backBtn = new Flu_Button(285, 43, 25, 25, "@<-");
    backBtn->labelcolor(fl_rgb_color(80, 180, 200));
    backBtn->labelsize(16);
    backBtn->box(FL_FLAT_BOX);
    backBtn->enter_box(FL_THIN_UP_BOX);
    backBtn->callback(_backCB, this);
    backBtn->tooltip(backTTxt.c_str());

    forwardBtn = new Flu_Button(310, 43, 25, 25, "@->");
    forwardBtn->labelcolor(fl_rgb_color(80, 180, 200));
    forwardBtn->labelsize(16);
    forwardBtn->box(FL_FLAT_BOX);
    forwardBtn->enter_box(FL_THIN_UP_BOX);
    forwardBtn->callback(_forwardCB, this);
    forwardBtn->tooltip(forwardTTxt.c_str());

    upDirBtn = new Flu_Button(335, 43, 25, 25);
    upDirBtn->image(MRV2_LOAD_SVG(UpFolder));
    upDirBtn->box(FL_FLAT_BOX);
    upDirBtn->enter_box(FL_THIN_UP_BOX);
    upDirBtn->callback(upDirCB, this);
    upDirBtn->tooltip(upTTxt.c_str());

    reloadBtn = new Flu_Button(360, 43, 25, 25);
    reloadBtn->image(MRV2_LOAD_SVG(RefreshDir));
    reloadBtn->box(FL_FLAT_BOX);
    reloadBtn->enter_box(FL_THIN_UP_BOX);
    reloadBtn->callback(reloadCB, this);
    reloadBtn->tooltip(reloadTTxt.c_str());

    {
        Flu_Separator* sep = new Flu_Separator(385, 42, 10, 28);
        sep->type(Flu_Separator::VERTICAL);
        sep->color(FL_WHITE);
        sep->box(FL_FLAT_BOX);
    }

    trashBtn = new Flu_Button(395, 43, 25, 25);
    trashBtn->image(MRV2_LOAD_SVG(Trashcan));
    trashBtn->box(FL_FLAT_BOX);
    trashBtn->enter_box(FL_THIN_UP_BOX);
    trashBtn->callback(_trashCB, this);
    trashBtn->tooltip(trashTTxt.c_str());

    newDirBtn = new Flu_Button(420, 43, 25, 25);
    newDirBtn->image(MRV2_LOAD_SVG(NewFolder));
    newDirBtn->box(FL_FLAT_BOX);
    newDirBtn->enter_box(FL_THIN_UP_BOX);
    newDirBtn->callback(_newFolderCB, this);
    newDirBtn->tooltip(newDirTTxt.c_str());

    addFavoriteBtn = new Flu_Button(445, 43, 25, 25);
    addFavoriteBtn->image(MRV2_LOAD_SVG(FavoritesFolders));
    addFavoriteBtn->box(FL_FLAT_BOX);
    addFavoriteBtn->enter_box(FL_THIN_UP_BOX);
    addFavoriteBtn->callback(_addToFavoritesCB, this);
    addFavoriteBtn->tooltip(addFavoriteTTxt.c_str());

    {
        Flu_Separator* sep = new Flu_Separator(470, 42, 10, 28);
        sep->type(Flu_Separator::VERTICAL);
        sep->color(FL_WHITE);
        sep->box(FL_FLAT_BOX);
    }

    previewBtn = new Flu_Button(482, 43, 23, 25);
    previewBtn->type(FL_TOGGLE_BUTTON);
    previewBtn->value(thumbnailsFileReq);
    previewBtn->image(preview_img);
    previewBtn->callback(_previewCB, this);
    previewBtn->tooltip(previewTTxt.c_str());

    {
        Fl_Group* g2 = new Fl_Group(511, 43, 81, 25);
        fileListBtn = new Flu_Button(511, 43, 25, 25);
        fileListBtn->type(FL_RADIO_BUTTON);
        fileListBtn->callback(_listModeCB, this);
        fileListBtn->image(file_list_img);
        fileListBtn->tooltip(listTTxt.c_str());
        fileListWideBtn = new Flu_Button(540, 43, 25, 25);
        fileListWideBtn->type(FL_RADIO_BUTTON);
        fileListWideBtn->callback(_listModeCB, this);
        fileListWideBtn->image(file_listwide_img);
        fileListWideBtn->tooltip(wideListTTxt.c_str());
        fileDetailsBtn = new Flu_Button(569, 43, 25, 25);
        fileDetailsBtn->type(FL_RADIO_BUTTON);
        fileDetailsBtn->image(fileDetails);
        fileDetailsBtn->value(1);
        fileDetailsBtn->callback(_listModeCB, this);
        fileDetailsBtn->tooltip(detailTTxt.c_str());
        g2->end();
    }

    g->resizable(hiddenFiles);
    g->end();

    // dummy->resizable( location );
    dummy->end();

    ////////////////////////////////////////////////////////////////

    fileGroup = new Fl_Group(110, 70, w() - 120 - 5, h() - 80 - 40 - 15);
    {
        fileGroup->box(FL_DOWN_FRAME);

        filelist = new FileList(
            fileGroup->x() + 2, fileGroup->y() + 2, fileGroup->w() - 4,
            fileGroup->h() - 4, this);
        filelist->copy_label("FileList");
        filelist->labeltype(FL_NO_LABEL);
        filelist->box(FL_FLAT_BOX);
        filelist->color(FL_WHITE);
        filelist->type(FL_HORIZONTAL);
        filelist->spacing(2, 0);
        filelist->scrollbar.linesize(DEFAULT_ENTRY_WIDTH + 4);
        filelist->end();

        {
            fileDetailsGroup = new Fl_Group(
                fileGroup->x() + 2, fileGroup->y() + 2, fileGroup->w() - 4,
                fileGroup->h() - 4);

            {
                filescroll = new Flu_Scroll(
                    fileDetailsGroup->x() + 2, fileDetailsGroup->y() + 22,
                    fileDetailsGroup->w() - 4, fileDetailsGroup->h() - 20 - 4,
                    this);
                filescroll->color(FL_WHITE);
                filescroll->scrollbar.linesize(20);
                filescroll->box(FL_FLAT_BOX);
                filescroll->type(Flu_Scroll::BOTH);
                {
                    filedetails = new FileDetails(
                        filescroll->x() + 2, filescroll->y() + 2,
                        filescroll->w() - 4, filescroll->h() - 20 - 4, this);
                    filedetails->copy_label("FileDetails");
                    filedetails->labeltype(FL_NO_LABEL);
                    filedetails->end();
                }
                filescroll->end();
                {
                    filecolumns = new FileColumns(
                        fileGroup->x() + 2, fileGroup->y() + 2,
                        fileGroup->w() - 4, 20, this);
                    filecolumns->copy_label("FileColumns");
                    filecolumns->labeltype(FL_NO_LABEL);
                    filecolumns->end();
                }
            }
            fileDetailsGroup->end();
        }
        fileDetailsGroup->resizable(filecolumns);

        fileGroup->resizable(filelist);
    }
    fileGroup->end();

    filePattern = new Flu_Combo_List(
        70, h() - 30, w() - 70 - 85 - 10, 25, _(fileTypesTxt.c_str()));
    filePattern->labelcolor(FL_WHITE);
    filePattern->editable(false);
    filePattern->callback(reloadCB, this);
    filePattern->pop_height(200);

    ok.callback(_okCB, this);
    cancel.callback(_cancelCB, this);

    {
        g = new Fl_Group(0, h() - 60, w(), 30);
        g->end();
        g->add(filename);
        g->add(ok);
        g->resizable(filename);
        g = new Fl_Group(0, h() - 30, w(), 30);
        g->end();
        g->add(filePattern);
        g->add(cancel);
        g->resizable(filePattern);
    }

    end();

    FL_NORMAL_SIZE = oldNormalSize;

    char buf[1024];

    // try to load the favorites
    {
        FILE* f = fl_fopen(configFilename.c_str(), "r");
        if (f)
        {
            buf[0] = '\0';
            while (!feof(f))
            {
                char* err = fgets(buf, 1024, f);
                char* newline = strrchr(buf, '\n');
                if (newline)
                    *newline = '\0';
                if (strlen(buf) > 0)
                {
                    // eliminate duplicates
                    bool duplicate = false;
                    for (int i = 1; i <= favoritesList->size(); i++)
                    {
                        if (streq(buf, favoritesList->text(i)))
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                    {
                        fs::path p(buf);
                        if (!fs::exists(p))
                            continue;

                        favoritesList->add(buf);
                        std::string favs = "/";
                        favs += _(favoritesTxt.c_str());
                        favs += "/";
                        favs += mrv::string::commentCharacter(buf);
                        location->tree.add(favs.c_str());
                    }
                }
            }
            fclose(f);
        }
    }

    pattern(pat);
    default_file_icon(&default_file);

    cd(nullptr); // prime with the current directory

    clear_history();

    cd(pathname);

    // if pathname does not start with "/" or "~", set the filename to it
    if (pathname && pathname[0] != '/' && pathname[0] != '~' &&
        (strlen(pathname) < 2 || pathname[1] != ':'))
    {
        filename.value(pathname);
    }
}

Flu_File_Chooser::~Flu_File_Chooser()
{
    cancelThumbnailRequests();

    // Fl::remove_timeout( Flu_Entry::_editCB );
    Fl::remove_timeout(Flu_File_Chooser::delayedCdCB);
    Fl::remove_timeout(Flu_File_Chooser::selectCB);

    for (int i = 0; i < locationQuickJump->children(); i++)
        free((void*)locationQuickJump->child(i)->label());

    filelist->clear();
    filedetails->clear();

    clear_history();
}

void Flu_File_Chooser::hideCB()
{
    // the user hid the browser by pushing the "X"
    // this is the same as cancel
    cancelCB();
}

void Flu_File_Chooser::cancelThumbnailRequests()
{
    TLRENDER_P();
    Fl_Group* g = getEntryGroup();
    int c = g->children();
    for (int i = 0; i < c; ++i)
    {
        Flu_Entry* e = static_cast<Flu_Entry*>(g->child(i));
        e->cancelRequest();
    }
}

void Flu_File_Chooser::cancelCB()
{
    cancelThumbnailRequests();
    filename.value("");
    filename.insert_position(filename.size(), filename.size());
    unselect_all();
    do_callback();
    hide();
}

void Flu_File_Chooser::do_callback()
{
    if (_callback)
    {
        _callback(this, _userdata);
    }
}

void Flu_File_Chooser::pattern(const char* p)
{
    // just like in Fl_File_Chooser, we accept tab, |, and ; delimited strings
    // like this: "Description (patterns)" or just "patterns" where patterns is
    // of the form *.xxx or *.{xxx,yyy,zzz}}

    rawPattern = p;

    // clear out the old
    filePattern->list.clear();
    filePattern->input.value("");
    patterns.clear();

    if (p == 0)
        p = "*";
    else if (p[0] == '\0')
        p = "*";

    std::string pat = p, pattern;

    bool addedAll = false;
    const char* next = strtok((char*)pat.c_str(), "\t|;");
    const char* start;
    while (next)
    {
        if (next[0] == '\0')
            break;

        // eat whitespace
        while (isspace(*next))
            next++;

        // degenerate check
        if (strcmp(next, "*") == 0)
        {
            addedAll = true;
            filePattern->list.add(allFilesTxt.c_str());
            patterns.push_back("*");
            next = strtok(nullptr, "\t|;");
            continue;
        }

        // extract the patterns from the substring
        if (next[0] != '*') // starts with description
        {
            // the pattern starts after the first '('
            start = strchr(next, '(');
            if (!start) // error: couldn't find the '('
            {
                next = strtok(nullptr, "\t|;");
                continue;
            }
            start++; // skip the '('
        }
        else
            start = next;

        if (start[0] != '*')
        {
            next = strtok(nullptr, "\t|;");
            continue;
        }
        start++; // skip the '*'

        if (start[0] != '.')
        {
            next = strtok(nullptr, "\t|;");
            continue;
        }
        start++; // skip the '.'

        if (start[0] == '{')
        {
            // the pattern is between '{' and '}'
            pattern = start + 1;
        }
        else
            pattern = start;

        // remove the last '}'
        size_t brace = pattern.find('}');
        if (brace != std::string::npos)
            pattern[brace] = '\0';

        // remove the last ')'
        size_t paren = pattern.find(')');
        if (paren != std::string::npos)
            pattern[paren] = '\0';

        if (pattern.size())
        {
            // add the whole string to the list
            filePattern->list.add(next);
            patterns.push_back(pattern);
        }

        // advance to the pattern token
        next = strtok(nullptr, "\t|;");
    }

    // add all files
    if (!addedAll)
    {
        filePattern->list.add(allFilesTxt.c_str());
        patterns.push_back("*");
    }

    // choose the first added item
    filePattern->value(filePattern->list.text(1));
}

int Flu_File_Chooser::handle(int event)
{
    TLRENDER_P();

    if (Fl_Double_Window::callback() != _hideCB)
    {
        _callback = Fl_Double_Window::callback();
        _userdata = Fl_Double_Window::user_data();
        Fl_Double_Window::callback(_hideCB, this);
    }

    if (event == FL_HIDE)
    {
    }
    else if (event == FL_SHOW)
    {
    }

    if (Fl_Double_Window::handle(event))
        return 1;
    else if (event == FL_KEYDOWN && Fl::event_key(FL_Escape))
    {
        cancel.do_callback();
        return 1;
    }
    else if (
        event == FL_KEYDOWN && Fl::event_key('a') && Fl::event_state(FL_CTRL))
    {
        select_all();
        return 1;
    }
    else
        return 0;
}

void Flu_File_Chooser::newFolderCB()
{
    // start with the name "New Folder". while the name exists, keep appending a
    // number (1..2..etc)
    std::string newName = defaultFolderNameTxt.c_str(),
                path = currentDir + newName;
    int count = 1;
    int i;
    for (;;)
    {
        bool found = false;
        // see if any entry already has that name
        Fl_Group* g = getEntryGroup();
        for (i = 0; i < g->children(); i++)
        {
            if (((Flu_Entry*)g->child(i))->filename == newName)
            {
                found = true;
                break;
            }
        }

        // since an entry already exists, change the name and try again
        if (found)
        {
            char buf[16];
            snprintf(buf, 16, "%d", count++);
            newName = defaultFolderNameTxt.c_str() + std::string(buf);
            path = currentDir + newName;
        }
        else
            break;
    }

    // try to create the folder
#if (defined _WIN32 || defined MINGW) && !defined CYGWIN
    if (_mkdir(path.c_str()) != 0)
#else
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
#endif
    {
        mrv::fl_alert(createFolderErrTxt.c_str(), newName.c_str());
        return;
    }

    // create a new entry with the name of the new folder. add to either the
    // list or the details at the end of directories and scroll to it.
    Flu_Entry* entry = new Flu_Entry(
        newName.c_str(), ENTRY_DIR, fileDetailsBtn->value(), this);

    i = 0;
    if (fileDetailsBtn->value())
    {
        for (; i < filedetails->children(); ++i)
        {
            Flu_Entry* e = (Flu_Entry*)filedetails->child(i);
            if (e->type != ENTRY_DIR)
                break;
        }
        filedetails->insert(*entry, i);
    }
    else
    {
        for (; i < filelist->children(); ++i)
        {
            Flu_Entry* e = (Flu_Entry*)filelist->child(i);
            if (e->type != ENTRY_DIR)
                break;
        }
        filelist->insert(*entry, i);
    }

    // Update the entry sizes (so scrollbar is updated)
    updateEntrySizes();

    // switch that entry to input mode and scroll the browser to it
    entry->editCB();

    // Only scroll if the entry is not visible already.
    if (!entry->visible())
    {
        if (!fileDetailsBtn->value())
            filelist->scroll_to(entry);
        else
            filedetails->scroll_to(entry);
    }
}

void Flu_File_Chooser::recursiveScan(const char* dir, FluStringVector* files)
{
    dirent** e;
    char* name;
    std::string fullpath;
    int num = fl_filename_list(dir, &e);
    for (int i = 0; i < num; i++)
    {
        name = e[i]->d_name;

        // if 'name' ends in '/' or '\', remove it
        if (name[strlen(name) - 1] == '/' || name[strlen(name) - 1] == '\\')
            name[strlen(name) - 1] = '\0';

        // ignore the "." and ".." names
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        // file or directory?
        fullpath = dir;
        fullpath += "/";
        fullpath += name;
        if (fl_filename_isdir(fullpath.c_str()))
            recursiveScan(fullpath.c_str(), files);

        files->push_back(fullpath);
    }
    files->push_back(dir);

    fl_filename_free_list(&e, num);
}

void Flu_File_Chooser::trashCB(bool recycle)
{
    // linux doesn't have a recycle bin
#ifndef _WIN32
    recycle = false;
#endif

    bool inFavorites = (currentDir == FAVORITES_UNIQUE_STRING);
    if (inFavorites)
        recycle = false;

    // see how many files are selected
    std::string name;
    int selected = 0;
    int i;
    const char* first = "";

    Fl_Group* g = getEntryGroup();
    for (i = 0; i < g->children(); i++)
    {
        if (((Flu_Entry*)g->child(i))->selected)
        {
            if (selected == 0)
                first = ((Flu_Entry*)g->child(i))->filename.c_str();
            selected++;
        }
    }

    if (selected)
    {
        if (selected == 1)
        {
            if (recycle)
            {
                if (!mrv::fl_choice(
                        "Really send '%s' to the Recycle Bin?", "No", "Yes", 0,
                        first))
                    return;
            }
            else
            {
                if (!mrv::fl_choice(
                        "Really delete '%s'?", "No", "Yes", 0, first))
                    return;
            }
        }
        else
        {
            if (recycle)
            {
                if (!mrv::fl_choice(
                        "Really send these %d files to the Recycle Bin?", "No",
                        "Yes", 0, selected))
                    return;
            }
            else
            {
                if (!mrv::fl_choice(
                        "Really delete these %d files?", "No", "Yes", 0,
                        selected))
                    return;
            }
        }

        if (inFavorites)
        {
            for (i = 0; i < g->children();)
            {
                Flu_Entry* e = ((Flu_Entry*)g->child(i));
                if (e->selected)
                {
                    favoritesList->remove(i + 1);
                    g->remove(*e);
                    delete e;
                }
                else
                    i++;
            }
            // save the favorites
            FILE* f = fopen(configFilename.c_str(), "w");
            if (f)
            {
                for (i = 1; i <= favoritesList->size(); i++)
                    fprintf(f, "%s\n", favoritesList->text(i));
                fclose(f);
            }
            cd(FAVORITES_UNIQUE_STRING);
            return;
        }

#ifdef _WIN32
        SHFILEOPSTRUCT fileop;
        memset(&fileop, 0, sizeof(SHFILEOPSTRUCT));
        fileop.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
        if (recycle)
            fileop.fFlags |= FOF_ALLOWUNDO;
        fileop.wFunc = FO_DELETE;
        fileop.pTo = nullptr;
#endif

        for (i = 0; i < g->children(); i++)
        {
            if (((Flu_Entry*)g->child(i))->selected)
            {
                int result = 0;

                name = currentDir + ((Flu_Entry*)g->child(i))->filename;

                // if directory, recursively remove
                if (((Flu_Entry*)g->child(i))->type == ENTRY_DIR)
                {
                    // if we are recycling in windows, then the recursive part
                    // happens automatically
#ifdef _WIN32
                    if (!recycle)
#endif
                    {
                        Fl_Group::current(0);
                        Fl_Window* win = new Fl_Window(200, 100, "Notice");
                        Flu_Label* label = new Flu_Label(
                            30, 30, 150, 30, "Preparing to delete...");
                        win->end();
                        win->show();
                        Fl::check();
                        // recursively build a list of all files that will be
                        // deleted
                        FluStringVector files;
                        recursiveScan(name.c_str(), &files);
                        // delete all the files
                        label->label("Deleting files...");
                        for (unsigned int i = 0; i < files.size(); i++)
                        {
                            if (::remove(files[i].c_str()) != 0)
                            {
                                win->hide();
                                delete win;
                                cd("./");
                                return;
                            }
                        }
                        win->hide();
                        delete win;
                        Fl::check();
                        continue;
                    }
                }

#ifdef _WIN32
                // this moves files to the recycle bin, depending on the value
                // of 'recycle'
                {
                    size_t len = name.size();
                    char* buf = (char*)malloc(len + 2);
                    strcpy(buf, name.c_str());
                    buf[len + 1] = '\0'; // have to have 2 '\0' at the end
                    fileop.pFrom = buf;
                    result = SHFileOperation(&fileop);
                    free(buf);
                }
#else
                result = ::remove(name.c_str());
#endif

                // if remove fails, report an error
                if (result != 0)
                {
                    char error[2048];
                    error[0] = 0;
#ifdef _WIN32
                    if (result == ERROR_SHARING_VIOLATION)
                    {
                        snprintf(error, 2048,
                                 _("Another process using the file."));
                    }
#endif
                    
                    char buf[2048];
                    snprintf(buf, 2048, _(deleteFileErrTxt.c_str()),
                             name.c_str(), error);
                    mrv::fl_alert(buf, nullptr);
                    cd("./");
                    return;
                }
            }
        }

        // refresh this directory
        cd("./");
    }
}

void Flu_File_Chooser::updateLocationQJ()
{
    const char* path = location->value();
    for (int i = 0; i < locationQuickJump->children(); i++)
        free((void*)locationQuickJump->child(i)->label());
    locationQuickJump->clear();
    fl_font(location->input.textfont(), location->input.textsize());
    const char* next = path;
    const char* slash = strchr(next, '/');
    char* blank = strdup(path);
    int offset = 0;
    while (slash)
    {
        memset(blank, 0, strlen(path));
        slash++;
        memcpy(blank, next, slash - next);
        int w = 0, h = 0;
        fl_measure(blank, w, h);
        if (blank[0] == '/')
            w += Fl::box_dx(location->box());
        memset(blank, 0, strlen(path));
        memcpy(blank, path, slash - path);
        Fl_Button* b = new Fl_Button(
            locationQuickJump->x() + offset, locationQuickJump->y(), w,
            locationQuickJump->h(), strdup(blank));
        b->labeltype(FL_NO_LABEL);
        b->callback(_locationQJCB, this);
        offset += w;
        locationQuickJump->add(b);
        next = slash;
        slash = strchr(next, '/');
    }
    Fl_Button* b = new Fl_Button(
        locationQuickJump->x() + offset, locationQuickJump->y(), 1,
        locationQuickJump->h(), strdup(""));
    b->box(FL_NO_BOX);
    b->labeltype(FL_NO_LABEL);
    locationQuickJump->add(b);
    locationQuickJump->resizable(b);
    free(blank);
}

void Flu_File_Chooser::favoritesCB()
{
    cd(FAVORITES_UNIQUE_STRING);
}

void Flu_File_Chooser::myComputerCB()
{
    cd("/");
}

void Flu_File_Chooser::documentsCB()
{
    cd(userDocs.c_str());
}

Flu_File_Chooser::FileInput::FileInput(
    int x, int y, int w, int h, const char* l, Flu_File_Chooser* c) :
    Fl_Input(x, y, w, h, l)
{
    chooser = c;
}

Flu_File_Chooser::FileInput::~FileInput() {}

int Flu_File_Chooser::FileInput::handle(int event)
{

    if (event == FL_KEYDOWN)
    {
        if (Fl::event_key(FL_Tab))
        {
            chooser->filenameTabCallback = true;
            std::string v(value());
#ifdef _WIN32
            // turn "C:" into "C:\"
            if (v.size() >= 2)
                if (v[1] == ':' && v[2] == '\0')
                {
                    v += "/";
                    value(v.c_str());
                    position(size(), size());
                }
#endif
            chooser->delayedCd = v + "*";
            Fl::add_timeout(0.0f, Flu_File_Chooser::delayedCdCB, chooser);
            return 1;
        }
        else if (Fl::event_key(FL_Left))
        {
            if (Fl_Input::insert_position() == 0)
                return 1;
            else
                return Fl_Input::handle(event);
        }
        else if (Fl::event_key(FL_Right))
        {
            if (Fl_Input::insert_position() == (int)strlen(Fl_Input::value()))
                return 1;
            else
                return Fl_Input::handle(event);
        }
        else if (Fl::event_key(FL_Up) || Fl::event_key(FL_Down))
        {
            chooser->getEntryContainer()->take_focus();
            if (!chooser->lastSelected)
            {
                if (chooser->getEntryGroup()->children())
                {
                    Flu_Entry* e =
                        (Flu_Entry*)chooser->getEntryGroup()->child(0);
                    e->selected = true;
                    chooser->lastSelected = e;
                    e->redraw();
                }
            }
            return chooser->getEntryContainer()->handle(event);
        }
        else if ((Fl::event_key() == FL_Enter ||
                  Fl::event_key() == FL_KP_Enter))
        {
            return chooser->ok.handle(event);
        }
    }

    return Fl_Input::handle(event);
}

void Flu_File_Chooser::sortCB(Fl_Widget* w)
{
    // if the sort method is already selected, toggle the REVERSE bit
    if (w == detailNameBtn)
    {
        if (sortMethod & SORT_NAME)
            sortMethod ^= SORT_REVERSE;
        else
            sortMethod = SORT_NAME;
    }
    else if (w == detailSizeBtn)
    {
        if (sortMethod & SORT_SIZE)
            sortMethod ^= SORT_REVERSE;
        else
            sortMethod = SORT_SIZE;
    }
    else if (w == detailDateBtn)
    {
        if (sortMethod & SORT_DATE)
            sortMethod ^= SORT_REVERSE;
        else
            sortMethod = SORT_DATE;
    }
    else if (w == detailTypeBtn)
    {
        if (sortMethod & SORT_TYPE)
            sortMethod ^= SORT_REVERSE;
        else
            sortMethod = SORT_TYPE;
    }

    bool reverse = (sortMethod & SORT_REVERSE);
    detailNameBtn->label(_(detailTxt[0].c_str()));
    detailSizeBtn->label(_(detailTxt[1].c_str()));
    detailDateBtn->label(_(detailTxt[2].c_str()));
    detailTypeBtn->label(_(detailTxt[3].c_str()));
    switch (sortMethod & ~SORT_REVERSE)
    {
    case SORT_NAME:
        detailNameBtn->label(reverse ? dArrow[0].c_str() : uArrow[0].c_str());
        break;
    case SORT_SIZE:
        detailSizeBtn->label(reverse ? dArrow[1].c_str() : uArrow[1].c_str());
        break;
    case SORT_DATE:
        detailDateBtn->label(reverse ? dArrow[2].c_str() : uArrow[2].c_str());
        break;
    case SORT_TYPE:
        detailTypeBtn->label(reverse ? dArrow[3].c_str() : uArrow[3].c_str());
        break;
    }

    filelist->sort(filelist->countDirs());
    filedetails->sort(filedetails->countDirs());

    Fl_Group* g = getEntryGroup();
    unsigned num = g->children();
    for (unsigned i = 0; i < num; ++i)
    {
        Flu_Entry* c = (Flu_Entry*)g->child(i);
        c->set_colors();
    }
}

Flu_File_Chooser::CBTile::CBTile(
    int x, int y, int w, int h, Flu_File_Chooser* c) :
    Fl_Tile(x, y, w, h)
{
    chooser = c;
}

int Flu_File_Chooser::CBTile::handle(int event)
{

    if (event == FL_DRAG)
    {
        // the user is probably dragging to resize the columns
        // update the sizes for each entry
        chooser->updateEntrySizes();
        chooser->redraw();
    }
    return Fl_Tile::handle(event);
}

Flu_File_Chooser::FileColumns::FileColumns(
    int x, int y, int w, int h, Flu_File_Chooser* c) :
    Fl_Tile(x, y, w, h)
{
    chooser = c;

    W1 = int(float(w) * 0.45f);
    W2 = int(float(w) * 0.15f);
    W3 = int(float(w) * 0.15f);
    W4 = w - W1 - W2 - W3;

    Fl_Box* box = new Fl_Box(x + 50, y, w - 200, h);
    add(*box);
    c->detailNameBtn = new Flu_Button(x, y, W1, h, _(detailTxt[0].c_str()));
    c->detailNameBtn->align(FL_ALIGN_CLIP);
    c->detailNameBtn->callback(Flu_File_Chooser::_sortCB, c);
    {
        CBTile* tile = new CBTile(x + W1, y, W2 + W3 + W4, h, c);
        Fl_Box* box =
            new Fl_Box(tile->x() + 50, tile->y(), tile->w() - 150, tile->h());
        tile->add(*box);
        c->detailTypeBtn =
            new Flu_Button(x + W1, y, W2, h, _(detailTxt[3].c_str()));
        c->detailTypeBtn->align(FL_ALIGN_CLIP);
        c->detailTypeBtn->callback(Flu_File_Chooser::_sortCB, c);
        {
            CBTile* tile = new CBTile(x + W1 + W2, y, W3 + W4, h, c);
            Fl_Box* box = new Fl_Box(
                tile->x() + 50, tile->y(), tile->w() - 100, tile->h());
            tile->add(*box);
            c->detailSizeBtn =
                new Flu_Button(x + W1 + W2, y, W3, h, _(detailTxt[1].c_str()));
            c->detailSizeBtn->align(FL_ALIGN_CLIP);
            c->detailSizeBtn->callback(Flu_File_Chooser::_sortCB, c);
            c->detailDateBtn = new Flu_Button(
                x + W1 + W2 + W3, y, W4, h, _(detailTxt[2].c_str()));
            c->detailDateBtn->align(FL_ALIGN_CLIP);
            c->detailDateBtn->callback(Flu_File_Chooser::_sortCB, c);
            tile->end();
        }
        tile->end();
    }
    end();
}

Flu_File_Chooser::FileColumns::~FileColumns() {}

void Flu_File_Chooser::FileColumns::resize(int x, int y, int w, int h)
{
    // TODO resize the buttons/tiles according to their stored relative sizes
    Fl_Tile::resize(x, y, w, 20);
    chooser->filescroll->resize(
        x, y + 20, w, chooser->fileDetailsGroup->h() - 20);
    chooser->updateEntrySizes();
    chooser->redraw();
}

int Flu_File_Chooser::FileColumns::handle(int event)
{

    if (event == FL_DRAG)
    {
        // the user is probably dragging to resize the columns
        // update the sizes for each entry
        chooser->updateEntrySizes();
        chooser->redraw();
    }
    return Fl_Tile::handle(event);
}

void Flu_File_Chooser::filenameCB()
{
    if (Fl::callback_reason() == FL_REASON_CHANGED)
    {
        unselect_all();
        Fl_Group* g = getEntryGroup();
        int num = g->children();
        for (int i = 0; i < num; ++i)
        {
            Flu_Entry* c = (Flu_Entry*)g->child(i);
            if (c->filename == filename.value())
            {
                lastSelected = c;
                c->selected = true;
                c->redraw();
                break;
            }
        }
    }
    else
    {
        filenameEnterCallback = true;
        // cd( filename.value() );
        okCB();
    }
}

inline bool _isProbablyAPattern(const char* s)
{
    return (strpbrk(s, "*;|[]?") != nullptr);
}

void Flu_File_Chooser::okCB()
{
    cancelThumbnailRequests();
    // if exactly one directory is selected and we are not choosing directories,
    // cd to that directory.
    if (!(selectionType & DIRECTORY) && !(selectionType & STDFILE))
    {
        Fl_Group* g = getEntryGroup();
        std::string dir;
        int count = 0;
        for (int i = 0; i < g->children(); i++)
        {
            if (((Flu_Entry*)g->child(i))->selected)
            {
                count++;
                dir = ((Flu_Entry*)g->child(i))->filename;
            }
        }
        if (count == 1)
        {
            std::string path = currentDir + dir;
            if (fl_filename_isdir(path.c_str()))
            {
                delayedCd = path + "/";
                Fl::add_timeout(0.1f, Flu_File_Chooser::delayedCdCB, this);
                return;
            }
        }
    }

    // only hide if the filename is not blank or the user is choosing
    // directories, in which case use the current directory

    if (selectionType & DIRECTORY ||
        ((selectionType & STDFILE) &&
         fl_filename_isdir((currentDir + filename.value()).c_str())))
    {
#ifdef _WIN32
        if (strcmp(_(myComputerTxt.c_str()), filename.value()) == 0)
        {
            myComputerCB();
            return;
        }
#endif
        if (!(selectionType & MULTI))
        {
            if (strlen(filename.value()) != 0)
                cd(filename.value());
            filename.value(currentDir.c_str());
            filename.insert_position(filename.size());
        }
        do_callback();
        hide();
    }
    else
    {
        const char* file = filename.value();
        if (strlen(file) != 0)
        {
            if (mrv::file::isDirectory(file))
            {
                cd(file);
                filename.value("");
                return;
            }

            Fl_Group* g = getEntryGroup();
            Flu_Entry* e = nullptr;
            for (int i = 0; i < g->children(); i++)
            {
                e = (Flu_Entry*)g->child(i);
                if (e->selected)
                {
                    break;
                }
            }

            // prepend the path
            std::string fullname;
            if (selectionType & SAVING)
            {
                std::string file = filename.value();
                if (e && e->type == ENTRY_SEQUENCE && e->selected &&
                    e->filename == file)
                {
                    fullname = e->toTLRender();
                }
                else
                {
                    fullname = currentDir + file;
                }
            }
            else
            {
                fullname = e->toTLRender();
            }
            filename.value(fullname.c_str());
            filename.insert_position(filename.size());
            do_callback();
            hide();
        }
    }
}

void Flu_File_Chooser::homeCB()
{
#ifdef _WIN32
    cd("/");
#else
    cd(userHome.c_str());
#endif
}

void Flu_File_Chooser::desktopCB()
{
    cd(userDesktop.c_str());
}

#define QSCANL(field)                                                          \
    while (((Flu_Entry*)array[left])->field <                                  \
           ((Flu_Entry*)array[pivot])->field)                                  \
    left++
#define QSCANR(field)                                                          \
    while (((Flu_Entry*)array[right])->field >                                 \
           ((Flu_Entry*)array[pivot])->field)                                  \
    right--

#define RQSCANL(field)                                                         \
    while (((Flu_Entry*)array[left])->field >                                  \
           ((Flu_Entry*)array[pivot])->field)                                  \
    left++
#define RQSCANR(field)                                                         \
    while (((Flu_Entry*)array[right])->field <                                 \
           ((Flu_Entry*)array[pivot])->field)                                  \
    right--

#define CASE_QSCANL(field)                                                     \
    while (strcasecmp(                                                         \
               ((Flu_Entry*)array[left])->field.c_str(),                       \
               ((Flu_Entry*)array[pivot])->field.c_str()) < 0)                 \
    left++
#define CASE_QSCANR(field)                                                     \
    while (strcasecmp(                                                         \
               ((Flu_Entry*)array[right])->field.c_str(),                      \
               ((Flu_Entry*)array[pivot])->field.c_str()) > 0)                 \
    right--

#define CASE_RQSCANL(field)                                                    \
    while (strcasecmp(                                                         \
               ((Flu_Entry*)array[left])->field.c_str(),                       \
               ((Flu_Entry*)array[pivot])->field.c_str()) > 0)                 \
    left++
#define CASE_RQSCANR(field)                                                    \
    while (strcasecmp(                                                         \
               ((Flu_Entry*)array[right])->field.c_str(),                      \
               ((Flu_Entry*)array[pivot])->field.c_str()) < 0)                 \
    right--

#define CUSTOM_QSCANL(field)                                                   \
    while (customSort(                                                         \
               ((Flu_Entry*)array[left])->field,                               \
               ((Flu_Entry*)array[pivot])->field) < 0)                         \
    left++
#define CUSTOM_QSCANR(field)                                                   \
    while (customSort(                                                         \
               ((Flu_Entry*)array[right])->field,                              \
               ((Flu_Entry*)array[pivot])->field) > 0)                         \
    right--

#define CUSTOM_RQSCANL(field)                                                  \
    while (customSort(                                                         \
               ((Flu_Entry*)array[left])->field,                               \
               ((Flu_Entry*)array[pivot])->field) > 0)                         \
    left++
#define CUSTOM_RQSCANR(field)                                                  \
    while (customSort(                                                         \
               ((Flu_Entry*)array[right])->field,                              \
               ((Flu_Entry*)array[pivot])->field) < 0)                         \
    right--

void Flu_File_Chooser::_qSort(
    int how, bool caseSort, Fl_Widget** array, int low, int high)
{
    int left, right, pivot;
    Fl_Widget* temp;
    bool reverse = (how & SORT_REVERSE);

    if (high > low)
    {
        left = low;
        right = high;
        pivot = low;

        while (right >= left)
        {
            switch (how & ~SORT_REVERSE)
            {
            case SORT_NAME:
                if (reverse)
                {
                    if (customSort)
                    {
                        CUSTOM_RQSCANL(filename.c_str());
                        CUSTOM_RQSCANR(filename.c_str());
                    }
                    else if (!caseSort)
                    {
                        CASE_RQSCANL(filename);
                        CASE_RQSCANR(filename);
                    }
                    else
                    {
                        RQSCANL(filename);
                        RQSCANR(filename);
                    }
                }
                else
                {
                    if (customSort)
                    {
                        CUSTOM_QSCANL(filename.c_str());
                        CUSTOM_QSCANR(filename.c_str());
                    }
                    else if (!caseSort)
                    {
                        CASE_QSCANL(filename);
                        CASE_QSCANR(filename);
                    }
                    else
                    {
                        QSCANL(filename);
                        QSCANR(filename);
                    }
                }
                break;
            case SORT_SIZE:
                if (reverse)
                {
                    RQSCANL(isize);
                    RQSCANR(isize);
                }
                else
                {
                    QSCANL(isize);
                    QSCANR(isize);
                }
                break;
            case SORT_DATE:
                if (reverse)
                {
                    RQSCANL(idate);
                    RQSCANR(idate);
                }
                else
                {
                    QSCANL(idate);
                    QSCANR(idate);
                }
                break;
            case SORT_TYPE:
                if (reverse)
                {
                    RQSCANL(description);
                    RQSCANR(description);
                }
                else
                {
                    QSCANL(description);
                    QSCANR(description);
                }
                break;
            }

            if (left > right)
                break;

            temp = array[left];
            array[left] = array[right];
            array[right] = temp;
            left++;
            right--;
        }

        _qSort(how, caseSort, array, low, right);
        _qSort(how, caseSort, array, left, high);
    }
}

Flu_File_Chooser::FileList::FileList(
    int x, int y, int w, int h, Flu_File_Chooser* c) :
    Flu_Wrap_Group(x, y, w, h)
{
    chooser = c;
    numDirs = 0;
}

Flu_File_Chooser::FileList::~FileList() {}

int Flu_File_Chooser::FileList::countDirs()
{
    numDirs = 0;
    for (int i = 0; i < children(); ++i)
    {
        Flu_Entry* e = static_cast<Flu_Entry*>(child(i));
        if (e->type == ENTRY_DIR)
            ++numDirs;
    }
    return numDirs;
}

void Flu_File_Chooser::FileList::sort(int n)
{
    if (n != -1)
        numDirs = n;
    if (children() == 0)
        return;
    // the directories are already first. sort the directories then the names
    // lexigraphically
    Flu_File_Chooser::_qSort(
        chooser->sortMethod, chooser->caseSort, (Fl_Widget**)array(), 0,
        numDirs - 1);
    Flu_File_Chooser::_qSort(
        chooser->sortMethod, chooser->caseSort, (Fl_Widget**)array(), numDirs,
        children() - 1);
    chooser->redraw();
}

int Flu_File_Chooser::FileList::handle(int event)
{

    if (event == FL_FOCUS || event == FL_UNFOCUS)
        return 1;

    if (Flu_Wrap_Group::handle(event))
        return 1;

    // if push on no file, unselect all files and turn off editing mode
    if (event == FL_PUSH && !Fl::event_key(FL_SHIFT) && !Fl::event_key(FL_CTRL))
    {

        chooser->unselect_all();
        chooser->filename.value("");
        chooser->filename.insert_position(
            chooser->filename.size(), chooser->filename.size());

        if (Fl::event_button3())
            return chooser->popupContextMenu(nullptr);

        return 1;
    }
    else if (event == FL_KEYDOWN)
    {
        if (Fl::event_key(FL_Delete))
        {
            // recycle by default, unless the shift key is held down
            chooser->trashCB(!Fl::event_state(FL_SHIFT));
            return 1;
        }

        Flu_Entry* e = chooser->lastSelected;
        if (!e)
        {

            for (int i = 0; i < children(); i++)
                if (((Flu_Entry*)child(i))->selected)
                {
                    e = (Flu_Entry*)child(i);
                    break;
                }
        }
        if (e)
        {

            switch (Fl::event_key())
            {
            case FL_Up:
                e = (Flu_Entry*)previous(e);
                if (!e && children())
                    e = (Flu_Entry*)child(0);
                break;
            case FL_Down:
                e = (Flu_Entry*)next(e);
                if (!e && children())
                    e = (Flu_Entry*)child(children() - 1);
                break;
            case FL_Left:
                e = (Flu_Entry*)left(e);
                break;
            case FL_Right:
                e = (Flu_Entry*)right(e);
                break;
            case FL_Home:
                if (children())
                    e = (Flu_Entry*)child(0);
                break;
            case FL_End:
                if (children())
                    e = (Flu_Entry*)child(children() - 1);
                break;
            case FL_Enter:
                chooser->filenameEnterCallback = true;
                // chooser->cd( e->filename.c_str() );
                chooser->okCB();
                return 1;
            case ' ':
                chooser->cd(e->filename.c_str());
                return 1;
            default:
                e = 0;
                break;
            }
            if (e)
            {

                chooser->unselect_all();
                e->selected = true;
                chooser->lastSelected = e;
                chooser->filename.value(e->filename.c_str());
                chooser->filename.insert_position(
                    chooser->filename.size(), chooser->filename.size());
                chooser->redraw();
                scroll_to(e);
                return 1;
            }
        }
    }

    return 0;
}

Flu_File_Chooser::FileDetails::FileDetails(
    int x, int y, int w, int h, Flu_File_Chooser* c) :
    Flu_Pack(x, y, w, h)
{
    chooser = c;
    numDirs = 0;
}

Flu_File_Chooser::FileDetails::~FileDetails() {}

void Flu_File_Chooser::FileDetails::scroll_to(Fl_Widget* w)
{
    int H = 0;
    for (int i = 0; i < children(); ++i)
    {
        const Flu_Entry* o = static_cast<Flu_Entry*>(child(i));
        if (o == w)
        {
            int maxH = (int)chooser->filescroll->scrollbar.maximum() + 6;
            if (chooser->filescroll->hscrollbar.visible())
                maxH += (int)chooser->filescroll->hscrollbar.h();
            if (H > maxH)
                H = maxH;
            chooser->filescroll->scroll_to(0, H);
            return;
        }
        H += o->h();
    }
}

int Flu_File_Chooser::FileDetails::countDirs()
{
    numDirs = 0;
    for (int i = 0; i < children(); ++i)
    {
        Flu_Entry* e = static_cast<Flu_Entry*>(child(i));
        if (e->type == ENTRY_DIR)
            ++numDirs;
    }
    return numDirs;
}

void Flu_File_Chooser::FileDetails::sort(int n)
{
    if (n != -1)
        numDirs = n;
    if (children() == 0)
        return;
    // the directories are already first. sort the directories then the names
    // lexigraphically
    Flu_File_Chooser::_qSort(
        chooser->sortMethod, chooser->caseSort, (Fl_Widget**)array(), 0,
        numDirs - 1);
    Flu_File_Chooser::_qSort(
        chooser->sortMethod, chooser->caseSort, (Fl_Widget**)array(), numDirs,
        children() - 1);
    chooser->redraw();
}

Fl_Widget* Flu_File_Chooser::FileDetails::next(Fl_Widget* w)
{
    for (int i = 0; i < children() - 1; i++)
    {
        if (w == child(i))
            return child(i + 1);
    }
    return nullptr;
}

Fl_Widget* Flu_File_Chooser::FileDetails::previous(Fl_Widget* w)
{
    for (int i = 1; i < children(); i++)
    {
        if (w == child(i))
            return child(i - 1);
    }
    return nullptr;
}

int Flu_File_Chooser::FileDetails::handle(int event)
{
    if (Flu_Pack::handle(event))
        return 1;
    else if (event == FL_PUSH)
        return 1;

    else if (event == FL_KEYDOWN)
    {
        if (Fl::event_key(FL_Delete))
        {
            // recycle by default, unless the shift key is held down
            chooser->trashCB(!Fl::event_state(FL_SHIFT));
            return 1;
        }

        Flu_Entry* e = chooser->lastSelected;
        if (!e)
        {
            for (int i = 0; i < children(); i++)
                if (((Flu_Entry*)child(i))->selected)
                {
                    e = (Flu_Entry*)child(i);
                    break;
                }
        }
        if (e)
        {
            switch (Fl::event_key())
            {
            case FL_Up:
                e = (Flu_Entry*)previous(e);
                if (!e && children())
                    e = (Flu_Entry*)child(0);
                break;
            case FL_Down:
                e = (Flu_Entry*)next(e);
                if (!e && children())
                    e = (Flu_Entry*)child(children() - 1);
                break;
            case FL_Home:
                if (children())
                    e = (Flu_Entry*)child(0);
                break;
            case FL_End:
                if (children())
                    e = (Flu_Entry*)child(children() - 1);
                break;
            case FL_Enter:
                chooser->filenameEnterCallback = true;
                // chooser->cd( e->filename.c_str() );
                chooser->okCB();
                return 1;
            case ' ':
                chooser->cd(e->filename.c_str());
                return 1;
            default:
                e = 0;
                break;
            }
            if (e)
            {
                chooser->unselect_all();
                e->selected = true;
                chooser->lastSelected = e;
                chooser->filename.value(e->filename.c_str());
                chooser->filename.insert_position(
                    chooser->filename.size(), chooser->filename.size());
                chooser->redraw();
                scroll_to(e);
                return 1;
            }
        }
    }

    return 0;
}

void Flu_File_Chooser::resize(int x, int y, int w, int h)
{
    Fl_Double_Window::resize(x, y, w, h);
    if (fileListWideBtn->value())
        filelist->scrollbar.linesize(filelist->w());
    else if (fileListBtn->value())
        filelist->scrollbar.linesize(DEFAULT_ENTRY_WIDTH + 4);
    // round position to nearest multiple of the linesize
    ((Fl_Valuator*)&(filelist->scrollbar))
        ->value(filelist->w() * (filelist->scrollbar.value() / filelist->w()));
    for (int i = 0; i < filelist->children(); i++)
        ((Flu_Entry*)filelist->child(i))->updateSize();
}

void Flu_File_Chooser::listModeCB()
{
    bool listMode =
        !fileDetailsBtn->value() || (currentDir == FAVORITES_UNIQUE_STRING);
    if (listMode)
    {
        while (filedetails->children())
            filelist->add(filedetails->child(0));
    }
    else
    {
        while (filelist->children())
            filedetails->add(filelist->child(0));
    }

    resize(x(), y(), w(), h());
    updateEntrySizes();
    if (listMode)
    {
        fileDetailsGroup->hide();
        filelist->show();
        filelist->redraw();
        filelist->parent()->resizable(filelist);
    }
    else
    {
        filelist->hide();
        fileDetailsGroup->show();
        fileDetailsGroup->parent()->resizable(fileDetailsGroup);
    }
    // redraw();
}

Fl_Group* Flu_File_Chooser::getEntryGroup()
{
    return (!fileDetailsBtn->value() || currentDir == FAVORITES_UNIQUE_STRING)
               ? &(filelist->group)
               : filedetails;
}

Fl_Group* Flu_File_Chooser::getEntryContainer()
{
    return (!fileDetailsBtn->value() || currentDir == FAVORITES_UNIQUE_STRING)
               ? (Fl_Group*)filelist
               : filedetails;
}

static const int kColorOne = fl_rgb_color(200, 200, 200);
static const int kColorTwo = fl_rgb_color(180, 180, 180);

int Flu_File_Chooser::popupContextMenu(Flu_Entry* entry)
{
    int type = entry ? entry->type : ENTRY_NONE;
    const char* filename = entry ? entry->filename.c_str() : nullptr;
    char* ext = nullptr;

    if (filename)
        ext = const_cast<char*>(strrchr(filename, '.'));
    if (ext)
    {
        ext = strdup(ext + 1); // skip the '.'
        for (unsigned int i = 0; i < strlen(ext); i++)
            ext[i] = tolower(ext[i]);
    }

    enum { ACTION_NEW_FOLDER = -1, ACTION_RENAME = -2, ACTION_DELETE = -3 };

    Fl_Group::current(0);
    Fl_Menu_Button entryPopup(0, 0, 0, 0);
    entryPopup.type(Fl_Menu_Button::POPUP3);
    entryPopup.clear();
    switch (type)
    {
    case ENTRY_NONE: // right click on nothing
        entryPopup.add(
            _(contextMenuTxt[0].c_str()), 0, 0, (void*)ACTION_NEW_FOLDER);
        break;

    case ENTRY_DIR:
        entryPopup.add(
            _(contextMenuTxt[1].c_str()), 0, 0, (void*)ACTION_RENAME);
        entryPopup.add(
            _(contextMenuTxt[2].c_str()), 0, 0, (void*)ACTION_DELETE);
        break;

    case ENTRY_FILE:
        entryPopup.add(
            _(contextMenuTxt[1].c_str()), 0, 0, (void*)ACTION_RENAME);
        entryPopup.add(
            _(contextMenuTxt[2].c_str()), 0, 0, (void*)ACTION_DELETE);
        break;

    case ENTRY_FAVORITE:
        entryPopup.add(
            _(contextMenuTxt[2].c_str()), 0, 0, (void*)ACTION_DELETE);
        break;

    case ENTRY_DRIVE:
        break;

    case ENTRY_MYDOCUMENTS:
        break;

    case ENTRY_MYCOMPUTER:
        break;

    case ENTRY_SEQUENCE:
        break;
    }

    // add the programmable context handlers
    for (size_t i = 0; i < contextHandlers.size(); i++)
    {
        if (!(contextHandlers[i].type & type))
            continue;
        if (type == ENTRY_FILE || type == ENTRY_SEQUENCE)
            if (contextHandlers[i].ext.size() && contextHandlers[i].ext != ext)
                continue;
        entryPopup.add(_(contextHandlers[i].name.c_str()), 0, 0, (void*)i);
    }
    if (ext)
        free(ext);

    const Fl_Menu_Item* selection = entryPopup.popup();
    if (selection)
    {

        int handler = (intptr_t)(selection->user_data());

        switch (handler)
        {
        case ACTION_NEW_FOLDER:
            newFolderCB();
            break;
        case ACTION_RENAME:
            entry->editCB();
            break;
        case ACTION_DELETE:
            // recycle by default, unless the shift key is held down
            trashCB(!Fl::event_state(FL_SHIFT));
            break;
        default:
            contextHandlers[handler].callback(
                filename, type, contextHandlers[handler].callbackData);
            break;
        }
    }
    else
        return handle(FL_PUSH);
    return 1;
}

void Flu_File_Chooser::unselect_all()
{
    Fl_Group* g = getEntryGroup();
    Flu_Entry* e;
    for (int i = 0; i < g->children(); i++)
    {
        e = ((Flu_Entry*)g->child(i));
        e->selected = false;
        e->editMode = 0;
        e->set_colors();
    }
    lastSelected = 0;
    trashBtn->deactivate();
    redraw();
}

void Flu_File_Chooser::select_all()
{
    if (!(selectionType & MULTI))
        return;
    Fl_Group* g = getEntryGroup();
    Flu_Entry* e;
    for (int i = 0; i < g->children(); i++)
    {
        e = ((Flu_Entry*)g->child(i));
        e->selected = true;
        e->editMode = 0;
        filename.value(e->filename.c_str());
    }
    lastSelected = 0;
    trashBtn->deactivate();
    redraw();
}

void Flu_File_Chooser::updateEntrySizes()
{
    int i;
    filecolumns->W1 = detailNameBtn->w();
    filecolumns->W2 = detailTypeBtn->w();
    filecolumns->W3 = detailSizeBtn->w();
    filecolumns->W4 = detailDateBtn->w();

    for (i = 0; i < filedetails->children(); ++i)
    {
        Flu_Entry* e = ((Flu_Entry*)filedetails->child(i));
        e->Fl_Widget::position(0, filedetails->y() + e->y());
    }

    // update the size of each entry because the user changed the size of each
    // column
    filedetails->resize(
        filedetails->x(), filedetails->y(), filescroll->w(), filedetails->h());
    for (i = 0; i < filedetails->children(); ++i)
        ((Flu_Entry*)filedetails->child(i))->updateSize();
    for (i = 0; i < filelist->children(); ++i)
        ((Flu_Entry*)filelist->child(i))->updateSize();
}

const char* Flu_File_Chooser::value()
{
    if (filename.size() == 0)
        return nullptr;
    else
    {
        return filename.value();
    }
}

int Flu_File_Chooser::count()
{
    if (selectionType & MULTI)
    {
        int n = 0;
        Fl_Group* g = getEntryGroup();
        for (int i = 0; i < g->children(); i++)
        {
#ifdef _WIN32
            if (((Flu_Entry*)g->child(i))->filename == myComputerTxt)
                continue;
#endif
            if (((Flu_Entry*)g->child(i))->selected ||
                (currentDir + ((Flu_Entry*)g->child(i))->filename) ==
                    filename.value())
            {
                ((Flu_Entry*)g->child(i))->selected = true;
                n++;
            }
        }
        return n;
    }
    else
    {
        return (strlen(filename.value()) == 0) ? 0 : 1;
    }
}

void Flu_File_Chooser::value(const char* v)
{
    cd(v);
    if (!v)
        return;
    // try to find the file and select it
    const char* slash = strrchr(v, '/');
    if (slash)
        slash++;
    else
    {
        slash = strrchr(v, '\\');
        if (slash)
            slash++;
        else
            slash = v;
    }
    filename.value(slash);
    filename.insert_position(filename.size(), filename.size());
    Fl_Group* g = getEntryGroup();
    for (int i = 0; i < g->children(); i++)
    {
        if (((Flu_Entry*)g->child(i))->filename == slash)
        {
            ((Flu_Entry*)g->child(i))->selected = true;
            filelist->scroll_to((Flu_Entry*)g->child(i));
            filedetails->scroll_to((Flu_Entry*)g->child(i));
            redraw();
            return;
        }
    }
}

const char* Flu_File_Chooser::value(int n)
{
    Fl_Group* g = getEntryGroup();
    for (int i = 0; i < g->children(); i++)
    {
        Flu_Entry* e = (Flu_Entry*)g->child(i);
#ifdef _WIN32
        if (e->filename == myComputerTxt)
            continue;
#endif
        if (e->selected)
        {
            n--;
            if (n == 0)
            {
                std::string s = e->toTLRender();
                filename.value(s.c_str());
                filename.insert_position(filename.size());
                return value();
            }
        }
    }
    return "";
}

void Flu_File_Chooser::reloadCB()
{
#ifdef _WIN32
    refreshDrives = true;
#endif
    cd(currentDir.c_str());
}

void Flu_File_Chooser::addToFavoritesCB()
{
    // eliminate duplicates
    bool duplicate = false;
    for (int i = 1; i <= favoritesList->size(); i++)
    {
        if (streq(currentDir.c_str(), favoritesList->text(i)))
        {
            duplicate = true;
            break;
        }
    }
    if (!duplicate)
    {
        favoritesList->add(currentDir.c_str());
        location->tree.add(mrv::string::commentCharacter(currentDir).c_str());
    }

    // save the favorites
    FILE* f = fopen(configFilename.c_str(), "w");
    if (f)
    {
        for (int i = 1; i <= favoritesList->size(); i++)
            fprintf(f, "%s\n", favoritesList->text(i));
        fclose(f);
    }
}

std::string Flu_File_Chooser::formatDate(const char* d)
{
    if (d == 0)
    {
        std::string s;
        return s;
    }

    // convert style "Wed Mar 19 07:23:11 2003" to "MM/DD/YY HH:MM AM|PM"

    int month, day, year, hour, minute, second;
    bool pm;
    char MM[16], dummy[64];

    sscanf(
        d, "%s %s %d %d:%d:%d %d", dummy, MM, &day, &hour, &minute, &second,
        &year);

    pm = (hour >= 12);
    if (hour == 0)
        hour = 12;
    if (hour >= 13)
        hour -= 12;

    if (strcmp(MM, _("Jan")) == 0)
        month = 1;
    else if (strcmp(MM, _("Feb")) == 0)
        month = 2;
    else if (strcmp(MM, _("Mar")) == 0)
        month = 3;
    else if (strcmp(MM, _("Apr")) == 0)
        month = 4;
    else if (strcmp(MM, _("May")) == 0)
        month = 5;
    else if (strcmp(MM, _("Jun")) == 0)
        month = 6;
    else if (strcmp(MM, _("Jul")) == 0)
        month = 7;
    else if (strcmp(MM, _("Aug")) == 0)
        month = 8;
    else if (strcmp(MM, _("Sep")) == 0)
        month = 9;
    else if (strcmp(MM, _("Oct")) == 0)
        month = 10;
    else if (strcmp(MM, _("Nov")) == 0)
        month = 11;
    else if (strcmp(MM, _("Dec")) == 0)
        month = 11;
    else
        month = 12;

    snprintf(
        dummy, 64, "%d/%d/%02d %d:%02d %s", month, day, year, hour, minute,
        pm ? "PM" : "AM");

    std::string formatted = dummy;

    return formatted;
}

void Flu_File_Chooser::win2unix(std::string& s)
{
    size_t len = s.size();
    for (size_t i = 0; i < len; i++)
        if (s[i] == '\\')
            s[i] = '/';
}

void Flu_File_Chooser::cleanupPath(std::string& s)
{
    // convert all '\' to '/'
    win2unix(s);

    std::string newS;
    newS.resize(s.size() + 1);

    size_t oldPos, newPos;
    for (oldPos = 0, newPos = 0; oldPos < s.size(); oldPos++)
    {
        // remove "./"
        if (s[oldPos] == '.' && s[oldPos + 1] == '/')
            oldPos += 2;

        // convert "//" to "/"
        else if (s[oldPos] == '/' && s[oldPos + 1] == '/')
            oldPos++;

#ifdef _WIN32
        // downcase "c:" to "C:"
        else if (s[oldPos + 1] == ':')
            s[oldPos] = toupper(s[oldPos]);
#endif

        // remove "../" by removing everything back to the last "/"
        if (oldPos + 2 < s.size()) // bounds check
        {
            if (s[oldPos] == '.' && s[oldPos + 1] == '.' &&
                s[oldPos + 2] == '/' && newS != "/")
            {
                // erase the last character, which should be a '/'
                newPos--;
                newS = newS.substr(0, newPos);
                // look for the previous '/'
                char* lastSlash = const_cast<char*>(strrchr(newS.c_str(), '/'));
                // make the new string position after the slash
                newPos = (lastSlash - newS.c_str()) + 1;
                oldPos += 3;
            }
        }

        newS[newPos] = s[oldPos];
        newPos++;
    }

    newS = newS.substr(0, newPos);
    s = newS;
}

void Flu_File_Chooser::backCB()
{
    if (!currentHist)
        return;
    if (currentHist->last)
    {
        currentHist = currentHist->last;
        walkingHistory = true;
        delayedCd = currentHist->path;
        Fl::add_timeout(0.0f, Flu_File_Chooser::delayedCdCB, this);
    }
}

void Flu_File_Chooser::forwardCB()
{
    if (!currentHist)
        return;
    if (currentHist->next)
    {
        currentHist = currentHist->next;
        walkingHistory = true;
        delayedCd = currentHist->path;
        Fl::add_timeout(0.0f, Flu_File_Chooser::delayedCdCB, this);
    }
}

bool Flu_File_Chooser::correctPath(std::string& path)
{
    // the path may or may not be an alias, needing corrected
#ifdef _WIN32
    // point to the correct desktop
    std::string desk = "/" + desktopTxt + "/";
    std::string comp = desk + myComputerTxt + "/";
    std::string usercomp = userDesktop + myComputerTxt + "/";
    std::string docs = desk + myDocumentsTxt + "/";
    std::string userdocs = userDesktop + myDocumentsTxt + "/";
    if (path == desk)
    {
        path = userDesktop;
        return true;
    }
    else if (path == userDesktop)
        return true;
    else if (path == comp || path == usercomp)
        path = "/";
    else if (path == docs || path == userdocs)
        path = userDocs;
#endif
    return false;
}

void Flu_File_Chooser::locationCB(const char* path)
{
#ifdef _WIN32
    std::string p = path;
    std::string favs = "/";
    favs += favoritesTxt;
    favs += "/";
    std::string mycomp = "/";
    mycomp += desktopTxt;
    mycomp += "/";
    mycomp += myComputerTxt;
    mycomp += "/";
    std::string mydocs = "/";
    mydocs += desktopTxt;
    mydocs += "/";
    mydocs += myDocumentsTxt;
    mydocs += "/";
    std::string desk = "/";
    desk += desktopTxt;
    desk += "/";
    if (p == favs)
        favoritesCB();
    else if (p == mycomp)
        myComputerCB();
    else if (p == mydocs)
        documentsCB();
    else if (p == desk)
        desktopCB();
    // if the path leads off with "/Desktop/My Computer", then strip that part
    // off and cd to the remaining
    else
    {
        std::string s = mycomp;
        if (strstr(path, s.c_str()) == path)
        {
            // search for '(' and if present, extract the drive name and cd to it
            char* paren = const_cast<char*>(strrchr(path, '('));
            if (paren)
            {
                char drive[] = "A:/";
                drive[0] = toupper(paren[1]);
                cd(drive);
            }
            else
            {
                cd(path + 21);
            }
        }
        else
        {
            cd(path);
        }
    }
#else
    cd(path);
#endif
    updateLocationQJ();
}

void Flu_File_Chooser::buildLocationCombo()
{
    // add all filesystems
    // location->tree.clear_children( location->tree.root() );

    Fl_Tree_Item* n;
#ifdef _WIN32
    std::string s;
    char volumeName[1024];
    s = mrv::string::commentCharacter("/" + desktopTxt + "/");
    n = location->tree.add(_(s.c_str()));
    if (n)
        n->usericon(&little_desktop);
    s = mrv::string::commentCharacter("/" + desktopTxt + "/" + myDocumentsTxt + "/");
    n = location->tree.add(_(s.c_str()));
    if (n)
        n->usericon(&documents);
    s = mrv::string::commentCharacter("/" + desktopTxt + "/" + myComputerTxt + "/");
    n = location->tree.add(_(s.c_str()));
    if (n)
        n->usericon(&computer);
    // get the location and add them
    {
        if (refreshDrives)
            driveMask = GetLogicalDrives();
        DWORD mask = driveMask;

        for (int i = 0; i < 26; i++)
        {
            drives[i] = "";
            driveIcons[i] = &disk_drive;
            if (mask & 1)
            {
                s = "/";
                s += _(desktopTxt.c_str());
                s += "/";
                s += myComputerTxt;
                s += "/";
                char drive[] = "A:";
                char windrive[] = "A:\\";
                windrive[0] = drive[0] = 'A' + i;
                DWORD type;
                if (refreshDrives)
                {
                    volumeName[0] = '\0';
                    type = driveTypes[i] = GetDriveType(windrive);
                    if (type != DRIVE_REMOVABLE && type != DRIVE_REMOTE)
                        GetVolumeInformation(
                            windrive, volumeName, 1024, nullptr, nullptr,
                            nullptr, nullptr, 0);
                    volumeNames[i] = volumeName;
                }
                else
                {
                    strncpy(volumeName, volumeNames[i].c_str(), 1024);
                    type = driveTypes[i];
                }

                // s += volume
                const char* disk = "Disk";
                switch (type)
                {
                case DRIVE_REMOVABLE:
                    disk = strlen(volumeName)
                               ? volumeName
                               : (1 < 2 ? diskTypesTxt[0].c_str()
                                        : diskTypesTxt[1].c_str());
                    driveIcons[i] = &floppy_drive;
                    break;
                case DRIVE_FIXED:
                    disk = strlen(volumeName) ? volumeName
                                              : diskTypesTxt[2].c_str();
                    // driveIcons[i] = &disk_drive;
                    break;
                case DRIVE_CDROM:
                    disk = strlen(volumeName) ? volumeName
                                              : diskTypesTxt[3].c_str();
                    driveIcons[i] = &cd_drive;
                    break;
                case DRIVE_REMOTE:
                    disk = strlen(volumeName) ? volumeName
                                              : diskTypesTxt[4].c_str();
                    driveIcons[i] = &network_drive;
                    break;
                case DRIVE_RAMDISK:
                    disk = strlen(volumeName) ? volumeName
                                              : diskTypesTxt[5].c_str();
                    driveIcons[i] = &ram_drive;
                    break;
                }
                drives[i] =
                    std::string(disk) + " (" + std::string(drive) + ")/";
                s += mrv::string::commentCharacter(drives[i]);
                n = location->tree.add(s.c_str());
                if (n)
                    n->usericon(driveIcons[i]);
                // erase the trailing '/' to make things look nicer
                drives[i] = drives[i].substr(0, drives[i].size() - 1);
            }
            mask >>= 1;
        }
    }
    std::string favs = "/";
    favs += _(favoritesTxt.c_str());
    favs += "/";
    n = location->tree.add(_(favs.c_str()));
    if (n)
        n->usericon(&little_favorites);
    refreshDrives = false;

#elif defined __APPLE__

    int i;
    // get all volume mount points and add to the location combobox
    dirent** e;
    char* name;
    int num = fl_filename_list("/Volumes/", &e);
    if (num > 0)
    {
        for (i = 0; i < num; i++)
        {
            name = e[i]->d_name;

            // ignore the "." and ".." names
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
                strcmp(name, "./") == 0 || strcmp(name, "../") == 0 ||
                strcmp(name, ".\\") == 0 || strcmp(name, "..\\") == 0)
                continue;

            // if 'name' ends in '/', remove it
            if (name[strlen(name) - 1] == '/')
                name[strlen(name) - 1] = '\0';

            std::string fullpath = "/Volumes/";
            fullpath += name;
            fullpath += "/";
            location->tree.add(mrv::string::commentCharacter(fullpath).c_str());
        }
    }

    fl_filename_free_list(&e, num);

#else

    // get all mount points and add to the location combobox
    FILE* fstab; // /etc/mtab or /etc/mnttab file
    char dummy[256], mountPoint[256], line[1024]; // Input line
    std::string mount;

    fstab = fopen("/etc/fstab", "r"); // Otherwise fallback to full list
    if (fstab)
    {
        while (fgets(line, 1024, fstab))
        {
            if (line[0] == '#' || line[0] == '\n')
                continue;

            // in fstab, mount point is second full string
            sscanf(line, "%s %s", dummy, mountPoint);
            mount = mountPoint;

            // cull some stuff
            if (mount[0] != '/')
                continue;
            if (mount == "/")
                continue;
            if (mount == "/boot")
                continue;
            if (mount == "/proc")
                continue;

            // now add the mount point
            mount += "/";
            location->tree.add(mrv::string::commentCharacter(mount).c_str());
        }

        fclose(fstab);
    }

    std::string favs = "/";
    favs += _(favoritesTxt.c_str());
    favs += "/";
    n = location->tree.find_item(_(favs.c_str()));
    if (n)
        n->usericon(&little_favorites);

#endif
}

void Flu_File_Chooser::clear_history()
{
    currentHist = history;
    while (currentHist)
    {
        History* next = currentHist->next;
        delete currentHist;
        currentHist = next;
    }
    currentHist = history = nullptr;
    backBtn->deactivate();
    forwardBtn->deactivate();
}

void Flu_File_Chooser::addToHistory()
{
    // remember history
    // only store this path in the history if it is not the current directory
    if (currentDir.size() && !walkingHistory)
    {
        if (history == nullptr)
        {
            history = new History;
            currentHist = history;
            currentHist->path = currentDir;
        }
        else if (currentHist->path != currentDir)
        {
            // since we are adding a new path, delete everything after this path
            History* h = currentHist->next;
            while (h)
            {
                History* next = h->next;
                delete h;
                h = next;
            }
            currentHist->next = new History;
            currentHist->next->last = currentHist;
            currentHist = currentHist->next;
            currentHist->path = currentDir;
        }
        History* h = history;
        while (h)
            h = h->next;
    }
    walkingHistory = false;

    if (currentHist)
    {
        if (currentHist->last)
            backBtn->activate();
        else
            backBtn->deactivate();
        if (currentHist->next)
            forwardBtn->activate();
        else
            forwardBtn->deactivate();
    }
}

// treating the string as a '|' or ';' delimited sequence of patterns, strip
// them out and place in patterns return whether it is likely that "s"
// represents a regexp file-matching pattern
bool Flu_File_Chooser::stripPatterns(std::string s, FluStringVector* patterns)
{
    if (s.size() == 0)
        return false;

    char* tok = strtok((char*)s.c_str(), "|;");
    int tokens = 0;
    while (tok)
    {
        tokens++;
        if (tok[0] == ' ')
            tok++; // skip whitespace
        patterns->push_back(tok);
        tok = strtok(nullptr, "|;");
    }

    // if there is just a single token and it looks like it's not a pattern,
    // then it is probably JUST a filename, in which case it should not be
    // treated as a pattern
    if (_isProbablyAPattern(s.c_str()))
        return true;
    else if (tokens == 1)
    {
        patterns->clear();
        return false;
    }
    else
        return true;
}

void Flu_File_Chooser::statFile(Flu_Entry* entry, const char* file)
{
#ifdef _WIN32
    wchar_t buf[1024];
    struct _stati64 s;
    s.st_size = 0;
    fl_utf8towc(file, (unsigned)strlen(file), buf, 1024);
    ::_wstati64(buf, &s);
#else
    struct stat s;
    ::stat(file, &s);
#endif

    bool isDir = (fl_filename_isdir(file) != 0);

    // store size as human readable and sortable integer
#ifdef _WIN32
#else
    passwd* pwd = getpwuid(s.st_uid); // this must not be freed
    if (pwd != nullptr)
        entry->owner = pwd->pw_name;
    else
        entry->owner = "unknown";
#endif

    entry->isize = s.st_size;
    entry->permissions = "";

    if (isDir && entry->isize == 0)
    {
        entry->filesize = "";
        entry->permissions = 'd';
    }
    else
    {
        char buf[32];
        if ((entry->isize >> 30) > 0) // gigabytes
        {
            double GB = double(entry->isize) / double(1 << 30);
            snprintf(buf, 32, "%.1f GB", GB);
        }
        else if ((entry->isize >> 20) > 0) // megabytes
        {
            double MB = double(entry->isize) / double(1 << 20);
            snprintf(buf, 32, "%.1f MB", MB);
        }
        else if ((entry->isize >> 10) > 0) // kilabytes
        {
            double KB = double(entry->isize) / double(1 << 10);
            snprintf(buf, 32, "%.1f KB", KB);
        }
        else // bytes
        {
            snprintf(buf, 32, "%d bytes", (int)entry->isize);
        }
        entry->filesize = buf;
    }

    // store date as human readable and sortable integer
    entry->date = formatDate(ctime(&s.st_mtime));
    entry->idate = s.st_mtime;

    // convert the permissions into UNIX style rwx-rwx-rwx (user-group-others)
    unsigned int p = s.st_mode;
#ifdef _WIN32
    entry->pU = int(bool(p & _S_IREAD) << 2) | int(bool(p & _S_IWRITE) << 1) |
                int(bool(p & _S_IEXEC));
    entry->pG = entry->pU;
    entry->pO = entry->pG;
#else
    entry->pU =
        bool(p & S_IRUSR) << 2 | bool(p & S_IWUSR) << 1 | bool(p & S_IXUSR);
    entry->pG =
        bool(p & S_IRGRP) << 2 | bool(p & S_IWGRP) << 1 | bool(p & S_IXGRP);
    entry->pO =
        bool(p & S_IROTH) << 2 | bool(p & S_IWOTH) << 1 | bool(p & S_IXOTH);
#endif
    const char* perms[8] = {"---", "--x", "-w-", "-wx",
                            "r--", "r-x", "rw-", "rwx"};
    entry->permissions += perms[entry->pU];
    entry->permissions += perms[entry->pG];
    entry->permissions += perms[entry->pO];
}

void Flu_File_Chooser::cd(const char* path)
{
    TLRENDER_P();
    Flu_Entry* entry;
    char cwd[1024];

    cancelThumbnailRequests();

    if (!path || path[0] == '\0')
    {
        path = getcwd(cwd, 1024);
        if (!path)
            path = "./";
    }

    if (path[0] == '~')
    {
        if (path[1] == '/' || path[1] == '\\')
            snprintf(cwd, 1024, "%s%s", userHome.c_str(), path + 2);
        else
            snprintf(cwd, 1024, "%s%s", userHome.c_str(), path + 1);
        path = cwd;
    }

    lastSelected = 0;

    filelist->scroll_to_beginning();
    filescroll->scroll_to(0, 0);

    bool listMode =
        !fileDetailsBtn->value() || streq(path, FAVORITES_UNIQUE_STRING);

#ifdef _WIN32
    // refresh the drives if viewing "My Computer"
    if (strcmp(path, "/") == 0)
        refreshDrives = true;
#endif

    buildLocationCombo();

    filename.take_focus();

    trashBtn->deactivate();
    reloadBtn->activate();
    newDirBtn->activate();
    previewBtn->activate();
    hiddenFiles->activate();
    addFavoriteBtn->activate();

    resize(x(), y(), w(), h());
    if (listMode)
    {
        // filecolumns->hide();
        // filescroll->hide();

        fileDetailsGroup->hide();
        filelist->show();
        filelist->parent()->resizable(filelist);
    }
    else
    {
        filelist->hide();

        // filecolumns->show();
        // filescroll->show();
        // filescroll->parent()->resizable( filescroll );
        fileDetailsGroup->show();
        fileDetailsGroup->parent()->resizable(fileDetailsGroup);
    }
    updateEntrySizes();

    std::string currentFile = filename.value();
    filescroll->scroll_to(0, 0);
    Fl::focus(&filename);
    upDirBtn->activate();
    ok.activate();

    // check for favorites
    if (streq(path, FAVORITES_UNIQUE_STRING))
    {
        currentDir = FAVORITES_UNIQUE_STRING;
        addToHistory();

        newDirBtn->deactivate();
        previewBtn->deactivate();
        reloadBtn->deactivate();
        addFavoriteBtn->deactivate();
        hiddenFiles->deactivate();
        location->input.value(_(favoritesTxt.c_str()));
        updateLocationQJ();

        filelist->clear();
        filedetails->clear();
        if (listMode)
            filelist->begin();
        else
            filedetails->begin();

        for (int i = 1; i <= favoritesList->size(); i++)
        {
            entry = new Flu_Entry(
                favoritesList->text(i), ENTRY_FAVORITE,
                false /*fileDetailsBtn->value()*/, this);
            entry->updateSize();
            entry->updateIcon();
        }
        if (listMode)
            filelist->end();
        else
        {
            filedetails->end();
        }

        Fl_Group* g = getEntryGroup();

        unsigned num = g->children();

        for (unsigned i = 0; i < num; ++i)
        {
            Flu_Entry* c = (Flu_Entry*)g->child(i);
            c->set_colors();
        }

        redraw();
        ok.deactivate();
        return;
    }
    // check for the current directory
    else if (streq(path, ".") || streq(path, "./") || streq(path, ".\\"))
    {
        // do nothing. just rescan this directory
    }
    // check for parent directory
    else if (streq(path, "..") || streq(path, "../") || streq(path, "..\\"))
    {
        // if we are viewing the favorites and want to go back a directory, go
        // to the previous directory
        if (currentDir == FAVORITES_UNIQUE_STRING)
        {
            backCB();
            return;
        }
#ifdef _WIN32
        // if we are at the desktop already, then we cannot go back any further
        // if( currentDir == "/Desktop/" )
        //{
        // do nothing
        //}
        // else if( currentDir == userHome+"Desktop/" )
        // currentDir = userHome;
        // if we are viewing "My Computer" and want to go back a directory, go
        // to the desktop
        if (currentDir == "/")
        {
            // currentDir = userDesktop;//userHome + "Desktop";
            //  do nothing
        }
        // if we are at a top level drive, go to "My Computer" (i.e. "/")
        else if (currentDir[1] == ':' && currentDir[3] == '\0')
            currentDir = "/";
        else
#else
        // if the current directory is already as far back as we can go, ignore
        if (currentDir != "/")
#endif
        {
            // strip everything off the end to the next "/"
            size_t end = currentDir.size() - 1;
            currentDir = currentDir.substr(0, end);
            while (currentDir[end] != '/')
            {
                currentDir = currentDir.substr(0, end);
                end--;
            }
        }
    }
    // check for absolute path
#ifdef _WIN32
    else if (path[1] == ':' || path[0] == '/')
#else
    else if (path[0] == '/')
#endif
    {
        currentDir = path;
    }
    // else relative path
    else
    {
        // concatenate currentDir with path to make an absolute path
        currentDir += path;
    }

    int numDirs = 0, numFiles = 0;
    filelist->clear();
    filedetails->clear();

    cleanupPath(currentDir);

#ifdef _WIN32
    std::string topdesk = "/";
    topdesk += _(desktopTxt.c_str());
    topdesk += "/";
    bool isTopDesktop = (currentDir == topdesk);
    bool isDesktop = correctPath(currentDir);
    if (isTopDesktop)
        upDirBtn->deactivate();
#else
    if (currentDir == "/")
        upDirBtn->deactivate();
#endif

#ifdef _WIN32
    bool root = false;
    // check for my computer
    if (currentDir == "/")
    {
        ok.deactivate();
        root = true;
        if (listMode)
            filelist->begin();
        else
            filedetails->begin();
        for (int i = 0; i < 26; i++)
        {
            if (drives[i][0] != '\0')
            {
                char drive[] = "A:/";
                drive[0] = 'A' + i;
                entry = new Flu_Entry(
                    drive, ENTRY_DRIVE, fileDetailsBtn->value(), this);
                switch (driveTypes[i])
                {
                case DRIVE_REMOVABLE:
                    entry->description = diskTypesTxt[0].c_str();
                    break;
                case DRIVE_FIXED:
                    entry->description = diskTypesTxt[2].c_str();
                    break;
                case DRIVE_CDROM:
                    entry->description = diskTypesTxt[3].c_str();
                    break;
                case DRIVE_REMOTE:
                    entry->description = diskTypesTxt[4].c_str();
                    break;
                case DRIVE_RAMDISK:
                    entry->description = diskTypesTxt[5].c_str();
                    break;
                }
                entry->icon = driveIcons[i];
                entry->altname = drives[i];
                entry->updateSize();
                entry->updateIcon();
            }
        }
        if (listMode)
            filelist->end();
        else
        {
            filedetails->end();
        }

        Fl_Group* g = getEntryGroup();
        unsigned num = g->children();
        for (unsigned i = 0; i < num; ++i)
        {
            Flu_Entry* c = (Flu_Entry*)g->child(i);
            c->set_colors();
        }

        redraw();
    }
    // check for desktop. if so, add My Computer and My Documents
    else if (isDesktop)
    {
        if (listMode)
            filelist->begin();
        else
            filedetails->begin();
        entry = new Flu_Entry(
            myDocumentsTxt.c_str(), ENTRY_MYDOCUMENTS, fileDetailsBtn->value(),
            this);
        entry->updateSize();
        entry->updateIcon();
        entry = new Flu_Entry(
            myComputerTxt.c_str(), ENTRY_MYCOMPUTER, fileDetailsBtn->value(),
            this);
        entry->updateSize();
        entry->updateIcon();
        if (listMode)
            filelist->end();
        else
        {
            filedetails->end();
        }
        numDirs += 2;

        Fl_Group* g = getEntryGroup();
        unsigned num = g->children();
        for (unsigned i = 0; i < num; ++i)
        {
            Flu_Entry* c = (Flu_Entry*)g->child(i);
            c->set_colors();
        }
    }
#endif

    // see if currentDir is in fact a directory
    // if so, make sure there is a trailing "/" and we're done
    if (fl_filename_isdir(currentDir.c_str()) || currentDir == "/")
    {
        if (currentDir[strlen(currentDir.c_str()) - 1] != '/')
            currentDir += "/";
#ifdef _WIN32
        if (filename.value()[1] != ':')
#else
        if (filename.value()[0] != '/')
#endif
        {
            if (!(selectionType & SAVING))
                filename.value("");
        }
        if (!(selectionType & SAVING))
            currentFile = "";
    }

    // now we have the current directory and possibly a file at the end
    // try to split into path and file
    if (currentDir[currentDir.size() - 1] != '/')
    {
        char* lastSlash = const_cast<char*>(strrchr(currentDir.c_str(), '/'));
        if (lastSlash)
        {
            currentFile = lastSlash + 1;
            currentDir = currentDir.substr(0, lastSlash - currentDir.c_str());
        }
    }
    // make sure currentDir ends in '/'
    if (currentDir[currentDir.size() - 1] != '/')
        currentDir += "/";

#ifdef _WIN32
    {
        std::string tmp = currentDir;
        if (isTopDesktop)
        {
            currentDir = "/";
            currentDir += _(desktopTxt.c_str());
            currentDir += "/";
        }
        addToHistory();
        if (isTopDesktop)
            currentDir = tmp;
    }
#else

    addToHistory();

#endif

    delayedCd = "./*";

#ifdef _WIN32
    // set the location input value
    // check for drives
    if (currentDir[1] == ':' && currentDir[3] == '\0')
    {
        location->input.value(currentDir.c_str());
    }
    else if (currentDir == "/")
        location->input.value(myComputerTxt.c_str());
    else
#endif
    {

        location->input.value(currentDir.c_str());
#ifdef _WIN32
        std::string treePath =
            "/" + desktopTxt + "/" + myComputerTxt + "/" + currentDir;
        Fl_Tree_Item* n =
            location->tree.add(mrv::string::commentCharacter(treePath).c_str());
        std::string userDesk = userHome + desktopTxt + "/";
        if (currentDir == userDesk)
            if (n)
                n->usericon(&little_desktop);
        std::string userDocs = userHome + myDocumentsTxt + "/";
        if (currentDir == userDocs)
            if (n)
                n->usericon(&documents);
#else

        Fl_Tree_Item* i;
        for (i = location->tree.first(); i; i = location->tree.next(i))
        {
            if (!i->is_root())
                break;
        }
        if (i && i->label() != currentDir)
            location->tree.insert_above(i, currentDir.c_str());
#endif
    }

    updateLocationQJ();

#ifdef _WIN32

    if (root)
        return;
#endif

    std::string pathbase, fullpath;
    bool isDir, isCurrentFile = false;
    const char *lastAddedFile = nullptr, *lastAddedDir = nullptr;

    pathbase = currentDir;

    // take the current pattern and make a list of filter pattern strings
    FluStringVector currentPatterns;
    {
        std::string pat = patterns[filePattern->list.value() - 1];
        while (pat.size())
        {
            size_t p = pat.find(',');
            if (p == std::string::npos)
            {
                if (pat != "*")
                    pat = "*." + pat;
                currentPatterns.push_back(pat);
                break;
            }
            else
            {
                std::string s = pat.c_str() + p + 1;
                pat = pat.substr(0, p);
                if (pat != "*")
                    pat = "*." + pat;
                currentPatterns.push_back(pat);
                pat = s;
            }
        }
    }

    // add any user-defined patterns
    FluStringVector userPatterns;
    // if the user just hit <Tab> but the filename input area is empty,
    // then use the current patterns
    if (!filenameTabCallback || currentFile != "*")
        stripPatterns(currentFile, &userPatterns);

    typedef std::vector< std::string > Directories;
    Directories dirs;
    typedef std::vector< std::string > Files;
    Files files;

    mrv::SequenceList tmpseqs;

    // read the directory as UTF-8
    dirent** e;
    char* name;
    int num = fl_filename_list(pathbase.c_str(), &e);
    if (num > 0)
    {
        int i;

        for (i = 0; i < num; i++)
        {
            name = e[i]->d_name;

            // ignore the "." and ".." names
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
                strcmp(name, "./") == 0 || strcmp(name, "../") == 0 ||
                strcmp(name, ".\\") == 0 || strcmp(name, "..\\") == 0)
                continue;

            // if 'name' ends in '/', remove it
            if (name[strlen(name) - 1] == '/')
                name[strlen(name) - 1] = '\0';

            // file or directory?
            fullpath = pathbase + name;
            isDir = (fl_filename_isdir(fullpath.c_str()) != 0);

            // was this file specified explicitly?
            isCurrentFile = (currentFile == name);

#ifndef _WIN32
            // filter hidden files
            if (!isCurrentFile && !hiddenFiles->value() && (name[0] == '.'))
                continue;
#endif

            // only directories?
            if ((selectionType & DIRECTORY) && !isDir &&
                !(selectionType & STDFILE) &&
                !(selectionType & DEACTIVATE_FILES))
            {
                continue;
            }

            // if( !isDir /*!isCurrentFile*/ )
            {
                // filter according to the user pattern in the filename input
                if (userPatterns.size())
                {
                    bool cull = true;
                    for (unsigned int i = 0; i < userPatterns.size(); i++)
                    {
                        if (fl_filename_match(name, userPatterns[i].c_str()) !=
                            0)
                        {
                            cull = false;
                            break;
                        }
                    }
                    if (cull)
                    {
                        // only filter directories if someone just hit <TAB>
                        if (!isDir || (isDir && filenameTabCallback))
                            continue;
                    }
                }
                // filter files according to the current pattern
                else
                {
                    bool cull = true;
                    for (unsigned int i = 0; i < currentPatterns.size(); i++)
                    {
                        if (fl_filename_match(
                                name, currentPatterns[i].c_str()) != 0)
                        {
                            cull = false;
                            break;
                        }
                    }
                    if (cull)
                    {
                        // only filter directories if someone just hit <TAB>
                        if (!isDir || (isDir && filenameTabCallback))
                            continue;
                    }
                }
            }

            // add directories at the beginning, then sequences and files at
            // the end
            if (isDir)
            {
                dirs.push_back(name);
            }
            else
            {

                file::Path path(name);
                const std::string root = path.getBaseName();
                const std::string frame = path.getNumber();
                const std::string view = ""; // @todo: path.getView();
                const std::string ext = path.getExtension();

                bool is_sequence = mrv::file::isSequence(name);
                if (compact_files())
                {
                    if (mrv::file::isMovie(ext) || mrv::file::isAudio(ext) ||
                        mrv::file::isSubtitle(ext) || ext == ".ocio" ||
                        ext == ".prefs" || ext == ".py" || ext == ".pyc")
                        is_sequence = false;
                }
                else
                {
                    is_sequence = false;
                }

                if (is_sequence)
                {
                    mrv::Sequence seq;
                    seq.ext = ext;
                    seq.view = view;
                    seq.number = frame;
                    seq.root = root;
                    tmpseqs.push_back(seq);
                }
                else
                {
                    if (root == "")
                    {
                        files.push_back(name);
                        continue;
                    }

                    std::string tmp = root + frame + ext;
                    files.push_back(tmp);
                }
            }
        }

        //
        // Add all directories first
        //
        {
            Directories::const_iterator i = dirs.begin();
            Directories::const_iterator e = dirs.end();
            for (; i != e; ++i)
            {
                entry = new Flu_Entry(
                    (*i).c_str(), ENTRY_DIR, fileDetailsBtn->value(), this);
                if (!entry)
                    continue;

                if (listMode)
                    filelist->insert(*entry, 0);
                else
                    filedetails->insert(*entry, 0);
                ++numDirs;
                lastAddedDir = entry->filename.c_str();

                fullpath = pathbase + *i;
                statFile(entry, fullpath.c_str());
            }
        }

        //
        // Then, sort sequences and collapse them into a single file entry
        //
        {
            std::sort(tmpseqs.begin(), tmpseqs.end(), mrv::SequenceSort());

            std::string root;
            std::string first;
            std::string number;
            std::string view;
            std::string ext;
            int zeros = -1;

            std::string seqname;
            mrv::SequenceList seqs;

            {
                mrv::SequenceList::iterator i = tmpseqs.begin();
                mrv::SequenceList::iterator e = tmpseqs.end();
                for (; i != e; ++i)
                {
                    const char* s = (*i).number.c_str();
                    int z = 0;
                    for (; *s == '0'; ++s)
                        ++z;

                    if ((*i).root != root || (*i).view != view ||
                        (*i).ext != ext || (zeros != z && z != zeros - 1))
                    {
                        // New sequence
                        if (seqname != "")
                        {
                            mrv::Sequence seq;
                            seq.root = seqname;
                            seq.number = seq.ext = first;
                            if (first != number)
                            {
                                seq.ext = number;
                            }
                            seq.view = (*i).view;
                            seqs.push_back(seq);
                        }

                        root = (*i).root;
                        zeros = z;
                        number = first = (*i).number;
                        view = (*i).view;
                        ext = (*i).ext;

                        seqname = root;
                        seqname += view;
                        if (z == 0)
                            seqname += "%d";
                        else
                        {
                            seqname += "%0";
                            char buf[19];
                            buf[18] = 0;
#ifdef _WIN32
                            seqname += itoa(int((*i).number.size()), buf, 10);
#else
                            snprintf(buf, 18, "%ld", (*i).number.size());
                            seqname += buf;
#endif
                            seqname += "d";
                        }
                        seqname += ext;
                    }
                    else
                    {
                        zeros = z;
                        number = (*i).number;
                    }
                }
            }

            if (!root.empty() || !first.empty())
            {
                mrv::Sequence seq;
                seq.root = seqname;
                seq.number = seq.ext = first;
                seq.view = view;
                if (first != number)
                {
                    seq.ext = number;
                }
                seqs.push_back(seq);
            }

            mrv::SequenceList::const_iterator i = seqs.begin();
            mrv::SequenceList::const_iterator e = seqs.end();
            for (const auto& i : seqs)
            {
                int numFrames =
                    1 + (atoi(i.ext.c_str()) - atoi(i.number.c_str()));
                if (numFrames == 1)
                {
                    int number = atoi(i.number.c_str());
                    char buf[1024];
                    snprintf(buf, 1024, i.root.c_str(), number);
                    entry = new Flu_Entry(
                        buf, ENTRY_FILE, fileDetailsBtn->value(), this,
                        p.context);
                }
                else
                {

                    entry = new Flu_Entry(
                        i.root.c_str(), ENTRY_SEQUENCE, fileDetailsBtn->value(),
                        this, p.context);
                    entry->isize = numFrames;
                    entry->altname = i.root.c_str();

                    entry->filesize = i.number;
                    if (entry->isize > 1)
                    {
                        entry->filesize += "-";
                        entry->filesize += i.ext;
                    }
                }

                entry->updateSize();
                entry->updateIcon();

                ++numFiles;
                if (listMode)
                    filelist->add(entry);
                else
                    filedetails->add(entry);
            }
        }

        {
            Files::const_iterator i = files.begin();
            Files::const_iterator e = files.end();
            for (; i != e; ++i)
            {
                entry = new Flu_Entry(
                    (*i).c_str(), ENTRY_FILE, fileDetailsBtn->value(), this,
                    p.context);

                if (listMode)
                {
                    filelist->add(entry);
                }
                else
                {
                    filedetails->add(entry);
                }
                numFiles++;
                lastAddedFile = entry->filename.c_str();

                // get some information about the file
                fullpath = pathbase + *i;
                statFile(entry, fullpath.c_str());

                entry->updateSize();
                entry->updateIcon();

                // was this file specified explicitly?
                isCurrentFile = (currentFile == entry->filename);
                if (isCurrentFile)
                {
                    filename.value(currentFile.c_str());
                    entry->selected = true;
                    lastSelected = entry;

                    filelist->scroll_to(entry);

                    filedetails->scroll_to(entry);

                    // break;
                }
            } // i != e
        }

    } // num > 0

    fl_filename_free_list(&e, num);

    // sort the files: directories first, then files

    if (listMode)
        filelist->sort(numDirs);
    else
        filedetails->sort(numDirs);

    Fl_Group* g = getEntryGroup();
    num = g->children();
    for (int i = 0; i < num; ++i)
    {
        Flu_Entry* c = (Flu_Entry*)g->child(i);
        c->set_colors();
    }

    // see if the user pushed <Tab> in the filename input field
    if (filenameTabCallback)
    {
        filenameTabCallback = false;

        std::string prefix = commonStr();

        if (numDirs == 1 && currentFile == (std::string(lastAddedDir) + "*"))
        {
            delayedCd = lastAddedDir;
            Fl::add_timeout(0.0f, Flu_File_Chooser::delayedCdCB, this);
        }

        if (numDirs == 1 && numFiles == 0)
        {
#ifdef _WIN32
            if (filename.value()[1] == ':')
#else
            if (filename.value()[0] == '/')
#endif
            {
                std::string s = currentDir + lastAddedDir + "/";
                filename.value(s.c_str());
            }
            else
                filename.value(lastAddedDir);
        }
        else if (numFiles == 1 && numDirs == 0)
        {
#ifdef _WIN32
            if (filename.value()[1] == ':')
#else
            if (filename.value()[0] == '/')
#endif
            {
                std::string s = currentDir + lastAddedFile;
                filename.value(s.c_str());
            }
            else
                filename.value(lastAddedFile);
        }
        else if (prefix.size() >= currentFile.size())
        {
#ifdef _WIN32
            if (filename.value()[1] == ':')
#else
            if (filename.value()[0] == '/')
#endif
            {
                std::string s = currentDir + prefix;
                filename.value(s.c_str());
            }
            else
            {
                filename.value(prefix.c_str());
            }
        }

#ifdef _WIN32
        if (currentFile == "*" && filename.value()[1] != ':')
            filename.value("");
#else
        if (currentFile == "*" && filename.value()[0] != '/')
            filename.value("");
#endif
    }

    // see if the user pushed <Enter> in the filename input field
    if (filenameEnterCallback)
    {
        filenameEnterCallback = false;

#ifdef _WIN32
        if (filename.value()[1] == ':')
            filename.value("");
#else
        if (filename.value()[0] == '/')
            filename.value("");
#endif

        // if( isCurrentFile && numFiles == 1 )
        if (!_isProbablyAPattern(filename.value()))
            okCB();
    }

    if (_isProbablyAPattern(filename.value()))
        filename.insert_position(filename.size(), filename.size());
    else
        filename.insert_position(filename.size(), filename.size());

    if (numFiles == 1 || numDirs == 1)
        filename.take_focus();

    // Handle loading of icons
    previewCB();

    redraw();
}

// find the prefix string that is common to all entries in the list
std::string Flu_File_Chooser::commonStr()
{
    std::string common;
    size_t index = 0;
    const char* name;
    size_t len;
    int i;
    Fl_Group* g = getEntryGroup();
    for (;;)
    {
        bool allSkipped = true;
        for (i = 0; i < g->children(); i++)
        {
            name = ((Flu_Entry*)g->child(i))->filename.c_str();
            len = strlen(name);
            if (index >= len)
                continue;
            allSkipped = false;
            if (i == 0)
                common.push_back(name[index]);
            else if (toupper(common[index]) != toupper(name[index]))
            {
                common[index] = '\0';
                return common;
            }
        }
        if (allSkipped)
            break;
        index++;
    }
    return common;
}

std::string retname;

static const char* _flu_file_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, int type,
    FluStringVector& filelist, const bool compact_files = true)
{
    if (!retname.empty())
        filename = retname.c_str();

    Fl_Group::current(0);
    Flu_File_Chooser::window = new Flu_File_Chooser(
        filename, pattern, type, message, context, compact_files);
    Flu_File_Chooser::window->end();
    if (Flu_File_Chooser::window && !retname.empty())
    {
        Flu_File_Chooser::window->value(retname.c_str());
    }
    
    Flu_File_Chooser::window->set_non_modal();
    Flu_File_Chooser::window->show();

    while (Flu_File_Chooser::window->shown())
        Fl::check();

    Fl_Group::current(0);

    if (Flu_File_Chooser::window->value())
    {
        if (Flu_File_Chooser::window->count() == 1)
        {
            filelist.push_back(Flu_File_Chooser::window->value());
        }
        else
        {
            for (int i = 1; i <= Flu_File_Chooser::window->count(); i++)
            {
                filelist.push_back(
                    std::string(Flu_File_Chooser::window->value(i)));
            }
        }
        retname = Flu_File_Chooser::window->value();

        delete Flu_File_Chooser::window;
        Flu_File_Chooser::window = nullptr;
        return retname.c_str();
    }
    else
    {
        delete Flu_File_Chooser::window;
        Flu_File_Chooser::window = nullptr;
        return 0;
    }
}

size_t flu_multi_file_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, FluStringVector& filelist,
    const bool compact_files)
{
    _flu_file_chooser(
        context, message, pattern, filename, Flu_File_Chooser::MULTI, filelist,
        compact_files);
    return filelist.size();
}

const char* flu_file_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, const bool compact_files)
{
    FluStringVector filelist;
    return _flu_file_chooser(
        context, message, pattern, filename, Flu_File_Chooser::SINGLE, filelist,
        compact_files);
}

const char* flu_save_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, const bool compact_files)
{
    FluStringVector filelist;
    return _flu_file_chooser(
        context, message, pattern, filename,
        Flu_File_Chooser::SINGLE | Flu_File_Chooser::SAVING, filelist,
        compact_files);
}

const char* flu_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename)
{
    FluStringVector filelist;
    return _flu_file_chooser(
        context, message, "*", filename, Flu_File_Chooser::DIRECTORY, filelist);
}

const char* flu_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename, bool showFiles)
{
    FluStringVector filelist;
    if (showFiles)
        return _flu_file_chooser(
            context, message, "*", filename,
            Flu_File_Chooser::DIRECTORY | Flu_File_Chooser::DEACTIVATE_FILES,
            filelist);
    else
        return (flu_dir_chooser(context, message, filename));
}

const char* flu_file_and_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename)
{
    FluStringVector filelist;
    return _flu_file_chooser(
        context, message, "*", filename, Flu_File_Chooser::STDFILE, filelist);
}
