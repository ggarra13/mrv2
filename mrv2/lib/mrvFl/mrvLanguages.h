// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifndef mrvLanguages_h
#define mrvLanguages_h

#include <string>

struct LanguageTable
{
    int index;
    const char* code;
};

extern LanguageTable kLanguages[18];

class PreferencesUI;
void check_language(PreferencesUI* uiPrefs, int& language_index);

namespace mrv
{
    class PopupMenu;
    std::string setLanguageLocale();
} // namespace mrv

char* select_character(const char* p, bool colon = false);
void select_character(mrv::PopupMenu* w, bool colon = false);

#endif // mrvLanguages_h
