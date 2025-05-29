// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/RenderMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/RenderActions.h>

#include <tlPlay/RenderModel.h>

namespace tl
{
    namespace play_app
    {
        struct RenderMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<Menu> > menus;

            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> >
                imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<image::PixelType> >
                colorBufferObserver;
        };

        void RenderMenu::_init(
            const std::shared_ptr<RenderActions>& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions->getActions();

            p.menus["VideoLevels"] = addSubMenu("Video Levels");
            p.menus["VideoLevels"]->addItem(p.actions["FromFile"]);
            p.menus["VideoLevels"]->addItem(p.actions["FullRange"]);
            p.menus["VideoLevels"]->addItem(p.actions["LegalRange"]);

            p.menus["AlphaBlend"] = addSubMenu("Alpha Blend");
            p.menus["AlphaBlend"]->addItem(p.actions["AlphaBlendNone"]);
            p.menus["AlphaBlend"]->addItem(p.actions["AlphaBlendStraight"]);
            p.menus["AlphaBlend"]->addItem(
                p.actions["AlphaBlendPremultiplied"]);

            p.menus["ColorBuffer"] = addSubMenu("Color Buffer");
            std::vector<image::PixelType> colorBuffers =
                actions->getColorBuffers();
            for (auto type : colorBuffers)
            {
                std::stringstream ss;
                ss << type;
                p.menus["ColorBuffer"]->addItem(p.actions[ss.str()]);
            }

            p.imageOptionsObserver =
                observer::ValueObserver<timeline::ImageOptions>::create(
                    app->getRenderModel()->observeImageOptions(),
                    [this](const timeline::ImageOptions& value)
                    {
                        _p->menus["VideoLevels"]->setItemChecked(
                            _p->actions["FromFile"],
                            timeline::InputVideoLevels::FromFile ==
                                value.videoLevels);
                        _p->menus["VideoLevels"]->setItemChecked(
                            _p->actions["FullRange"],
                            timeline::InputVideoLevels::FullRange ==
                                value.videoLevels);
                        _p->menus["VideoLevels"]->setItemChecked(
                            _p->actions["LegalRange"],
                            timeline::InputVideoLevels::LegalRange ==
                                value.videoLevels);

                        _p->menus["AlphaBlend"]->setItemChecked(
                            _p->actions["AlphaBlendNone"],
                            timeline::AlphaBlend::kNone == value.alphaBlend);
                        _p->menus["AlphaBlend"]->setItemChecked(
                            _p->actions["AlphaBlendStraight"],
                            timeline::AlphaBlend::Straight == value.alphaBlend);
                        _p->menus["AlphaBlend"]->setItemChecked(
                            _p->actions["AlphaBlendPremultiplied"],
                            timeline::AlphaBlend::Premultiplied ==
                                value.alphaBlend);
                    });

            p.colorBufferObserver =
                observer::ValueObserver<image::PixelType>::create(
                    app->getRenderModel()->observeColorBuffer(),
                    [this, colorBuffers](image::PixelType value)
                    {
                        for (auto type : colorBuffers)
                        {
                            std::stringstream ss;
                            ss << type;
                            _p->menus["ColorBuffer"]->setItemChecked(
                                _p->actions[ss.str()], type == value);
                        }
                    });
        }

        RenderMenu::RenderMenu() :
            _p(new Private)
        {
        }

        RenderMenu::~RenderMenu() {}

        std::shared_ptr<RenderMenu> RenderMenu::create(
            const std::shared_ptr<RenderActions>& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RenderMenu>(new RenderMenu);
            out->_init(actions, app, context, parent);
            return out;
        }

        void RenderMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }
    } // namespace play_app
} // namespace tl
