// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#ifdef TLRENDER_OCIO
#    include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;
#endif

namespace mrv
{
    namespace ocio
    {
        extern std::string ocioDefault;
#ifdef TLRENDER_OCIO
        extern OCIO::ConstConfigRcPtr config;
#endif

        void setup();

        void defaultIcs();
        
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

        //! List of OCIO presets.
        std::vector<std::string> ocioPresetsList();

        //! Set OCIO settings to a valid preset.
        void setOcioPreset(const std::string& preset);

        //! Return a human readable summary of an OCIO preset.
        std::string ocioPresetSummary(const std::string& presetName);

        //! Create an OCIO preset from current defaults.
        void createOcioPreset(const std::string& preset);

        //! Remove an existing OCIO preset.
        void removeOcioPreset(const std::string& preset);

        //! Load OCIO presets from disk.
        bool loadOcioPresets(const std::string& fileName);

        //! Save OCIO presets to disk.
        bool saveOcioPresets(const std::string& fileName);

    } // namespace ocio
} // namespace mrv
