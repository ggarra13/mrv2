// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SettingsToolPrivate.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlPlay/Settings.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <tlQt/MetaTypes.h>

#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QSpinBox* cacheSizeSpinBox = nullptr;
            QDoubleSpinBox* readAheadSpinBox = nullptr;
            QDoubleSpinBox* readBehindSpinBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        CacheSettingsWidget::CacheSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.cacheSizeSpinBox = new QSpinBox;
            p.cacheSizeSpinBox->setRange(0, 1024);

            p.readAheadSpinBox = new QDoubleSpinBox;
            p.readAheadSpinBox->setRange(0.0, 60.0);

            p.readBehindSpinBox = new QDoubleSpinBox;
            p.readBehindSpinBox->setRange(0, 60.0);

            auto layout = new QFormLayout;
            layout->addRow(tr("Cache size (GB):"), p.cacheSizeSpinBox);
            layout->addRow(tr("Read ahead (seconds):"), p.readAheadSpinBox);
            layout->addRow(tr("Read behind (seconds):"), p.readBehindSpinBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.cacheSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                { _p->settings->setValue("Cache/Size", value); });

            connect(
                p.readAheadSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                { _p->settings->setValue("Cache/ReadAhead", value); });

            connect(
                p.readBehindSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                { _p->settings->setValue("Cache/ReadBehind", value); });
        }

        CacheSettingsWidget::~CacheSettingsWidget() {}

        void CacheSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("Cache/Size" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.cacheSizeSpinBox);
                p.cacheSizeSpinBox->setValue(
                    p.settings->getValue<int>("Cache/Size"));
            }
            if ("Cache/ReadAhead" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.readAheadSpinBox);
                p.readAheadSpinBox->setValue(
                    p.settings->getValue<double>("Cache/ReadAhead"));
            }
            if ("Cache/ReadBehind" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.readBehindSpinBox);
                p.readBehindSpinBox->setValue(
                    p.settings->getValue<double>("Cache/ReadBehind"));
            }
        }

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QComboBox* audioComboBox = nullptr;
            QLineEdit* audioFileName = nullptr;
            QLineEdit* audioDirectory = nullptr;
            QSpinBox* maxDigitsSpinBox = nullptr;
            QSpinBox* threadCountSpinBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        FileSequenceSettingsWidget::FileSequenceSettingsWidget(
            App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.audioComboBox = new QComboBox;
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                p.audioComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioFileName = new QLineEdit;

            p.audioDirectory = new QLineEdit;

            p.maxDigitsSpinBox = new QSpinBox;
            p.maxDigitsSpinBox->setRange(0, 255);

            p.threadCountSpinBox = new QSpinBox;
            p.threadCountSpinBox->setRange(1, 64);

            auto layout = new QFormLayout;
            layout->addRow(tr("Audio:"), p.audioComboBox);
            layout->addRow(tr("Audio file name:"), p.audioFileName);
            layout->addRow(tr("Audio directory:"), p.audioDirectory);
            layout->addRow(tr("Maximum digits:"), p.maxDigitsSpinBox);
            layout->addRow(tr("I/O threads:"), p.threadCountSpinBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.audioComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->settings->setValue(
                        "FileSequence/Audio",
                        static_cast<timeline::FileSequenceAudio>(value));
                });

            connect(
                p.audioFileName, &QLineEdit::textChanged,
                [this](const QString& value)
                {
                    _p->settings->setValue(
                        "FileSequence/AudioFileName",
                        std::string(value.toUtf8()));
                });

            connect(
                p.audioDirectory, &QLineEdit::textChanged,
                [this](const QString& value)
                {
                    _p->settings->setValue(
                        "FileSequence/AudioDirectory",
                        std::string(value.toUtf8()));
                });

            connect(
                p.maxDigitsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                { _p->settings->setValue("FileSequence/MaxDigits", value); });

            connect(
                p.threadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged), [this](int value)
                { _p->settings->setValue("SequenceIO/ThreadCount", value); });
        }

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget() {}

        void
        FileSequenceSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("FileSequence/Audio" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.audioComboBox);
                p.audioComboBox->setCurrentIndex(static_cast<int>(
                    p.settings->getValue<timeline::FileSequenceAudio>(
                        "FileSequence/Audio")));
            }
            if ("FileSequence/AudioFileName" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.audioFileName);
                p.audioFileName->setText(QString::fromUtf8(
                    p.settings
                        ->getValue<std::string>("FileSequence/AudioFileName")
                        .c_str()));
            }
            if ("FileSequence/AudioDirectory" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.audioDirectory);
                p.audioDirectory->setText(QString::fromUtf8(
                    p.settings
                        ->getValue<std::string>("FileSequence/AudioDirectory")
                        .c_str()));
            }
            if ("FileSequence/MaxDigits" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.maxDigitsSpinBox);
                p.maxDigitsSpinBox->setValue(
                    p.settings->getValue<int>("FileSequence/MaxDigits"));
            }
            if ("SequenceIO/ThreadCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.threadCountSpinBox);
                p.threadCountSpinBox->setValue(
                    p.settings->getValue<int>("SequenceIO/ThreadCount"));
            }
        }

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QCheckBox* yuvToRGBConversionCheckBox = nullptr;
            QSpinBox* threadCountSpinBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        FFmpegSettingsWidget::FFmpegSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.yuvToRGBConversionCheckBox = new QCheckBox;

            p.threadCountSpinBox = new QSpinBox;
            p.threadCountSpinBox->setRange(0, 64);

            auto layout = new QFormLayout;
            auto label = new QLabel(tr("Changes are applied to new files."));
            label->setWordWrap(true);
            layout->addRow(label);
            layout->addRow(
                tr("YUV to RGB conversion:"), p.yuvToRGBConversionCheckBox);
            layout->addRow(tr("I/O threads:"), p.threadCountSpinBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.yuvToRGBConversionCheckBox, &QCheckBox::toggled,
                [this](bool value) {
                    _p->settings->setValue("FFmpeg/YUVToRGBConversion", value);
                });

            connect(
                p.threadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged), [this](int value)
                { _p->settings->setValue("FFmpeg/ThreadCount", value); });
        }

        FFmpegSettingsWidget::~FFmpegSettingsWidget() {}

        void FFmpegSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("FFmpeg/YUVToRGBConversion" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.yuvToRGBConversionCheckBox);
                p.yuvToRGBConversionCheckBox->setChecked(
                    p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            }
            if ("FFmpeg/ThreadCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.threadCountSpinBox);
                p.threadCountSpinBox->setValue(
                    p.settings->getValue<int>("FFmpeg/ThreadCount"));
            }
        }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        struct USDSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QSpinBox* renderWidthSpinBox = nullptr;
            qtwidget::FloatEditSlider* complexitySlider = nullptr;
            QComboBox* drawModeComboBox = nullptr;
            QCheckBox* lightingCheckBox = nullptr;
            QCheckBox* sRGBCheckBox = nullptr;
            QSpinBox* stageCacheSpinBox = nullptr;
            QSpinBox* diskCacheSpinBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        USDSettingsWidget::USDSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.renderWidthSpinBox = new QSpinBox;
            p.renderWidthSpinBox->setRange(1, 8192);

            p.complexitySlider = new qtwidget::FloatEditSlider;

            p.drawModeComboBox = new QComboBox;
            for (const auto& i : usd::getDrawModeLabels())
            {
                p.drawModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.lightingCheckBox = new QCheckBox;

            p.sRGBCheckBox = new QCheckBox;

            p.stageCacheSpinBox = new QSpinBox;
            p.stageCacheSpinBox->setRange(0, 10);

            p.diskCacheSpinBox = new QSpinBox;
            p.diskCacheSpinBox->setRange(0, 1024);

            auto layout = new QFormLayout;
            layout->addRow(tr("Render width:"), p.renderWidthSpinBox);
            layout->addRow(tr("Render complexity:"), p.complexitySlider);
            layout->addRow(tr("Draw mode:"), p.drawModeComboBox);
            layout->addRow(tr("Enable lighting:"), p.lightingCheckBox);
            layout->addRow(tr("Enable sRGB color space:"), p.sRGBCheckBox);
            layout->addRow(tr("Stage cache size:"), p.stageCacheSpinBox);
            layout->addRow(tr("Disk cache size (GB):"), p.diskCacheSpinBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.renderWidthSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged), [this](int value)
                { _p->settings->setValue("USD/renderWidth", value); });

            connect(
                p.complexitySlider, &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                { _p->settings->setValue("USD/complexity", value); });

            connect(
                p.drawModeComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value) {
                    _p->settings->setValue(
                        "USD/drawMode", static_cast<usd::DrawMode>(value));
                });

            connect(
                p.lightingCheckBox, &QCheckBox::stateChanged,
                [this](int value) {
                    _p->settings->setValue(
                        "USD/enableLighting", Qt::Checked == value);
                });

            connect(
                p.sRGBCheckBox, &QCheckBox::stateChanged, [this](int value)
                { _p->settings->setValue("USD/sRGB", Qt::Checked == value); });

            connect(
                p.stageCacheSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged), [this](int value)
                { _p->settings->setValue("USD/stageCacheCount", value); });

            connect(
                p.diskCacheSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    _p->settings->setValue(
                        "USD/diskCacheByteCount", value * memory::gigabyte);
                });
        }

        USDSettingsWidget::~USDSettingsWidget() {}

        void USDSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("USD/renderWidth" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.renderWidthSpinBox);
                p.renderWidthSpinBox->setValue(
                    p.settings->getValue<int>("USD/renderWidth"));
            }
            if ("USD/complexity" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.complexitySlider);
                p.complexitySlider->setValue(
                    p.settings->getValue<float>("USD/complexity"));
            }
            if ("USD/drawMode" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.drawModeComboBox);
                p.drawModeComboBox->setCurrentIndex(static_cast<int>(
                    p.settings->getValue<usd::DrawMode>("USD/drawMode")));
            }
            if ("USD/enableLighting" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.lightingCheckBox);
                p.lightingCheckBox->setChecked(
                    p.settings->getValue<bool>("USD/enableLighting"));
            }
            if ("USD/sRGB" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.sRGBCheckBox);
                p.sRGBCheckBox->setChecked(
                    p.settings->getValue<bool>("USD/sRGB"));
            }
            if ("USD/stageCacheCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.stageCacheSpinBox);
                p.stageCacheSpinBox->setValue(
                    p.settings->getValue<size_t>("USD/stageCacheCount"));
            }
            if ("USD/diskCacheByteCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.diskCacheSpinBox);
                p.diskCacheSpinBox->setValue(
                    p.settings->getValue<size_t>("USD/diskCacheByteCount") /
                    memory::gigabyte);
            }
        }
#endif // TLRENDER_USD

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QCheckBox* nativeFileDialogCheckBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        FileBrowserSettingsWidget::FileBrowserSettingsWidget(
            App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.nativeFileDialogCheckBox = new QCheckBox;
            p.nativeFileDialogCheckBox->setText(tr("Native file dialog"));

            auto layout = new QFormLayout;
            layout->addRow(p.nativeFileDialogCheckBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.nativeFileDialogCheckBox, &QCheckBox::stateChanged,
                [this](int value)
                {
                    _p->settings->setValue(
                        "FileBrowser/NativeFileDialog", Qt::Checked == value);
                });
        }

        FileBrowserSettingsWidget::~FileBrowserSettingsWidget() {}

        void FileBrowserSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.nativeFileDialogCheckBox);
                p.nativeFileDialogCheckBox->setChecked(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
        }

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QComboBox* timerModeComboBox = nullptr;
            QSpinBox* audioBufferFrameCountSpinBox = nullptr;
            QSpinBox* videoRequestCountSpinBox = nullptr;
            QSpinBox* audioRequestCountSpinBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        PerformanceSettingsWidget::PerformanceSettingsWidget(
            App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.timerModeComboBox = new QComboBox;
            for (const auto& i : timeline::getTimerModeLabels())
            {
                p.timerModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioBufferFrameCountSpinBox = new QSpinBox;
            p.audioBufferFrameCountSpinBox->setRange(1024, 4096);

            p.videoRequestCountSpinBox = new QSpinBox;
            p.videoRequestCountSpinBox->setRange(1, 64);

            p.audioRequestCountSpinBox = new QSpinBox;
            p.audioRequestCountSpinBox->setRange(1, 64);

            auto layout = new QFormLayout;
            auto label = new QLabel(tr("Changes are applied to new files."));
            label->setWordWrap(true);
            layout->addRow(label);
            layout->addRow(tr("Timer mode:"), p.timerModeComboBox);
            layout->addRow(
                tr("Audio buffer frames:"), p.audioBufferFrameCountSpinBox);
            layout->addRow(tr("Video requests:"), p.videoRequestCountSpinBox);
            layout->addRow(tr("Audio requests:"), p.audioRequestCountSpinBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.timerModeComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->settings->setValue(
                        "Performance/TimerMode",
                        static_cast<timeline::TimerMode>(value));
                });

            connect(
                p.audioBufferFrameCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value) {
                    _p->settings->setValue(
                        "Performance/AudioBufferFrameCount", value);
                });

            connect(
                p.videoRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value) {
                    _p->settings->setValue(
                        "Performance/VideoRequestCount", value);
                });

            connect(
                p.audioRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value) {
                    _p->settings->setValue(
                        "Performance/AudioRequestCount", value);
                });
        }

        PerformanceSettingsWidget::~PerformanceSettingsWidget() {}

        void PerformanceSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("Performance/TimerMode" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.timerModeComboBox);
                p.timerModeComboBox->setCurrentIndex(
                    static_cast<int>(p.settings->getValue<timeline::TimerMode>(
                        "Performance/TimerMode")));
            }
            if ("Performance/AudioBufferFrameCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.audioBufferFrameCountSpinBox);
                p.audioBufferFrameCountSpinBox->setValue(
                    p.settings->getValue<int>(
                        "Performance/AudioBufferFrameCount"));
            }
            if ("Performance/VideoRequestCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.videoRequestCountSpinBox);
                p.videoRequestCountSpinBox->setValue(
                    p.settings->getValue<int>("Performance/VideoRequestCount"));
            }
            if ("Performance/AudioRequestCount" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.audioRequestCountSpinBox);
                p.audioRequestCountSpinBox->setValue(
                    p.settings->getValue<int>("Performance/AudioRequestCount"));
            }
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            QCheckBox* toolTipsCheckBox = nullptr;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
        };

        MiscSettingsWidget::MiscSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settings = app->settings();

            p.toolTipsCheckBox = new QCheckBox;
            p.toolTipsCheckBox->setText(tr("Enable tool tips"));

            auto layout = new QFormLayout;
            layout->addRow(p.toolTipsCheckBox);
            setLayout(layout);

            _settingsUpdate(std::string());

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            connect(
                p.toolTipsCheckBox, &QCheckBox::stateChanged,
                [this](int value) {
                    _p->settings->setValue(
                        "Misc/ToolTipsEnabled", Qt::Checked == value);
                });
        }

        MiscSettingsWidget::~MiscSettingsWidget() {}

        void MiscSettingsWidget::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            if ("Misc/ToolTipsEnabled" == name || name.empty())
            {
                QSignalBlocker signalBlocker(p.toolTipsCheckBox);
                p.toolTipsCheckBox->setChecked(
                    p.settings->getValue<bool>("Misc/ToolTipsEnabled"));
            }
        }

        SettingsTool::SettingsTool(App* app, QWidget* parent) :
            IToolWidget(app, parent)
        {
            addBellows(tr("Cache"), new CacheSettingsWidget(app));
            addBellows(
                tr("File Sequences"), new FileSequenceSettingsWidget(app));
#if defined(TLRENDER_FFMPEG)
            addBellows(tr("FFmpeg"), new FFmpegSettingsWidget(app));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            addBellows(tr("USD"), new USDSettingsWidget(app));
#endif // TLRENDER_USD
            addBellows(tr("File Browser"), new FileBrowserSettingsWidget(app));
            addBellows(tr("Performance"), new PerformanceSettingsWidget(app));
            addBellows(tr("Miscellaneous"), new MiscSettingsWidget(app));
            auto resetButton = new QToolButton;
            resetButton->setText(tr("Default Settings"));
            resetButton->setAutoRaise(true);
            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(1);
            layout->addWidget(resetButton);
            layout->addStretch();
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);
            addStretch();

            connect(
                resetButton, &QToolButton::clicked,
                [app]
                {
                    QMessageBox messageBox;
                    messageBox.setText("Reset preferences to default values?");
                    messageBox.setStandardButtons(
                        QMessageBox::Ok | QMessageBox::Cancel);
                    messageBox.setDefaultButton(QMessageBox::Ok);
                    if (messageBox.exec() == QMessageBox::Ok)
                    {
                        app->settings()->reset();
                    }
                });
        }

        SettingsDockWidget::SettingsDockWidget(
            SettingsTool* settingsTool, QWidget* parent)
        {
            setObjectName("SettingsTool");
            setWindowTitle(tr("Settings"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Settings"));
            dockTitleBar->setIcon(QIcon(":/Icons/Settings.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(settingsTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Settings.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F7));
            toggleViewAction()->setToolTip(tr("Show settings"));
        }
    } // namespace play_qt
} // namespace tl
