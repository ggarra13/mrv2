// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#    include <string.h>
#    include <libintl.h>

#    define _(String) gettext2(String)

inline char* gettext2(const char* msgid)
{
    const char* const empty = "";
    if (!msgid || strlen(msgid) == 0)
        return (char*)empty;
    return gettext(msgid);
};

#    ifdef _WIN32
#        undef fprintf
#        undef setlocale
#        undef sprintf
#        undef snprintf
#    endif

