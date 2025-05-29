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

    //! Player application support
    namespace play
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);
    } // namespace play
} // namespace tl
