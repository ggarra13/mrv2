#pragma once

#include <tlTimeline/IRender.h>

// FLTK includes

#ifdef USE_METAL
#  include <FL/Fl_Metal_Window.H>
#  define Fl_SuperClass Fl_Metal_Window
#else
#  include <FL/Fl_Gl_Window.H>
#  define Fl_SuperClass Fl_Gl_Window
#endif

class Fl_RGB_Image;

namespace mrv
{
    using namespace tl;

    //
    // This class implements a thumbnail factory using OpenGL
    //
    class ThumbnailCreator : public Fl_SuperClass
    {
        TLRENDER_NON_COPYABLE(ThumbnailCreator);
    public:
        using callback_t = void (*)
                           ( const int64_t,
                             const std::vector< std::pair<otime::RationalTime,
                             Fl_RGB_Image*> >&, void* data );
    public:
        ThumbnailCreator( const std::shared_ptr<system::Context>& context );

        ~ThumbnailCreator();

        //! Request a thumbnail. The request ID is returned.
        int64_t request(
            const std::string&,
            const otime::RationalTime&,
            const imaging::Size&,
            const callback_t callback,
            void* callbackData,
            const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
            const timeline::LUTOptions& = timeline::LUTOptions());

        //! Request a thumbnail. The request ID is returned.
        int64_t request(
            const std::string&,
            const std::vector< otime::RationalTime >&,
            const imaging::Size&,
            const callback_t func,
            void* callbackData,
            const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
            const timeline::LUTOptions& = timeline::LUTOptions());


        //! Initialize the main thread to look for thumbnails.
        //! This
        void initThread();

        //! Cancel thumbnail requests.
        void cancelRequests(int64_t);

        //! Set the request count.
        void setRequestCount(int);

        //! Set the request timeout (milliseconds).
        void setRequestTimeout(int);

        //! Set the timer interval (seconds).
        void setTimerInterval(double);

        static void timerEvent_cb( void* );

    protected:
        void timerEvent();

        //! Main thread function to create the thumbnails.  initThread calls it.
        void run();

        TLRENDER_PRIVATE();
    };
}
