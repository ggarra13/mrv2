// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/CompareToolBar.h>

#include <tlPlayApp/App.h>

#include <tlPlay/FilesModel.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct CompareToolBar::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<ui::ButtonGroup> buttonGroup;
            std::map<timeline::CompareMode, std::shared_ptr<ui::ToolButton> >
                buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
        };

        void CompareToolBar::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_app::CompareToolBar", context, parent);
            TLRENDER_P();

            p.actions = actions;

            p.buttonGroup =
                ui::ButtonGroup::create(ui::ButtonGroupType::Radio, context);
            const auto enums = timeline::getCompareModeEnums();
            const auto labels = timeline::getCompareModeLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto mode = enums[i];
                p.buttons[mode] = ui::ToolButton::create(context);
                p.buttons[mode]->setCheckable(true);
                p.buttons[mode]->setIcon(p.actions[labels[i]]->icon);
                p.buttons[mode]->setToolTip(p.actions[labels[i]]->toolTip);
                p.buttonGroup->addButton(p.buttons[mode]);
            }

            p.layout =
                ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::kNone);
            for (size_t i = 0; i < enums.size(); ++i)
            {
                p.buttons[enums[i]]->setParent(p.layout);
            }

            auto appWeak = std::weak_ptr<App>(app);
            p.buttonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (value)
                        {
                            auto options =
                                app->getFilesModel()->getCompareOptions();
                            options.mode =
                                static_cast<timeline::CompareMode>(index);
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    }
                });

            p.compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    app->getFilesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { _compareUpdate(value); });
        }

        CompareToolBar::CompareToolBar() :
            _p(new Private)
        {
        }

        CompareToolBar::~CompareToolBar() {}

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(actions, app, context, parent);
            return out;
        }

        void CompareToolBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CompareToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void
        CompareToolBar::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            p.buttonGroup->setChecked(static_cast<int>(value.mode), true);
        }
    } // namespace play_app
} // namespace tl
