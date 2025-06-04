// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace gl
    {
        //! Get the glReadPixels format.
        unsigned int getReadPixelsFormat(image::PixelType);

        //! Get the glReadPixels type.
        unsigned int getReadPixelsType(image::PixelType);

        //! Set whether an OpenGL capability is enabled and restore it to the
        //! previous value when finished.
        class SetAndRestore
        {
        public:
            SetAndRestore(unsigned int, bool);

            ~SetAndRestore();

        private:
            TLRENDER_PRIVATE();
        };

        //! Get an OpenGL error label.
        std::string getErrorLabel(unsigned int);
    } // namespace gl
} // namespace tl
