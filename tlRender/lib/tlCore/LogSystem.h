// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ICoreSystem.h>
#include <tlCore/ListObserver.h>

#include <chrono>

namespace tl
{
    //! Logging
    namespace log
    {
        //! Log types.
        enum class Type { Message, Warning, Error, kStatus };

        //! Log item.
        struct Item
        {
            float time = 0.F;
            std::string prefix;
            std::string message;
            Type type = Type::Message;
            std::string module;

            bool operator==(const Item&) const;
            bool operator!=(const Item&) const;
        };

        //! String conversion options.
        enum class StringConvert { kNone = 0, Time = 1, Prefix = 2 };

        //! Convert a log item to a string.
        std::string toString(const Item&, size_t options = 0);

        //! Log system.
        class System : public system::ICoreSystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            System();

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System>
            create(const std::shared_ptr<system::Context>&);

            //! Print to the log.
            void print(
                const std::string& prefix, const std::string&,
                Type = Type::Message, const std::string& module = "");

            //! Observe the log.
            std::shared_ptr<observer::IList<Item> > observeLog() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace log
} // namespace tl
