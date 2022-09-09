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
    inline const char* float_printf( char* buf, float x ) noexcept
    {
        if ( isnan(x) )
        {
            return _("   NAN  ");
        }
        else if ( !isfinite(x) )
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
        if ( isnan(x) )
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
        if ( isnan(x) )
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

}
