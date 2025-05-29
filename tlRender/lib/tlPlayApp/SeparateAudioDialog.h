// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IDialog.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Separate audio dialog.
        class SeparateAudioDialog : public ui::IDialog
        {
            TLRENDER_NON_COPYABLE(SeparateAudioDialog);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SeparateAudioDialog();

        public:
            virtual ~SeparateAudioDialog();

            static std::shared_ptr<SeparateAudioDialog> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<
                             void(const file::Path&, const file::Path&)>&);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
