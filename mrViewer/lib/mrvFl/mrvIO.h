#pragma once

#include <ostream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHome.h"
#include "mrvFl/mrvPreferences.h"

namespace mrv {


namespace trace {


typedef
std::basic_stringbuf<char, std::char_traits<char>, std::allocator<char> >
string_stream;

struct logbuffer : public string_stream
{
    static bool _debug;
    static std::fstream out;

    logbuffer() : string_stream() {
        str().reserve(1024);
    };
    virtual ~logbuffer();

    static void debug( bool t ) {
        _debug = t;
        open_stream();
    }
    static void open_stream();

    //! from basic_streambuf, stl function used to sync stream
    virtual int sync();

    virtual void print( const char* c ) = 0;

};

struct errorbuffer : public logbuffer
{
    virtual void print( const char* c );
};

struct warnbuffer : public logbuffer
{
    virtual void print( const char* c );
};

struct infobuffer : public logbuffer
{
    virtual void print( const char* c );
};


struct  errorstream : public std::ostream
{
    errorstream() : std::ostream( new errorbuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~errorstream() {
        delete rdbuf();
    };
};

struct  warnstream : public std::ostream
{
    warnstream() : std::ostream( new warnbuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~warnstream() {
        delete rdbuf();
    };
};

struct  infostream : public std::ostream
{
    infostream() : std::ostream( new infobuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~infostream() {
        delete rdbuf();
    };
};


extern infostream  info;
extern warnstream  warn;
extern errorstream error;

}


} // namespace mrv



#define mrvALERT(x) do { \
    std::ostringstream mErr; \
    mErr << x << std::endl; \
    std::cerr << x << std::endl; \
    mrv::alert( mErr.str().c_str() ); \
  } while (0);


#if 0

#define mrvLOG_ERROR(mod, msg)   do {                                   \
        std::cerr << _("ERROR: ") << N_("[") << mod << N_("] ") << msg; \
    } while(0)
#define mrvLOG_WARNING(mod, msg) do {                                   \
        std::cout << _("WARN : ") << N_("[") << mod << N_("] ") << msg; \
    } while(0)
#define mrvLOG_INFO(mod, msg)    do {                                   \
        std::cout << _("       ") << N_("[") << mod << N_("] ") << msg; \
    } while(0)
#define mrvCONN_INFO(mod, msg)    do {                                  \
        std::cout << _("{conn} ") << N_("[") << mod << N_("] ") << msg; \
    } while(0)

#else

#define mrvLOG_ERROR(mod, msg)   do {                                   \
   mrv::trace::error << _("ERROR: ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvLOG_WARNING(mod, msg) do {                                   \
    mrv::trace::warn << _("WARN : ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvLOG_INFO(mod, msg)    do {                                   \
    mrv::trace::info << _("       ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvCONN_INFO(mod, msg)    do {                                  \
    mrv::trace::conn << _("{conn} ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#endif

#define LOG_ERROR(msg)   mrvLOG_ERROR( kModule, msg << std::endl )
#define LOG_WARNING(msg) mrvLOG_WARNING( kModule, msg << std::endl )
#define LOG_INFO(msg)    mrvLOG_INFO( kModule, msg << std::endl )
#define LOG_DEBUG(msg)   mrvLOG_INFO( kModule,                          \
                                      __FUNCTION__ << "(" << __LINE__ << ") " \
                                      << msg << std::endl )
#define LOG_CONN(msg)    mrvCONN_INFO( kModule, msg << std::endl )
#define IMG_ERROR(msg)   do { if( !is_thumbnail() ) LOG_ERROR( this->name() << _(" frame ") << this->frame() << " - " << msg ); } while(0)
#define IMG_WARNING(msg) do { if( !is_thumbnail() ) LOG_WARNING( this->name() << _(" frame ") << this->frame() << " - " << msg ); } while(0)
#define IMG_INFO_F(msg) LOG_INFO( name() << _(" frame ") << this->frame() << " - " << msg )
#define IMG_INFO(msg) LOG_INFO( name() << " - " << msg )

#if 1
#include "mrvFl/mrvPreferences.h"
#define DBGM3(msg) do { \
    if ( mrv::Preferences::debug > 3 ) LOG_DEBUG( msg ); \
} while(0)

#define DBGM2(msg) do { \
    if ( mrv::Preferences::debug > 2 ) LOG_DEBUG( msg ); \
} while(0)

#define DBGM1(msg) do { \
    if ( mrv::Preferences::debug > 1 ) LOG_DEBUG( msg ); \
} while(0)

#define DBGM0(msg) do { \
    LOG_DEBUG( msg ); \
} while(0)

#define DBG3 do { \
    if ( mrv::Preferences::debug > 3 ) LOG_DEBUG( " " ); \
} while(0)

#define DBG2 do { \
    if ( mrv::Preferences::debug > 2 ) LOG_DEBUG( " " ); \
} while(0)

#define DBG do { \
    if ( mrv::Preferences::debug > 1 ) LOG_DEBUG( " " ); \
} while(0)

#else
#define DBGM3(msg)
#define DBGM2(msg)
#define DBGM1(msg)
#define DBG3
#define DBG2
#define DBG
#endif


#if 0
#  define TRACE(msg) do {                                          \
        std::cerr << "mrViewer TRACE : " << __PRETTY_FUNCTION__    \
                  << " (" << __LINE__ << ") " << msg               \
                  << std::flush << std::endl;                      \
    } while(0)
#else
#  define TRACE(msg)
#endif

#ifdef DEBUG
#  define TRACE2(msg) do {                                         \
        std::cerr << "mrViewer TRACE : " << __PRETTY_FUNCTION__    \
                  << " (" << __LINE__ << ") " << msg               \
                  << std::flush << std::endl;                      \
    } while(0)
#else
#  define TRACE2(msg)
#endif
