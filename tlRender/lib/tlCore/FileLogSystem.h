// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

#include <filesystem>

namespace tl
{
    namespace file
    {
        //! File logging system.
        class FileLogSystem : public system::ICoreSystem
        {
            TLRENDER_NON_COPYABLE(FileLogSystem);

        protected:
            void _init(
                const std::shared_ptr<system::Context>& context,
                const std::filesystem::path& path);

            FileLogSystem();

        public:
            virtual ~FileLogSystem();

            //! Create a new log system.
            static std::shared_ptr<FileLogSystem> create(
                const std::shared_ptr<system::Context>& context,
                const std::filesystem::path& path);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace file
} // namespace tl
