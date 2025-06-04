// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace file
    {
        bool listFilter(const std::string&, const ListOptions&);

        void listSequence(
            const std::string& path, const std::string& fileName,
            std::vector<FileInfo>&, const ListOptions&);

        void _list(
            const std::string&, std::vector<FileInfo>&,
            const ListOptions& = ListOptions());
    } // namespace file
} // namespace tl
