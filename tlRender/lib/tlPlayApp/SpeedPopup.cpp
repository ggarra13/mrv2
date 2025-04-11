// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SpeedPopup.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/Divider.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct SpeedPopup::Private
        {
            std::vector<double> speeds;
            std::vector<std::shared_ptr<ui::ListButton> > buttons;
            std::shared_ptr<ui::ButtonGroup> buttonGroup;
            std::shared_ptr<ui::VerticalLayout> layout;
            std::function<void(double)> callback;
        };

        void SpeedPopup::_init(
            double defaultSpeed,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IMenuPopup::_init("tl::play_app::SpeedPopup", context, parent);
            TLRENDER_P();

            p.speeds = {
                1.0,
                3.0,
                6.0,
                12.0,
                16.0,
                18.0,
                24000.0 / 1001.0,
                24.0,
                30000.0 / 1001.0,
                30.0,
                48.0,
                60000.0 / 1001.0,
                60.0,
                96.0,
                120.0,
                defaultSpeed};

            p.buttonGroup =
                ui::ButtonGroup::create(ui::ButtonGroupType::Click, context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::None);
            setWidget(p.layout);

            _menuUpdate();

            auto weak = std::weak_ptr<SpeedPopup>(
                std::dynamic_pointer_cast<SpeedPopup>(shared_from_this()));
            p.buttonGroup->setClickedCallback(
                [weak](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        if (widget->_p->callback)
                        {
                            widget->_p->callback(widget->_p->speeds[value]);
                        }
                    }
                });
        }

        SpeedPopup::SpeedPopup() :
            _p(new Private)
        {
        }

        SpeedPopup::~SpeedPopup() {}

        std::shared_ptr<SpeedPopup> SpeedPopup::create(
            double defaultSpeed,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SpeedPopup>(new SpeedPopup);
            out->_init(defaultSpeed, context, parent);
            return out;
        }

        void SpeedPopup::setCallback(const std::function<void(double)>& value)
        {
            _p->callback = value;
        }

        void SpeedPopup::_menuUpdate()
        {
            TLRENDER_P();
            auto children = p.layout->getChildren();
            for (const auto& child : children)
            {
                child->setParent(nullptr);
            }
            p.buttons.clear();
            p.buttonGroup->clearButtons();
            if (auto context = _context.lock())
            {
                for (size_t i = 0; i < p.speeds.size(); ++i)
                {
                    const bool last = (p.speeds.size() - 1) == i;
                    const bool secondToLast =
                        p.speeds.size() > 1 && (p.speeds.size() - 2) == i;
                    auto button =
                        ui::ListButton::create(context, shared_from_this());
                    button->setText(
                        last
                            ? string::Format("Default: {0}").arg(p.speeds[i], 2)
                            : string::Format("{0}").arg(p.speeds[i], 2));
                    button->setParent(p.layout);
                    p.buttons.push_back(button);
                    p.buttonGroup->addButton(button);
                    if (secondToLast)
                    {
                        ui::Divider::create(
                            ui::Orientation::Vertical, context, p.layout);
                    }
                }
            }
        }
    } // namespace play_app
} // namespace tl
