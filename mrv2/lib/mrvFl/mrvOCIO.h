// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    namespace ocio
    {
        extern std::string ocioDefault;

        std::string ocioConfig();
        void setOcioConfig(const std::string config);

        int ocioIcsIndex(const std::string&);
        std::string ocioIcs();
        void setOcioIcs(const std::string&);

        int ocioLookIndex(const std::string&);
        std::string ocioLook();
        void setOcioLook(const std::string&);

        //! Given a display and view, return mrv2's merged display view format
        std::string ocioDisplayViewShortened(
            const std::string& display, const std::string& view);

        void ocioSplitViewIntoDisplayView(const std::string& combined,
                                          std::string& display,
                                          std::string& view);
        
        int ocioViewIndex(const std::string&);
        std::string ocioView();
        void setOcioView(const std::string&);

        std::vector<std::string> ocioIcsList();
        std::vector<std::string> ocioLookList();
        std::vector<std::string> ocioViewList();

        // std::string ocioPreset();
        std::vector<std::string> ocioPresetsList();
        
        void setOcioPreset(const std::string& preset);
        void createOcioPreset(const std::string& preset);
        void removeOcioPreset(const std::string& preset);

        bool loadOcioPresets(const std::string& fileName);
        bool saveOcioPresets(const std::string& fileName);

    } // namespace ocio
} // namespace mrv
