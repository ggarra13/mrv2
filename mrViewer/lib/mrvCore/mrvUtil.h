#pragma once

#include <cmath>
#include <cstdio>
#include <cinttypes>

#include <tlCore/File.h>

#include <opentime/rationalTime.h>
namespace otime = opentime::OPENTIME_VERSION;

#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvI8N.h"


namespace mrv {

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
    inline const char* float_printf( char* buf, float x ) noexcept
    {
        if ( std::isnan(x) )
        {
            return _("   NAN  ");
        }
        else if ( !std::isfinite(x) )
        {
            return _("  INF.  ");
        }
        else
        {
            sprintf( buf, " %7.4f", x );
            return buf + strlen(buf) - 8;
        }
    }

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
    inline const char* hex_printf( char* buf, float x ) noexcept
    {
        if ( std::isnan(x) )
        {
            return  "        ";
        }
        else
        {
            unsigned h = 0;
            if ( x > 0.0f ) h = unsigned(x*255.0f);
            sprintf( buf, " %7x", h );
            return buf + strlen(buf) - 8;
        }
    }


/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
    inline const char* dec_printf( char* buf, float x ) noexcept
    {
        if ( std::isnan(x) )
        {
            return  "        ";
        }
        else
        {
            unsigned h = 0;
            if ( x > 0.0f ) h = unsigned(x*255.0f);
            sprintf( buf, " %7d", h );
            return buf + strlen(buf) - 8;
        }
    }

    //! Create a basename+number+extension from a path and a time
    inline std::string createStringFromPathAndTime(
        const tl::file::Path& path,
        const otime::RationalTime& time ) noexcept
    {
        const auto& name = path.getBaseName();
        int64_t    frame = time.to_frames();
        const auto& num = path.getNumber();
        const auto& extension = path.getExtension();
        if ( mrv::is_valid_movie( extension.c_str() ) )
            frame = atoi( num.c_str() );

        char buf[256]; buf[0] = 0;
        if ( !num.empty() )
        {
            const uint8_t padding = path.getPadding();
            sprintf( buf, "%0*" PRId64, padding, frame );
        }

        return name + buf + extension;
    }
}
