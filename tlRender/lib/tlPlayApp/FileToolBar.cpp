// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FileToolBar.h>

#include <tlPlayApp/App.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct FileToolBar::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
        };

        void FileToolBar::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_app::FileToolBar", context, parent);
            TLRENDER_P();

            p.actions = actions;

            p.buttons["Open"] = ui::ToolButton::create(context);
            p.buttons["Open"]->setIcon(p.actions["Open"]->icon);
            p.buttons["Open"]->setToolTip(p.actions["Open"]->toolTip);

            p.buttons["OpenSeparateAudio"] = ui::ToolButton::create(context);
            p.buttons["OpenSeparateAudio"]->setIcon(
                p.actions["OpenSeparateAudio"]->icon);
            p.buttons["OpenSeparateAudio"]->setToolTip(
                p.actions["OpenSeparateAudio"]->toolTip);

            p.buttons["Close"] = ui::ToolButton::create(context);
            p.buttons["Close"]->setIcon(p.actions["Close"]->icon);
            p.buttons["Close"]->setToolTip(p.actions["Close"]->toolTip);

            p.buttons["CloseAll"] = ui::ToolButton::create(context);
            p.buttons["CloseAll"]->setIcon(p.actions["CloseAll"]->icon);
            p.buttons["CloseAll"]->setToolTip(p.actions["CloseAll"]->toolTip);

            p.layout =
                ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["Open"]->setParent(p.layout);
            p.buttons["OpenSeparateAudio"]->setParent(p.layout);
            p.buttons["Close"]->setParent(p.layout);
            p.buttons["CloseAll"]->setParent(p.layout);

            p.buttons["Open"]->setClickedCallback(
                [this] { _p->actions["Open"]->callback(); });
            p.buttons["OpenSeparateAudio"]->setClickedCallback(
                [this] { _p->actions["OpenSeparateAudio"]->callback(); });
            p.buttons["Close"]->setClickedCallback(
                [this] { _p->actions["Close"]->callback(); });
            p.buttons["CloseAll"]->setClickedCallback(
                [this] { _p->actions["CloseAll"]->callback(); });

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->getFilesModel()->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _filesUpdate(value); });
        }

        FileToolBar::FileToolBar() :
            _p(new Private)
        {
        }

        FileToolBar::~FileToolBar() {}

        std::shared_ptr<FileToolBar> FileToolBar::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
            out->_init(actions, app, context, parent);
            return out;
        }

        void FileToolBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void FileToolBar::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            p.buttons["Close"]->setEnabled(!value.empty());
            p.buttons["CloseAll"]->setEnabled(!value.empty());
        }
    } // namespace play_app
} // namespace tl
