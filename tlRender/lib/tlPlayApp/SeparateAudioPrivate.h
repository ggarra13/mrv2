
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/SeparateAudioDialog.h>

namespace tl
{
    namespace play_app
    {
        class SeparateAudioWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(SeparateAudioWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SeparateAudioWidget();

        public:
            virtual ~SeparateAudioWidget();

            static std::shared_ptr<SeparateAudioWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<
                             void(const file::Path&, const file::Path&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
