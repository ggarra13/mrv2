// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SeparateAudioPrivate.h>

#include <tlUI/Divider.h>
#include <tlUI/FileEdit.h>
#include <tlUI/GroupBox.h>
#include <tlUI/Label.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Spacer.h>

namespace tl
{
    namespace play_app
    {
        struct SeparateAudioWidget::Private
        {
            std::shared_ptr<ui::FileEdit> videoFileEdit;
            std::shared_ptr<ui::FileEdit> audioFileEdit;
            std::shared_ptr<ui::PushButton> okButton;
            std::shared_ptr<ui::PushButton> cancelButton;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::function<void(const file::Path&, const file::Path&)> callback;
            std::function<void(void)> cancelCallback;
        };

        void SeparateAudioWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_app::SeparateAudioWidget", context, parent);
            TLRENDER_P();

            setHStretch(ui::Stretch::Expanding);
            _setMouseHover(true);
            _setMousePress(true);

            p.videoFileEdit = ui::FileEdit::create(context);

            p.audioFileEdit = ui::FileEdit::create(context);

            p.okButton = ui::PushButton::create("OK", context);
            p.cancelButton = ui::PushButton::create("Cancel", context);

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            auto label =
                ui::Label::create("Open Separate Audio", context, p.layout);
            label->setMarginRole(ui::SizeRole::MarginSmall);
            label->setBackgroundRole(ui::ColorRole::Button);
            auto vLayout = ui::VerticalLayout::create(context, p.layout);
            vLayout->setVStretch(ui::Stretch::Expanding);
            vLayout->setMarginRole(ui::SizeRole::MarginSmall);
            vLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto groupBox = ui::GroupBox::create("Video", context, vLayout);
            p.videoFileEdit->setParent(groupBox);
            groupBox = ui::GroupBox::create("Audio", context, vLayout);
            p.audioFileEdit->setParent(groupBox);
            auto spacer =
                ui::Spacer::create(ui::Orientation::Vertical, context, vLayout);
            spacer->setSpacingRole(ui::SizeRole::None);
            spacer->setVStretch(ui::Stretch::Expanding);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginSmall);
            hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            spacer = ui::Spacer::create(
                ui::Orientation::Horizontal, context, hLayout);
            spacer->setSpacingRole(ui::SizeRole::None);
            spacer->setHStretch(ui::Stretch::Expanding);
            p.okButton->setParent(hLayout);
            p.cancelButton->setParent(hLayout);

            p.okButton->setClickedCallback(
                [this]
                {
                    if (_p->callback)
                    {
                        _p->callback(
                            _p->videoFileEdit->getPath(),
                            _p->audioFileEdit->getPath());
                    }
                });

            p.cancelButton->setClickedCallback(
                [this]
                {
                    if (_p->cancelCallback)
                    {
                        _p->cancelCallback();
                    }
                });
        }

        SeparateAudioWidget::SeparateAudioWidget() :
            _p(new Private)
        {
        }

        SeparateAudioWidget::~SeparateAudioWidget() {}

        std::shared_ptr<SeparateAudioWidget> SeparateAudioWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out =
                std::shared_ptr<SeparateAudioWidget>(new SeparateAudioWidget);
            out->_init(context, parent);
            return out;
        }

        void SeparateAudioWidget::setCallback(
            const std::function<void(const file::Path&, const file::Path&)>&
                value)
        {
            _p->callback = value;
        }

        void SeparateAudioWidget::setCancelCallback(
            const std::function<void(void)>& value)
        {
            _p->cancelCallback = value;
        }

        void SeparateAudioWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void SeparateAudioWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    } // namespace play_app
} // namespace tl
