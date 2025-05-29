// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Image.h>

#include <future>

namespace tl
{
    namespace ui
    {
        //! Icon library.
        class IconLibrary : public std::enable_shared_from_this<IconLibrary>
        {
            TLRENDER_NON_COPYABLE(IconLibrary);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            IconLibrary();

        public:
            ~IconLibrary();

            //! Create a new icon library.
            static std::shared_ptr<IconLibrary>
            create(const std::shared_ptr<system::Context>&);

            //! Request an icon.
            std::future<std::shared_ptr<image::Image> >
            request(const std::string& name, float displayScale);

            //! Cancel requests.
            void cancelRequests();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl
