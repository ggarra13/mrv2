#pragma once

#ifdef _WIN32
#pragma warning( disable: 4275 )
#endif

#include <string>

#include "mrvFl/mrvColorSchemes.h"

// OpenColorIO
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;



class ViewerUI;
class PreferencesUI;

namespace mrv
{

    class Preferences
    {
    public:
        enum CacheType
        {
            kCacheAsLoaded,
            kCache8bits,
            kCacheHalf,
            kCacheFloat
        };

        enum MissingFrameType
        {
            kBlackFrame = 0,
            kRepeatFrame,
            kScratchedRepeatFrame
        };

        enum LutAlgorithm
        {
            kLutPreferCTL,
            kLutOnlyCTL,
            kLutPreferICC,
            kLutOnlyICC
        };

    public:
        Preferences( PreferencesUI* ui );
        ~Preferences();

        static void run( ViewerUI* main );
        static void save();

        static std::string temporaryDirectory() {
            return tempDir;
        }

        static OCIO::ConstConfigRcPtr OCIOConfig()
            {
                return config;
            }

    protected:
        static bool set_transforms();

    public:
        static ViewerUI* uiMain;
        static ColorSchemes    schemes;
        static bool use_ocio;
        static bool native_file_chooser;
        static int bgcolor;
        static int textcolor;
        static int selectioncolor;
        static int selectiontextcolor;
        static int R3dScale;
        static int BRAWScale;
        static int switching_images;

        static OCIO::ConstConfigRcPtr config;
        static std::string OCIO_Display;
        static std::string OCIO_View;


        static MissingFrameType missing_frame;
        static std::string video_threads;


        static std::string root;
        static std::string tempDir;
        static std::string hotkeys_file;
        static int language_index;
        static int debug;
    };


} // namespace mrv
