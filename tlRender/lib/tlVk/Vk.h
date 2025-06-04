// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <FL/Fl_Vk_Context.H>

#ifdef __APPLE__
#  define MRV2_NO_VMA
#endif

namespace tl
{
    namespace vlk
    {
        static constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
    } // namespace vlk
} // namespace tl
