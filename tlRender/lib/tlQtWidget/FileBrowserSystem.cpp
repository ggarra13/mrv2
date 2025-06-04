// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FileBrowserSystem.h>

#include <tlTimeline/Util.h>

#include <tlCore/File.h>

#include <QFileDialog>

#if defined(TLRENDER_NFD)
#    include <nfd.hpp>
#endif // TLRENDER_NFD

namespace tl
{
    namespace qtwidget
    {
        struct FileBrowserSystem::Private
        {
            bool native = true;
            std::string path;
            QStringList extensions;
        };

        void FileBrowserSystem::_init(
            const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::ui::FileBrowserSystem", context);
            TLRENDER_P();

            p.path = file::getCWD();

            std::vector<std::string> extensions;
            for (const auto& i : timeline::getExtensions(
                     static_cast<int>(io::FileType::Movie) |
                         static_cast<int>(io::FileType::Sequence) |
                         static_cast<int>(io::FileType::Audio),
                     context))
            {
                p.extensions.push_back(QString::fromUtf8(i.c_str()));
            }

#if defined(TLRENDER_NFD)
            NFD::Init();
#endif // TLRENDER_NFD
        }

        FileBrowserSystem::FileBrowserSystem() :
            _p(new Private)
        {
        }

        FileBrowserSystem::~FileBrowserSystem()
        {
#if defined(TLRENDER_NFD)
            NFD::Quit();
#endif // TLRENDER_NFD
        }

        std::shared_ptr<FileBrowserSystem> FileBrowserSystem::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out =
                std::shared_ptr<FileBrowserSystem>(new FileBrowserSystem);
            out->_init(context);
            return out;
        }

        void FileBrowserSystem::open(
            QWidget* window,
            const std::function<void(const file::Path&)>& callback)
        {
            TLRENDER_P();
            bool native = p.native;
#if defined(TLRENDER_NFD)
            if (native)
            {
                nfdu8char_t* outPath = nullptr;
                NFD::OpenDialog(outPath);
                if (outPath)
                {
                    if (callback)
                    {
                        callback(file::Path(outPath));
                    }
                    NFD::FreePath(outPath);
                }
            }
#else  // TLRENDER_NFD
            native = false;
#endif // TLRENDER_NFD
            if (!native)
            {
                QString filter;
                if (!_p->extensions.isEmpty())
                {
                    filter.append(QObject::tr("Files"));
                    filter.append(" (");
                    QStringList extensions;
                    Q_FOREACH (QString i, _p->extensions)
                    {
                        extensions.push_back(QString("*%1").arg(i));
                    }
                    filter.append(extensions.join(' '));
                    filter.append(")");
                }
                const auto fileName = QFileDialog::getOpenFileName(
                    window, QObject::tr("Open"),
                    QString::fromUtf8(p.path.c_str()));
                if (callback)
                {
                    callback(file::Path(fileName.toUtf8().data()));
                }
            }
        }

        bool FileBrowserSystem::isNativeFileDialog() const
        {
            return _p->native;
        }

        void FileBrowserSystem::setNativeFileDialog(bool value)
        {
            _p->native = value;
        }

        const std::string& FileBrowserSystem::getPath() const
        {
            return _p->path;
        }

        void FileBrowserSystem::setPath(const std::string& value)
        {
            _p->path = value;
        }
    } // namespace qtwidget
} // namespace tl
