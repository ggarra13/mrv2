// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Init.h>

#include <tlQtWidget/FileBrowserSystem.h>

#include <tlQt/Init.h>

#include <tlTimelineUI/Init.h>

#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QDir>
#include <QFontDatabase>
#include <QMap>

#include <iostream>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlQtWidget);
}

namespace tl
{
    namespace qtwidget
    {
        void init(
            qt::DefaultSurfaceFormat defaultSurfaceFormat,
            const std::shared_ptr<system::Context>& context)
        {
            timelineui::init(context);
            qt::init(defaultSurfaceFormat, context);
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
            if (!context->getSystem<FileBrowserSystem>())
            {
                context->addSystem(FileBrowserSystem::create(context));
            }
        }

        void initFonts(const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> fontFamilyList;
            for (const auto& i : std::vector<std::string>(
                     {"NotoMono-Regular", "NotoSans-Bold", "NotoSans-Regular"}))
            {
                const auto font = image::getFontData(i);
                const int id =
                    QFontDatabase::addApplicationFontFromData(QByteArray(
                        reinterpret_cast<const char*>(font.data()),
                        font.size()));
                for (const auto& j : QFontDatabase::applicationFontFamilies(id))
                {
                    fontFamilyList.push_back(j.toUtf8().data());
                }
            }
            context->log(
                "tl::qtwidget::initFonts",
                string::Format("Added Qt application fonts: {0}")
                    .arg(string::join(fontFamilyList, ", ")));
        }

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::qtwidget::System", context);

            qtInitResources();
        }

        System::System() {}

        System::~System() {}

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(context);
            return out;
        }
    } // namespace qtwidget
} // namespace tl
