// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>

namespace tl
{
    namespace ui
    {
        //! Base class for clipboards.
        class IClipboard : public std::enable_shared_from_this<IClipboard>
        {
            TLRENDER_NON_COPYABLE(IClipboard);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            IClipboard();

        public:
            virtual ~IClipboard() = 0;

            //! Get the clipboard text.
            virtual std::string getText() const = 0;

            //! Set the clipboard text.
            virtual void setText(const std::string&) = 0;

        private:
            std::weak_ptr<system::Context> _context;
        };
    } // namespace ui
} // namespace tl
