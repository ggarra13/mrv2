// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ValueObserver.h>

#include <nlohmann/json.hpp>

#include <string>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        class App;

        //! Settings.
        class Settings : public std::enable_shared_from_this<Settings>
        {
            TLRENDER_NON_COPYABLE(Settings);

        protected:
            void _init(
                const std::string fileName, bool reset,
                const std::shared_ptr<system::Context>&);

            Settings();

        public:
            virtual ~Settings();

            //! Create a new settings.
            static std::shared_ptr<Settings> create(
                const std::string fileName, bool reset,
                const std::shared_ptr<system::Context>&);

            //! Get a value.
            template <typename T> T getValue(const std::string&) const;

            //! Observe value changes.
            std::shared_ptr<observer::IValue<std::string> >
            observeValues() const;

            //! Set a default value.
            template <typename T> void setDefaultValue(const std::string&, T);

            //! Set a value.
            template <typename T> void setValue(const std::string&, T);

            //! Reset the settings to defaults.
            void reset();

        private:
            void _read();
            void _write();

            std::weak_ptr<system::Context> _context;
            std::string _fileName;
            nlohmann::json _defaultValues;
            nlohmann::json _values;
            std::shared_ptr<observer::Value<std::string> > _observer;
        };
    } // namespace play
} // namespace tl

#include <tlPlay/SettingsInline.h>
