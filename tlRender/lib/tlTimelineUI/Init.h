// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "Namespace.h"

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Timeline user interface
    namespace TIMELINEUI
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);
    } // namespace TIMELINEUI
} // namespace tl
