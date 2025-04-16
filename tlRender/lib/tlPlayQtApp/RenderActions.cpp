// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/RenderActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>

#include <tlPlay/RenderModel.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct RenderActions::Private
        {
            App* app = nullptr;
            std::vector<image::PixelType> colorBuffers;
            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;
            QScopedPointer<QMenu> menu;
            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> >
                imageOptionsObserver;
        };

        RenderActions::RenderActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["VideoLevels/FromFile"] = new QAction(this);
            p.actions["VideoLevels/FromFile"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(
                    timeline::InputVideoLevels::FromFile));
            p.actions["VideoLevels/FromFile"]->setCheckable(true);
            p.actions["VideoLevels/FromFile"]->setText(tr("From File"));
            p.actions["VideoLevels/FullRange"] = new QAction(this);
            p.actions["VideoLevels/FullRange"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(
                    timeline::InputVideoLevels::FullRange));
            p.actions["VideoLevels/FullRange"]->setCheckable(true);
            p.actions["VideoLevels/FullRange"]->setText(tr("Full Range"));
            p.actions["VideoLevels/LegalRange"] = new QAction(this);
            p.actions["VideoLevels/LegalRange"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(
                    timeline::InputVideoLevels::LegalRange));
            p.actions["VideoLevels/LegalRange"]->setCheckable(true);
            p.actions["VideoLevels/LegalRange"]->setText(tr("Legal Range"));
            p.actionGroups["VideoLevels"] = new QActionGroup(this);
            p.actionGroups["VideoLevels"]->addAction(
                p.actions["VideoLevels/FromFile"]);
            p.actionGroups["VideoLevels"]->addAction(
                p.actions["VideoLevels/FullRange"]);
            p.actionGroups["VideoLevels"]->addAction(
                p.actions["VideoLevels/LegalRange"]);

            p.actions["AlphaBlend/None"] = new QAction(this);
            p.actions["AlphaBlend/None"]->setData(
                QVariant::fromValue<timeline::AlphaBlend>(
                    timeline::AlphaBlend::kNone));
            p.actions["AlphaBlend/None"]->setCheckable(true);
            p.actions["AlphaBlend/None"]->setText(tr("None"));
            p.actions["AlphaBlend/Straight"] = new QAction(this);
            p.actions["AlphaBlend/Straight"]->setData(
                QVariant::fromValue<timeline::AlphaBlend>(
                    timeline::AlphaBlend::Straight));
            p.actions["AlphaBlend/Straight"]->setCheckable(true);
            p.actions["AlphaBlend/Straight"]->setText(tr("Straight"));
            p.actions["AlphaBlend/Premultiplied"] = new QAction(this);
            p.actions["AlphaBlend/Premultiplied"]->setData(
                QVariant::fromValue<timeline::AlphaBlend>(
                    timeline::AlphaBlend::Premultiplied));
            p.actions["AlphaBlend/Premultiplied"]->setCheckable(true);
            p.actions["AlphaBlend/Premultiplied"]->setText(tr("Premultiplied"));
            p.actionGroups["AlphaBlend"] = new QActionGroup(this);
            p.actionGroups["AlphaBlend"]->addAction(
                p.actions["AlphaBlend/None"]);
            p.actionGroups["AlphaBlend"]->addAction(
                p.actions["AlphaBlend/Straight"]);
            p.actionGroups["AlphaBlend"]->addAction(
                p.actions["AlphaBlend/Premultiplied"]);

            p.colorBuffers.push_back(image::PixelType::RGBA_U8);
            p.colorBuffers.push_back(image::PixelType::RGBA_F16);
            p.colorBuffers.push_back(image::PixelType::RGBA_F32);
            p.actionGroups["ColorBuffer"] = new QActionGroup(this);
            for (auto type : p.colorBuffers)
            {
                std::stringstream ss;
                ss << type;
                QString name = QString::fromUtf8("ColorBuffer/");
                name.append(QString::fromUtf8(ss.str().c_str()));
                p.actions[name] = new QAction(this);
                p.actions[name]->setData(
                    QVariant::fromValue<image::PixelType>(type));
                p.actions[name]->setCheckable(true);
                p.actions[name]->setText(QString::fromUtf8(ss.str().c_str()));
                p.actionGroups["ColorBuffer"]->addAction(p.actions[name]);
            }

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Render"));
            auto videoLevelsMenu = p.menu->addMenu(tr("Video Levels"));
            videoLevelsMenu->addAction(p.actions["VideoLevels/FromFile"]);
            videoLevelsMenu->addAction(p.actions["VideoLevels/FullRange"]);
            videoLevelsMenu->addAction(p.actions["VideoLevels/LegalRange"]);
            auto alphaBlendMenu = p.menu->addMenu(tr("Alpha Blend"));
            alphaBlendMenu->addAction(p.actions["AlphaBlend/None"]);
            alphaBlendMenu->addAction(p.actions["AlphaBlend/Straight"]);
            alphaBlendMenu->addAction(p.actions["AlphaBlend/Premultiplied"]);
            auto colorBufferMenu = p.menu->addMenu(tr("Color Buffer"));
            for (auto action : p.actionGroups["ColorBuffer"]->actions())
            {
                colorBufferMenu->addAction(action);
            }

            _actionsUpdate();

            connect(
                p.actionGroups["VideoLevels"], &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->renderModel()->getImageOptions();
                    options.videoLevels =
                        action->data().value<timeline::InputVideoLevels>();
                    app->renderModel()->setImageOptions(options);
                });

            connect(
                _p->actionGroups["AlphaBlend"], &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->renderModel()->getImageOptions();
                    options.alphaBlend =
                        action->data().value<timeline::AlphaBlend>();
                    app->renderModel()->setImageOptions(options);
                });

            connect(
                _p->actionGroups["ColorBuffer"], &QActionGroup::triggered,
                [app](QAction* action)
                {
                    app->renderModel()->setColorBuffer(
                        action->data().value<image::PixelType>());
                });

            p.imageOptionsObserver =
                observer::ValueObserver<timeline::ImageOptions>::create(
                    app->renderModel()->observeImageOptions(),
                    [this](const timeline::ImageOptions&)
                    { _actionsUpdate(); });
        }

        RenderActions::~RenderActions() {}

        const QMap<QString, QAction*>& RenderActions::actions() const
        {
            return _p->actions;
        }

        QMenu* RenderActions::menu() const
        {
            return _p->menu.get();
        }

        void RenderActions::_actionsUpdate()
        {
            TLRENDER_P();
            auto renderModel = p.app->renderModel();
            const auto& imageOptions = renderModel->getImageOptions();
            {
                QSignalBlocker blocker(p.actionGroups["VideoLevels"]);
                for (auto action : p.actionGroups["VideoLevels"]->actions())
                {
                    if (action->data().value<timeline::InputVideoLevels>() ==
                        imageOptions.videoLevels)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actionGroups["AlphaBlend"]);
                for (auto action : p.actionGroups["AlphaBlend"]->actions())
                {
                    if (action->data().value<timeline::AlphaBlend>() ==
                        imageOptions.alphaBlend)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                const image::PixelType colorBuffer =
                    renderModel->getColorBuffer();
                QSignalBlocker blocker(p.actionGroups["ColorBuffer"]);
                for (auto action : p.actionGroups["ColorBuffer"]->actions())
                {
                    if (action->data().value<image::PixelType>() == colorBuffer)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
        }
    } // namespace play_qt
} // namespace tl
