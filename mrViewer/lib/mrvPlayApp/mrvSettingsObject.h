// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <mrvFl/mrvTimeObject.h>

#include <any/any.hpp>
#include <vector>
#include <memory>

#ifdef LINB_ANY_HPP
#  define std_any linb::any
#  define std_any_cast linb::any_cast
#else
#  define std_any std::any
#  define std_any_cast std::any_cast
#endif

namespace mrv
{
    //! Settings object.
    class SettingsObject 
    {

    public:
        SettingsObject(
            bool reset,
            TimeObject*);

        ~SettingsObject();

        //! Get a settings value.
        std_any value(const std::string&);

        //! Get the list of recent files.
        const std::vector<std::string>& recentFiles() const;

        //! Get whether tooltips are enabled.
        bool hasToolTipsEnabled() const;

    public:  // Q_SLOTS
        //! Set a settings value.
        void setValue(const std::string&, const std_any&);

        //! Set a default settings value.
        void setDefaultValue(const std::string&, const std_any&);

        //! Reset the settings to defaults.
        void reset();

        //! Add a recent file.
        void addRecentFile(const std::string&);

        //! Set whether tooltips are enabled.
        void setToolTipsEnabled(bool);

        // Q_SIGNALS:
        //! This signal is emitted when a settings valu// e is changed.
        // void valueChanged(const std::string&, const std_any&);

        // //! This signal is emitted when the recent files list is changed.
        // void recentFilesChanged(const std::vector<std::string>&);

        // //! This signal is emitted when the tooltips enabled state is changed.
        // void toolTipsEnabledChanged(bool);

    private:
        // void _toolTipsUpdate();

        TLRENDER_PRIVATE();
    };
}
