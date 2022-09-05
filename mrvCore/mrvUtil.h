#pragma once

#include <cmath>
#include "mrvCore/mrvI8N.h"

namespace mrv {

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
    inline std::string float_printf( float x )
    {
        if ( isnan(x) )
        {
            static std::string empty( _("   NAN  ") );
            return empty;
        }
        else if ( !isfinite(x) )
        {
            static std::string inf( _("  INF.  ") );
            return inf;
        }
        else
        {
            char buf[ 64 ];
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
    inline std::string hex_printf( float x )
    {
        if ( isnan(x) )
        {
            static std::string empty( "        " );
            return empty;
        }
        else
        {
            char buf[ 64 ];
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
    inline std::string dec_printf( float x )
    {
        if ( isnan(x) )
        {
            static std::string empty( "        " );
            return empty;
        }
        else
        {
            char buf[ 64 ];
            unsigned h = 0;
            if ( x > 0.0f ) h = unsigned(x*255.0f);
            sprintf( buf, " %7d", h );
            return buf + strlen(buf) - 8;
        }
    }

}
