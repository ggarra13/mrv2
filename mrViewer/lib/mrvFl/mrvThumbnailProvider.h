#pragma once

#include <tlTimeline/IRender.h>

// FLTK includes
#include <FL/Fl_Gl_Window.H>

class Fl_RGB_Image;

namespace mrv
{
    using namespace tl;

    //
    // This class implements a thumbnail using OpenGL
    //
    class ThumbnailProvider : public Fl_Gl_Window
    {
        TLRENDER_NON_COPYABLE(ThumbnailProvider);
    public:
        // using callback_t = std::function< void ( const int64_t,
        //     const std::vector< std::pair<otime::RationalTime,
        //     Fl_RGB_Image*> >&, void* data ) >;

        using callback_t = void (*) ( const int64_t,
                                      const std::vector< std::pair<otime::RationalTime,
                                      Fl_RGB_Image*> >&, void* data );
    public:
        ThumbnailProvider( const std::shared_ptr<system::Context>& context );

        ~ThumbnailProvider();

        //! Request a thumbnail. The request ID is returned.
        int64_t request(
            const std::string&,
            const otime::RationalTime&,
            const imaging::Size&,
            const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
            const timeline::LUTOptions& = timeline::LUTOptions());

        //! Request a thumbnail. The request ID is returned.
        int64_t request(
            const std::string&,
            const std::vector< otime::RationalTime >&,
            const imaging::Size&,
            const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
            const timeline::LUTOptions& = timeline::LUTOptions());

        //! Create the thumbnails
        void run();

        //! Create the thumbnails
        void draw() override;

        //! Cancel thumbnail requests.
        void cancelRequests(int64_t);

        //! Set the request count.
        void setRequestCount(int);

        //! Set the request timeout (milliseconds).
        void setRequestTimeout(int);

        //! Set the timer interval (seconds).
        void setTimerInterval(double);

        void setThumbnailCallback( callback_t func, void* data );

        static void timerEvent_cb( void* );

    protected:
        //! This signal is emitted when thumbnails are ready.
        void initializeGL();

        void timerEvent();

        TLRENDER_PRIVATE();
    };
}
