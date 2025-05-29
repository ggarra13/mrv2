// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/OpenSeparateAudioDialog.h>

#include <tlQtWidget/FileBrowserSystem.h>

#include <tlTimeline/Timeline.h>

#include <tlCore/String.h>

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

namespace tl
{
    namespace play_qt
    {
        struct OpenSeparateAudioDialog::Private
        {
            std::weak_ptr<system::Context> context;
            QString videoFileName;
            QString audioFileName;
            QLineEdit* videoLineEdit = nullptr;
            QLineEdit* audioLineEdit = nullptr;
        };

        OpenSeparateAudioDialog::OpenSeparateAudioDialog(
            const std::shared_ptr<system::Context>& context, QWidget* parent) :
            QDialog(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            setWindowTitle(tr("Open with Audio"));

            auto videoGroupBox = new QGroupBox(tr("Video"));
            p.videoLineEdit = new QLineEdit;
            auto videoBrowseButton = new QPushButton(tr("Browse"));

            auto audioGroupBox = new QGroupBox(tr("Audio"));
            p.audioLineEdit = new QLineEdit;
            auto audioBrowseButton = new QPushButton(tr("Browse"));

            auto buttonBox = new QDialogButtonBox;
            buttonBox->addButton(QDialogButtonBox::Ok);
            buttonBox->addButton(QDialogButtonBox::Cancel);

            auto layout = new QVBoxLayout;
            auto vLayout = new QVBoxLayout;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.videoLineEdit);
            hLayout->addWidget(videoBrowseButton);
            videoGroupBox->setLayout(hLayout);
            vLayout->addWidget(videoGroupBox);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.audioLineEdit);
            hLayout->addWidget(audioBrowseButton);
            audioGroupBox->setLayout(hLayout);
            vLayout->addWidget(audioGroupBox);
            layout->addLayout(vLayout);
            layout->addWidget(buttonBox);
            setLayout(layout);

            connect(
                p.videoLineEdit, SIGNAL(textChanged(const QString&)),
                SLOT(_videoLineEditCallback(const QString&)));

            connect(
                videoBrowseButton, SIGNAL(clicked()),
                SLOT(_browseVideoCallback()));

            connect(
                p.audioLineEdit, SIGNAL(textChanged(const QString&)),
                SLOT(_audioLineEditCallback(const QString&)));

            connect(
                audioBrowseButton, SIGNAL(clicked()),
                SLOT(_browseAudioCallback()));

            connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
            connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
        }

        OpenSeparateAudioDialog::~OpenSeparateAudioDialog() {}

        const QString& OpenSeparateAudioDialog::videoFileName() const
        {
            return _p->videoFileName;
        }

        const QString& OpenSeparateAudioDialog::audioFileName() const
        {
            return _p->audioFileName;
        }

        void
        OpenSeparateAudioDialog::_videoLineEditCallback(const QString& value)
        {
            _p->videoFileName = value.toUtf8().data();
        }

        void OpenSeparateAudioDialog::_browseVideoCallback()
        {
            TLRENDER_P();
            if (auto context = p.context.lock())
            {
                if (auto fileBrowserSystem =
                        context->getSystem<qtwidget::FileBrowserSystem>())
                {
                    fileBrowserSystem->open(
                        this,
                        [this](const file::Path& value)
                        {
                            if (!value.isEmpty())
                            {
                                const QString fileName =
                                    QString::fromUtf8(value.get().c_str());
                                _p->videoFileName = fileName;
                                _p->videoLineEdit->setText(fileName);
                            }
                        });
                }
            }
        }

        void
        OpenSeparateAudioDialog::_audioLineEditCallback(const QString& value)
        {
            TLRENDER_P();
            p.audioFileName = value.toUtf8().data();
        }

        void OpenSeparateAudioDialog::_browseAudioCallback()
        {
            TLRENDER_P();
            if (auto context = p.context.lock())
            {
                if (auto fileBrowserSystem =
                        context->getSystem<qtwidget::FileBrowserSystem>())
                {
                    fileBrowserSystem->open(
                        this,
                        [this](const file::Path& value)
                        {
                            if (!value.isEmpty())
                            {
                                const QString fileName =
                                    QString::fromUtf8(value.get().c_str());
                                _p->audioFileName = fileName;
                                _p->audioLineEdit->setText(fileName);
                            }
                        });
                }
            }
        }
    } // namespace play_qt
} // namespace tl
