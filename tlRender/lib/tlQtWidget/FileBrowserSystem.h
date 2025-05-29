// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

#include <tlCore/Path.h>

class QWidget;

namespace tl
{
    namespace qtwidget
    {
        //! File browser system.
        class FileBrowserSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(FileBrowserSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            FileBrowserSystem();

        public:
            virtual ~FileBrowserSystem();

            //! Create a new system.
            static std::shared_ptr<FileBrowserSystem>
            create(const std::shared_ptr<system::Context>&);

            //! Open the file browser.
            void open(QWidget*, const std::function<void(const file::Path&)>&);

            //! Get whether the native file dialog is used.
            bool isNativeFileDialog() const;

            //! Set whether the native file dialog is used.
            void setNativeFileDialog(bool);

            //! Get the path.
            const std::string& getPath() const;

            //! Set the path.
            void setPath(const std::string&);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
