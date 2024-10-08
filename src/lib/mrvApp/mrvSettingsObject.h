// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <any>
#include <vector>
#include <memory>

#include <tlCore/Util.h>
#include <tlCore/Memory.h>

#include "mrvFl/mrvIO.h"

#include "mrvApp/mrvStdAnyHelper.h"

#define std_any std::any
#define std_any_empty(x) !x.has_value()

namespace mrv
{
    using namespace tl;

    namespace
    {
        // Some constants to avoid typos in long typing
        const char* kTextFont = "Annotations/Text Font";
        const char* kPenColorR = "Annotations/Pen Color R";
        const char* kPenColorG = "Annotations/Pen Color G";
        const char* kPenColorB = "Annotations/Pen Color B";
        const char* kPenColorA = "Annotations/Pen Color A";
        const char* kOldPenColorR = "Annotations/Old Pen Color R";
        const char* kOldPenColorG = "Annotations/Old Pen Color G";
        const char* kOldPenColorB = "Annotations/Old Pen Color B";
        const char* kOldPenColorA = "Annotations/Old Pen Color A";
        const char* kLaser = "Annotations/Laser";
        const char* kPenSize = "Annotations/Pen Size";
        const char* kSoftBrush = "Annotations/Soft Brush";
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

        //! Get a value safely.
        template < typename T > T getValue(const std::string&);

        //! Check if it has a value.
        bool hasValue(const std::string&);
        
        //! Get default value safely.
        template < typename T > T getDefaultValue(const std::string&);

        //! Get the list of recent files.
        const std::vector<std::string>& recentFiles() const;

        //! Get the list of recent hosts.
        const std::vector<std::string>& recentHosts() const;

        //! Get the list of python scripts.
        const std::vector<std::string>& pythonScripts() const;

        const std::string pythonScript(size_t index) const;

        //! Get whether tooltips are enabled.
        bool hasToolTipsEnabled() const;

        //! Set a settings value.
        void setValue(const std::string&, const std_any&);

        //! Set a default settings value.
        void setDefaultValue(const std::string&, const std_any&);

        //! Reset the settings to defaults.
        void reset();

        //! Add a recent file.
        void addRecentFile(const std::string&);

        //! Add a recent host.
        void addRecentHost(const std::string&);

        //! Add a recent host.
        void addPythonScript(const std::string&);

    private:
        //! Get a settings value without error checking.
        std::any _value(const std::string&);
        //! Get a settings default value without error checking.
        std::any _defaultValue(const std::string&);

        TLRENDER_PRIVATE();
    };

} // namespace mrv
