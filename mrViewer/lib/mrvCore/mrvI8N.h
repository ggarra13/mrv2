// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.



#pragma once

#ifdef USE_GETTEXT


#include <string.h>

#include <libintl.h>
#define _(String)  gettext2(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

inline char* gettext2 (const char* msgid)
{
    const char* const empty = "";
    if ( !msgid || strlen( msgid ) == 0 ) return (char*)empty;
    return gettext( msgid );
};

#ifdef _WIN32
#undef fprintf
#undef setlocale
#undef sprintf
#endif

#else

#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

#endif
