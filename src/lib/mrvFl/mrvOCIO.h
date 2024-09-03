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
        extern OCIO::ConstConfigRcPtr OCIOconfig;
#endif

        void setup();

        void defaultIcs();
        
        std::string config();
        void setConfig(const std::string config);

        int icsIndex(const std::string&);
        std::string ics();
        void setIcs(const std::string&);

        int lookIndex(const std::string&);
        std::string look();
        void setLook(const std::string&);

        //! Given a display and view, return mrv2's merged display view format
        std::string displayViewShortened(
            const std::string& display, const std::string& view);

        void splitViewIntoDisplayView(
            const std::string& combined, std::string& display,
            std::string& view);

        // Set the display, with the default view if display exists.
        void setDisplay(const std::string&);

        int viewIndex(const std::string&);
        std::string view();
        
        // Set view (display/view combined).
        void setView(const std::string&);

        std::vector<std::string> icsList();
        std::vector<std::string> lookList();
        std::vector<std::string> viewList();

        //! List of OCIO presets.
        std::vector<std::string> presetsList();

        //! Set OCIO settings to a valid preset.
        void setPreset(const std::string& preset);

        //! Return a human readable summary of an OCIO preset.
        std::string presetSummary(const std::string& presetName);

        //! Create an OCIO preset from current defaults.
        void createPreset(const std::string& preset);

        //! Remove an existing OCIO preset.
        void removePreset(const std::string& preset);

        //! Load OCIO presets from disk.
        bool loadPresets(const std::string& fileName);

        //! Save OCIO presets to disk.
        bool savePresets(const std::string& fileName);

    } // namespace ocio
} // namespace mrv
