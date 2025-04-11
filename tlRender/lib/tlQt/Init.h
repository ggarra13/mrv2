// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    //! Qt support
    namespace qt
    {
        //! Surface formats.
        enum class DefaultSurfaceFormat { None, OpenGL_4_1_CoreProfile };

        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void
        init(DefaultSurfaceFormat, const std::shared_ptr<system::Context>&);

        //! Qt support system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(
                DefaultSurfaceFormat, const std::shared_ptr<system::Context>&);

            System();

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(
                DefaultSurfaceFormat, const std::shared_ptr<system::Context>&);
        };
    } // namespace qt
} // namespace tl
