// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifndef mrvLanguages_h
#define mrvLanguages_h

#include <vector>
#include <string>

namespace mrv
{
    class App;
    class PopupMenu;
    std::string setLanguageLocale();

    std::string codeToLanguage(const std::string& code);

    std::vector<std::string> getLanguageCodes();
    std::vector<std::string> getLanguages();
    

} // namespace mrv



class PreferencesUI;
void check_language(PreferencesUI* uiPrefs, int& language_index, mrv::App*);

char* select_character(const char* p, bool colon = false);
void select_character(mrv::PopupMenu* w, bool colon = false);

#endif // mrvLanguages_h
