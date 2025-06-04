// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace play
    {
        //! Get the information label.
        std::string infoLabel(const file::Path&, const io::Info&);

        //! Get the information tooltip.
        std::string infoToolTip(const file::Path&, const io::Info&);
    } // namespace play
} // namespace tl
