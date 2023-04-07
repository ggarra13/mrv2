// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvFl/mrvTimeObject.h>

#include <any>
#include <vector>
#include <memory>

#define std_any std::any
#define std_any_cast std::any_cast
#define std_any_empty(x) !x.has_value()

namespace mrv
{
    namespace
    {
        // Some constants to avoid typos in long typing
        const char* kTextFont = "Annotations/Text Font";
        const char* kPenColorR = "Annotations/Pen Color R";
        const char* kPenColorG = "Annotations/Pen Color G";
        const char* kPenColorB = "Annotations/Pen Color B";
        const char* kPenSize = "Annotations/Pen Size";
        const char* kFontSize = "Annotations/Font Size";
        const char* kGhostPrevious = "Annotations/Ghost Previous";
        const char* kGhostNext = "Annotations/Ghost Next";
        const char* kAllFrames = "Annotations/All Frames";
    } // namespace

    //! Settings object.
    class SettingsObject
    {

    public:
        SettingsObject();
        ~SettingsObject();

        //! Get the list of keys in settings.
        const std::vector<std::string> keys() const;

        //! Get a settings value.
        std_any value(const std::string&);

        //! Get the list of recent files.
        const std::vector<std::string>& recentFiles() const;

        //! Get whether tooltips are enabled.
        bool hasToolTipsEnabled() const;

    public: // Q_SLOTS
        //! Set a settings value.
        void setValue(const std::string&, const std_any&);

        //! Set a default settings value.
        void setDefaultValue(const std::string&, const std_any&);

        //! Reset the settings to defaults.
        void reset();

        //! Add a recent file.
        void addRecentFile(const std::string&);

    private:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
