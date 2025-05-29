// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace gl_tests
    {
        class TextureTest : public tests::ITest
        {
        protected:
            TextureTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<TextureTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _texture();
            void _textureAtlas();
        };
    } // namespace gl_tests
} // namespace tl
