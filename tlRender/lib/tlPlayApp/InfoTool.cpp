// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/InfoTool.h>

#include <tlPlayApp/App.h>

#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/SearchBox.h>

namespace tl
{
    namespace play_app
    {
        struct InfoTool::Private
        {
            io::Info info;
            std::string search;

            std::shared_ptr<ui::SearchBox> searchBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<
                observer::ValueObserver<std::shared_ptr<timeline::Player> > >
                playerObserver;
        };

        void InfoTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Info, "tl::play_app::InfoTool", app, context, parent);
            TLRENDER_P();

            p.searchBox = ui::SearchBox::create(context);
            p.searchBox->setHStretch(ui::Stretch::Expanding);

            p.layout = ui::GridLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(p.layout);
            scrollWidget->setVStretch(ui::Stretch::Expanding);

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::None);
            scrollWidget->setParent(layout);
            auto hLayout = ui::HorizontalLayout::create(context, layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.searchBox->setParent(hLayout);
            _setWidget(layout);

            p.playerObserver = observer::
                ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _p->info = value ? value->getIOInfo() : io::Info();
                        _widgetUpdate();
                    });

            p.searchBox->setCallback(
                [this](const std::string& value)
                {
                    _p->search = value;
                    _widgetUpdate();
                });
        }

        InfoTool::InfoTool() :
            _p(new Private)
        {
        }

        InfoTool::~InfoTool() {}

        std::shared_ptr<InfoTool> InfoTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<InfoTool>(new InfoTool);
            out->_init(app, context, parent);
            return out;
        }

        void InfoTool::_widgetUpdate()
        {
            TLRENDER_P();
            auto children = p.layout->getChildren();
            for (const auto& child : children)
            {
                child->setParent(nullptr);
            }
            if (auto context = _context.lock())
            {
                int row = 0;
                for (const auto& tag : p.info.tags)
                {
                    bool filter = false;
                    if (!p.search.empty() &&
                        !string::contains(
                            tag.first, p.search,
                            string::Compare::CaseInsensitive) &&
                        !string::contains(
                            tag.second, p.search,
                            string::Compare::CaseInsensitive))
                    {
                        filter = true;
                    }
                    if (!filter)
                    {
                        auto label = ui::Label::create(
                            tag.first + ":", context, p.layout);
                        p.layout->setGridPos(label, row, 0);
                        label =
                            ui::Label::create(tag.second, context, p.layout);
                        p.layout->setGridPos(label, row, 1);
                        ++row;
                    }
                }
            }
        }
    } // namespace play_app
} // namespace tl
