// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! OpenGL support
    namespace gl
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! Initialize GLAD.
        void initGLAD();
    } // namespace gl
} // namespace tl
