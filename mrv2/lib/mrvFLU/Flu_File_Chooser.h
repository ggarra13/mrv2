// $Id: Flu_File_Chooser.h,v 1.63 2004/10/18 15:14:58 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 *
 * Licnese is like FLTK.
 ***************************************************************/

#include <string>
#include <vector>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Check_Button.H>

#include "mrvFLU/Flu_Button.h"
#include "mrvFLU/Flu_Return_Button.h"
#include "mrvFLU/Flu_Wrap_Group.h"
#include "mrvFLU/Flu_Combo_Tree.h"
#include "mrvFLU/Flu_Combo_List.h"
#include "mrvFLU/flu_export.h"

#include "mrvGL/mrvThumbnailCreator.h"

struct ThumbnailData;
using namespace tl;

typedef std::vector< std::string > FluStringVector;

FLU_EXPORT const char* flu_file_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, const bool compact_files = true);
FLU_EXPORT size_t flu_multi_file_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, FluStringVector& filelist,
    const bool compact_files = true);
FLU_EXPORT const char* flu_save_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* pattern, const char* filename, const bool compact_files = true);
FLU_EXPORT const char* flu_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename);
FLU_EXPORT const char* flu_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename, bool showFiles);
FLU_EXPORT const char* flu_file_and_dir_chooser(
    const std::shared_ptr<tl::system::Context>& context, const char* message,
    const char* filename);

//! A file and directory choosing widget that looks and acts similar to the
//! stock Windows file chooser
class FLU_EXPORT Flu_File_Chooser : public Fl_Double_Window
{

    friend class FileInput;
    class FileInput : public Fl_Input
    {
    public:
        FileInput(
            int x, int y, int w, int h, const char* l, Flu_File_Chooser* c);
        ~FileInput();

        int handle(int event);

    protected:
        Flu_File_Chooser* chooser;
    };

public:
    //! strings to be set by a programmer to the correct phrase or name for
    //! their language
    /*! (they are in english by default)   */
    static std::string favoritesTxt;
    static std::string desktopTxt;
    static std::string myComputerTxt;
    static std::string myDocumentsTxt;

    static std::string filenameTxt;
    static std::string okTxt;
    static std::string cancelTxt;
    static std::string locationTxt;
    static std::string showHiddenTxt;
    static std::string fileTypesTxt;
    static std::string directoryTxt;
    static std::string allFilesTxt;
    static std::string defaultFolderNameTxt;

    static std::string backTTxt;
    static std::string forwardTTxt;
    static std::string upTTxt;
    static std::string reloadTTxt;
    static std::string trashTTxt;
    static std::string newDirTTxt;
    static std::string addFavoriteTTxt;
    static std::string previewTTxt;
    static std::string listTTxt;
    static std::string wideListTTxt;
    static std::string detailTTxt;

    static std::string detailTxt[7];
    static std::string contextMenuTxt[3];
    static std::string diskTypesTxt[6];

    static std::string createFolderErrTxt;
    static std::string deleteFileErrTxt;
    static std::string fileExistsErrTxt;
    static std::string renameErrTxt;

    //! This class must be derived from to create a "preview" widget.
    /*! Simply derive from this class and overload Fl_Group's methods to create
      a widget able to preview whatever file type you want. Register it with
      Flu_File_Chooser::add_preview_handler() When a file is previewed, all
      registered handlers are visited until the preview() virtual function for
      one of them returns nonzero. When preview() is called, the absolute path
      of the file is passed in, and the widget should determine whether it can
      preview the file and update itself accordingly. If it can preview the
      file, it should return nonzero, else it should return zero.
     */
    //! File entry type
    enum
    {
        ENTRY_NONE     = 1,  /*!< An empty (or non-existant) entry */
        ENTRY_DIR      = 2,  /*!< A directory entry */
        ENTRY_FILE     = 4,  /*!< A file entry */
        ENTRY_FAVORITE = 8,  /*!< A favorite entry */
        ENTRY_DRIVE    = 16, /*!< An entry that refers to a disk drive */
        ENTRY_MYDOCUMENTS =
            32, /*!< The entry referring to the current user's documents */
        ENTRY_MYCOMPUTER =
            64, /*!< The entry referring to "My Computer" in Windows */
        ENTRY_SEQUENCE = 128 /*!< The entry referring to a sequence of frames */
    };

    //! Chooser type
    enum
    {
        SINGLE    = 0, /*!< Choose a single file or directory */
        MULTI     = 1, /*!< Choose multiple files or directories */
        DIRECTORY = 4, /*!< Choose directories (choosing files is implicit if
                          this bit is clear) */
        DEACTIVATE_FILES = 8, /*!< When choosing directories, also show the
                                 files in a deactivated state */
        SAVING = 16, /*!< When choosing files, whether to keep the current
                        filename always in the input area */
        STDFILE = 32 /*!< Choose both files and directories at the same time */
    };

    //! Structure holding the info needed for custom file types
    struct FileTypeInfo
    {
        Fl_Image* icon;
        std::string extensions;
        std::string type, shortType;
    };

    //! Constructor opening a file chooser with title \b title visiting
    //! directory \b path with files filtered according to \b pattern. \b type
    //! is a logical OR of Flu_File_Chooser::SINGLE, Flu_File_Chooser::MULTI,
    //! and Flu_File_Chooser::DIRECTORY
    Flu_File_Chooser(
        const char* path, const char* pattern, int type, const char* title,
        const bool compact = true);

    //! Destructor
    ~Flu_File_Chooser();

    //! Add a custom callback that is called when the user right-clicks on an
    //! entry
    /*! \param type is the type of entry to handle (i.e. a logical OR of \c
      ENTRY_NONE, \c ENTRY_DIR, \c ENTRY_FILE, \c ENTRY_FAVORITE, \c
      ENTRY_DRIVE, \c ENTRY_MYDOCUMENTS, \c ENTRY_MYCOMPUTER). To add a
      "nothing" handler (when the user right-clicks on nothing), use ENTRY_NONE
      \param ext is the extension of the file that will cause this handler to be
      added to the popup menu \param name is the name that will appear in the
      popup menu for this handler
     */
    static void add_context_handler(
        int type, const char* ext, const char* name,
        void (*cb)(const char*, int, void*), void* cbd);

    //! Add descriptive information and an icon for a file type
    /*! \param extensions is a space- or comma-delimited list of file
      extensions, or \c NULL for directories. e.g. "zip,tgz,rar" \param
      short_description is a short description (!) of the file type. e.g.
      "Compressed Archive" \param icon is an optional custom icon to use for
      this file type
     */
    static void add_type(
        const char* extensions, const char* short_description,
        Fl_Image* icon = NULL);

    //! deprecated - do not use - right click to change filenames
    inline void allow_file_editing(bool b) { fileEditing = b; }

    //! deprecated - do not use - right click to change filenames
    inline bool allow_file_editing() const { return fileEditing; }

    // Make sequences be displayed as a single line
    inline void compact_files(const bool compact) { _compact = compact; }

    // Return if sequences are displayed as a single line
    inline bool compact_files() const { return _compact; }

    //! Set whether file sorting is case insensitive. Default value is
    //! case-insensitive for windows, case-sensitive for everything else
    inline void case_insensitive_sort(bool b) { caseSort = !b; }

    //! Get whether file sorting is case insensitive
    inline bool case_insensitive_sort() const { return !caseSort; }

    //! Change the current directory the chooser is browsing to \b path
    void cd(const char* path);

    //! Clear the history of which directories have been visited
    void clear_history();

    inline static void _previewCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->previewCB();
    }

    //! previewCB handle icon creation
    void previewCB();

    //! set the tlRender context for icon creation
    void setContext(const std::shared_ptr< system::Context >&);

    //! \return how many files are selected
    int count();

    //! Set the default icon to use for all files for which no other icon has
    //! been specified
    inline void default_file_icon(Fl_Image* i) { defaultFileIcon = i; }

    //! Alias for cd()
    inline void directory(const char* d) { cd(d); }

    //! Alias for pattern()
    inline void filter(const char* p) { pattern(p); }

    //! Alias for pattern()
    inline const char* filter() const { return pattern(); }

    //! \return a pointer to a FileTypeInfo structure for files with type \b
    //! extension
    static FileTypeInfo* find_type(const char* extension);

    //! \return the current directory that the browser is visiting
    inline const char* get_current_directory() const
    {
        return currentDir.c_str();
    }

    //! Override of Fl_Double_Window::handle()
    int handle(int event);

    //! Change the file filter pattern to \b p
    void pattern(const char* p);

    //! Get the current file filter pattern
    inline const char* pattern() const { return rawPattern.c_str(); }

    //! Refresh the current directory
    inline void rescan() { reloadCB(); }

    //! Override of Fl_Double_Window::resize()
    void resize(int x, int y, int w, int h);

    //! Select all entries (only valid for multiple-selections)
    void select_all();

    //! Set a custom sorting function for sorting entries based on filename
    inline void set_sort_function(int (*cb)(const char*, const char*))
    {
        customSort = cb;
        rescan();
    }

    //! Set the type of the chooser (see constructor)
    inline void type(int t)
    {
        selectionType = t;
        rescan();
    }

    //! Get the type of the chooser
    inline int type(int t) const { return selectionType; }

    //! Unselect all entries
    void unselect_all();

    //! Set the current file the chooser is selecting
    void value(const char* v);

    //! Get the current file the chooser is selecting
    const char* value();

    //! For MULTI file queries, get selected file \b n (base 1 - i.e. 1 returns
    //! the first file, 2 the second, etc)
    const char* value(int n);

    bool _compact;

    FileInput filename;
    Flu_Return_Button ok;
    Flu_Button cancel;

    class ContextHandler
    {
    public:
        std::string ext, name;
        int type;
        void (*callback)(const char*, int, void*);
        void* callbackData;
        inline ContextHandler& operator=(const ContextHandler& c)
        {
            ext          = c.ext;
            name         = c.name;
            type         = c.type;
            callback     = c.callback;
            callbackData = c.callbackData;
            return *this;
        }
    };
    typedef std::vector< ContextHandler > ContextHandlerVector;
    static ContextHandlerVector contextHandlers;

    Fl_Check_Button* hiddenFiles;
    Flu_Combo_Tree* location;

    inline static void _backCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->backCB();
    }
    void backCB();

    inline static void _forwardCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->forwardCB();
    }
    void forwardCB();

    inline static void _sortCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->sortCB(w);
    }
    void sortCB(Fl_Widget* w);

    inline static void _listModeCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->listModeCB();
    }
    void listModeCB();

    inline static void _filenameCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->filenameCB();
    }
    void filenameCB();

    inline static void _locationCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->locationCB(((Flu_Combo_Tree*)w)->value());
    }
    void locationCB(const char* path);

    inline static void _locationQJCB(Fl_Widget* w, void* arg)
    {
        ((Flu_File_Chooser*)arg)->cd(((Fl_Button*)w)->label());
    }

    inline static void delayedCdCB(void* arg)
    {
        ((Flu_File_Chooser*)arg)
            ->cd(((Flu_File_Chooser*)arg)->delayedCd.c_str());
    }

    inline static void selectCB(void* arg) { ((Flu_File_Chooser*)arg)->okCB(); }

    inline static void _cancelCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->cancelCB();
    }
    void cancelCB();

    inline static void _okCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->okCB();
    }
    void okCB();

    inline static void _trashCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->trashCB();
    }
    void trashCB(bool recycle = true);

    inline static void _newFolderCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->newFolderCB();
    }
    void newFolderCB();

    inline static void upDirCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->cd("../");
    }

    inline static void reloadCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->reloadCB();
    }
    void reloadCB();

    inline static void _homeCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->homeCB();
    }
    void homeCB();

    inline static void _desktopCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->desktopCB();
    }
    void desktopCB();

    inline static void _favoritesCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->favoritesCB();
    }
    void favoritesCB();

    inline static void _myComputerCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->myComputerCB();
    }
    void myComputerCB();

    inline static void _addToFavoritesCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->addToFavoritesCB();
    }
    void addToFavoritesCB();

    inline static void _documentsCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->documentsCB();
    }
    void documentsCB();

    inline static void _hideCB(Fl_Widget*, void* arg)
    {
        ((Flu_File_Chooser*)arg)->hideCB();
    }
    void hideCB();
    void do_callback();

    enum
    {
        SORT_NAME    = 1,
        SORT_SIZE    = 2,
        SORT_TYPE    = 4,
        SORT_DATE    = 8,
        SORT_REVERSE = 16
    };
    static void
    _qSort(int how, bool caseSort, Fl_Widget** array, int low, int high);

    void cancelThumbnailRequests();

    friend class Entry;

    class Entry : public Fl_Input
    {
    public:
        Entry(const char* name, int t, bool d, Flu_File_Chooser* c);
        ~Entry();

        int handle(int event);
        void draw();

        void set_colors();

        void updateSize();
        void updateIcon();

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
        bool delete_icon;

        int nameW, typeW, sizeW, dateW;
        bool details;

        inline static void _inputCB(Fl_Widget* w, void* arg)
        {
            ((Entry*)arg)->inputCB();
        }
        void inputCB();

        inline static void _editCB(void* arg) { ((Entry*)arg)->editCB(); }
        void editCB();
    };

    class EntryArray : public std::vector< Entry* >
    {
    public:
        EntryArray(){};
        ~EntryArray(){};

        void push_back(Entry* e)
        {
            for (auto x : *this)
            {
                if (x == e)
                    return;
            }

            std::vector< Entry* >::push_back(e);
        }
    };

    friend class FileList;
    class FileList : public Flu_Wrap_Group
    {
    public:
        FileList(int x, int y, int w, int h, Flu_File_Chooser* c);
        ~FileList();

        int handle(int event);
        void sort(int numDirs = -1);

        inline Fl_Widget* child(int n) const
        {
            return Flu_Wrap_Group::child(n);
        }

        inline int children() const { return Flu_Wrap_Group::children(); }

        int numDirs;
        Flu_File_Chooser* chooser;
    };

    friend class FileDetails;
    class FileDetails : public Fl_Pack
    {
    public:
        FileDetails(int x, int y, int w, int h, Flu_File_Chooser* c);
        ~FileDetails();

        int handle(int event);
        void sort(int numDirs = -1);

        void scroll_to(Fl_Widget* w);
        Fl_Widget* next(Fl_Widget* w);
        Fl_Widget* previous(Fl_Widget* w);

        int numDirs;
        Flu_File_Chooser* chooser;
    };

    friend class CBTile;
    class CBTile : public Fl_Tile
    {
    public:
        CBTile(int x, int y, int w, int h, Flu_File_Chooser* c);
        int handle(int event);
        Flu_File_Chooser* chooser;
    };

    friend class FileColumns;
    class FileColumns : public Fl_Tile
    {
    public:
        FileColumns(int x, int y, int w, int h, Flu_File_Chooser* c);
        ~FileColumns();

        int handle(int event);
        void resize(int x, int y, int w, int h);
        Flu_File_Chooser* chooser;
        int W1, W2, W3, W4;
    };

    void createdThumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        ThumbnailData* data);

    //! Selection array in the order of elements as they were selected
    EntryArray selection;

    Fl_Group* getEntryGroup();
    Fl_Group* getEntryContainer();
    std::string toTLRenderFilename(const Entry* e);
    otime::RationalTime toTLRenderTime(const Entry* e);

    void win2unix(std::string& s);

    void cleanupPath(std::string& s);

    bool correctPath(std::string& path);

    void updateEntrySizes();

    void buildLocationCombo();

    void updateLocationQJ();

    void statFile(Entry* e, const char* file);

    void addToHistory();

    std::string formatDate(const char* d);

    void recursiveScan(const char* dir, FluStringVector* files);

    bool stripPatterns(std::string s, FluStringVector* patterns);

    int popupContextMenu(Entry* entry);

    std::string commonStr();

    static int (*customSort)(const char*, const char*);

    Fl_Group* wingrp;
    Fl_Group *fileGroup, *locationQuickJump;
    Fl_Menu_Button entryPopup;
    Fl_Image* defaultFileIcon;
    Entry* lastSelected;
    FileList* filelist;
    FileColumns* filecolumns;
    Fl_Group* fileDetailsGroup;
    Fl_Scroll* filescroll;
    FileDetails* filedetails;
    Flu_Button *detailNameBtn, *detailTypeBtn, *detailSizeBtn, *detailDateBtn;
    std::string currentDir, delayedCd, rawPattern;
    std::string configFilename;
    std::string userHome, userDesktop, userDocs;
    std::string drives[26];
    Fl_Pixmap* driveIcons[26];
    Flu_Button *fileListBtn, *fileListWideBtn, *fileDetailsBtn, *backBtn,
        *forwardBtn, *upDirBtn, *trashBtn, *newDirBtn, *addFavoriteBtn,
        *reloadBtn, *previewBtn;
    Fl_Browser* favoritesList;
    Flu_Combo_List* filePattern;
    int selectionType;
    bool filenameEnterCallback, filenameTabCallback, walkingHistory, caseSort,
        fileEditing;
    int sortMethod;

    FluStringVector patterns;

    static FileTypeInfo* types;
    static int numTypes;
    static int typeArraySize;

    static bool thumbnailsFileReq;
    static bool singleButtonTravelDrawer;

    static std::string dArrow[4];
    static std::string uArrow[4];

#ifdef _WIN32
    unsigned int driveMask;
    unsigned int driveTypes[26];
    std::string volumeNames[26];
    bool refreshDrives;
#endif

    class History
    {
    public:
        History() { last = next = NULL; }
        std::string path;
        History *last, *next;
    };

    History *history, *currentHist;

    Fl_Callback* _callback;
    void* _userdata;

    TLRENDER_PRIVATE();
};
