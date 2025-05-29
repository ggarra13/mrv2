// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SecondaryWindow.h>
#include <tlPlayQtApp/Viewport.h>

#include <tlPlay/Settings.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/FileBrowserSystem.h>
#include <tlQtWidget/Style.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>
#include <tlQt/ToolTipsFilter.h>

#include <tlUI/RecentFilesModel.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/ViewportModel.h>
#include <tlPlay/Util.h>

#if defined(TLRENDER_BMD)
#    include <tlDevice/BMDDevicesModel.h>
#    include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlTimeline/Util.h>

#include <tlIO/System.h>
#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/AudioSystem.h>
#include <tlCore/FileLogSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <QPointer>
#include <QScreen>
#include <QTimer>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const size_t timeout = 5;
        }

        struct App::Private
        {
            play::Options options;
            QScopedPointer<qt::ContextObject> contextObject;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::string settingsFileName;
            std::shared_ptr<play::Settings> settings;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            QScopedPointer<qt::TimeObject> timeObject;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::vector<std::shared_ptr<timeline::Timeline> > timelines;
            QSharedPointer<qt::TimelinePlayer> player;
            std::shared_ptr<ui::RecentFilesModel> recentFilesModel;
            std::shared_ptr<play::ColorModel> colorModel;
            std::shared_ptr<play::RenderModel> renderModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::AudioModel> audioModel;
            QScopedPointer<qt::ToolTipsFilter> toolTipsFilter;

            QScopedPointer<MainWindow> mainWindow;
            QScopedPointer<SecondaryWindow> secondaryWindow;

            bool bmdDeviceActive = false;
#if defined(TLRENDER_BMD)
            std::shared_ptr<bmd::DevicesModel> bmdDevicesModel;
            std::shared_ptr<bmd::OutputDevice> bmdOutputDevice;
            image::VideoLevels bmdOutputVideoLevels = image::VideoLevels::First;
#endif // TLRENDER_BMD
            std::unique_ptr<QTimer> timer;

            std::shared_ptr<observer::ValueObserver<std::string> >
                settingsObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
                compareTimeObserver;
            std::shared_ptr<observer::ValueObserver<size_t> >
                recentFilesMaxObserver;
            std::shared_ptr<observer::ListObserver<file::Path> >
                recentFilesObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> >
                syncOffsetObserver;
#if defined(TLRENDER_BMD)
            std::shared_ptr<observer::ValueObserver<bmd::DevicesModelData> >
                bmdDevicesObserver;
            std::shared_ptr<observer::ValueObserver<bool> > bmdActiveObserver;
            std::shared_ptr<observer::ValueObserver<math::Size2i> >
                bmdSizeObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                bmdFrameRateObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> >
                ocioOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> >
                lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> >
                imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                displayOptionsObserver;
            std::shared_ptr<
                observer::ValueObserver<timeline::BackgroundOptions> >
                backgroundOptionsObserver;
#endif // TLRENDER_BMD
        };

        App::App(
            int& argc, char** argv,
            const std::shared_ptr<system::Context>& context) :
            QApplication(argc, argv),
            _p(new Private)
        {
            TLRENDER_P();
            const std::string appName = "tlplay-qt";
            const std::string appDocsPath = play::appDocsPath();
            const std::string logFileName =
                play::logFileName(appName, appDocsPath);
            const std::string settingsFileName =
                play::settingsName(appName, appDocsPath);
            BaseApp::_init(
                app::convert(argc, argv), context, appName,
                "Example Qt playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(
                    p.options, logFileName, settingsFileName));
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            setOrganizationName("tlRender");
            setApplicationName(QString::fromUtf8(appName.c_str()));
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());
            qtwidget::initFonts(context);

            _fileLogInit(logFileName);
            _settingsInit(settingsFileName);
            _modelsInit();
            _devicesInit();
            _observersInit();
            _inputFilesInit();
            _windowsInit();

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(p.timer.get(), &QTimer::timeout, this, &App::_timerUpdate);
            p.timer->start(timeout);
        }

        App::~App() {}

        const std::shared_ptr<timeline::TimeUnitsModel>&
        App::timeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject.get();
        }

        const std::shared_ptr<play::Settings>& App::settings() const
        {
            return _p->settings;
        }

        const std::shared_ptr<play::FilesModel>& App::filesModel() const
        {
            return _p->filesModel;
        }

        const QSharedPointer<qt::TimelinePlayer>& App::player() const
        {
            return _p->player;
        }

        const std::shared_ptr<ui::RecentFilesModel>&
        App::recentFilesModel() const
        {
            return _p->recentFilesModel;
        }

        const std::shared_ptr<play::ColorModel>& App::colorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::RenderModel>& App::renderModel() const
        {
            return _p->renderModel;
        }

        const std::shared_ptr<play::ViewportModel>& App::viewportModel() const
        {
            return _p->viewportModel;
        }

        const std::shared_ptr<play::AudioModel>& App::audioModel() const
        {
            return _p->audioModel;
        }

        MainWindow* App::mainWindow() const
        {
            return _p->mainWindow.get();
        }

#if defined(TLRENDER_BMD)
        const std::shared_ptr<bmd::DevicesModel>& App::bmdDevicesModel() const
        {
            return _p->bmdDevicesModel;
        }

        const std::shared_ptr<bmd::OutputDevice>& App::bmdOutputDevice() const
        {
            return _p->bmdOutputDevice;
        }
#endif // TLRENDER_BMD

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits =
                p.settings->getValue<size_t>("FileSequence/MaxDigits");
            for (const auto& path : timeline::getPaths(
                     file::Path(fileName.toUtf8().data()), pathOptions,
                     _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName.toUtf8().data());
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(path);
            }
        }

        void App::openDialog()
        {
            TLRENDER_P();
            if (auto fileBrowserSystem =
                    _context->getSystem<qtwidget::FileBrowserSystem>())
            {
                fileBrowserSystem->open(
                    p.mainWindow.get(),
                    [this](const file::Path& value)
                    {
                        if (!value.isEmpty())
                        {
                            open(QString::fromUtf8(value.get().c_str()));
                        }
                    });
            }
        }

        void App::openSeparateAudioDialog()
        {
            QScopedPointer<OpenSeparateAudioDialog> dialog(
                new OpenSeparateAudioDialog(_context));
            if (QDialog::Accepted == dialog->exec())
            {
                open(dialog->videoFileName(), dialog->audioFileName());
            }
        }

        void App::setSecondaryWindow(bool value)
        {
            TLRENDER_P();
            //! \bug macOS does not seem to like having an application with
            //! normal and fullscreen windows.
            QScreen* secondaryScreen = nullptr;
#if !defined(__APPLE__)
            auto screens = this->screens();
            auto mainWindowScreen = p.mainWindow->screen();
            screens.removeOne(mainWindowScreen);
            if (!screens.isEmpty())
            {
                secondaryScreen = screens[0];
            }
#endif // __APPLE__
            if (value)
            {
                p.secondaryWindow.reset(new SecondaryWindow(this));
                if (secondaryScreen)
                {
                    p.secondaryWindow->move(
                        secondaryScreen->availableGeometry().topLeft());
                    p.secondaryWindow->setWindowState(
                        p.secondaryWindow->windowState() ^
                        Qt::WindowFullScreen);
                }

                connect(
                    p.secondaryWindow.get(), &QObject::destroyed,
                    [this](QObject*)
                    {
                        _p->secondaryWindow.take();
                        Q_EMIT secondaryWindowChanged(false);
                    });

                p.secondaryWindow->show();
            }
            else if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
            Q_EMIT secondaryWindowChanged(value);
        }

        void App::_fileLogInit(const std::string& logFileName)
        {
            TLRENDER_P();
            std::string logFileName2 = logFileName;
            if (!p.options.logFileName.empty())
            {
                logFileName2 = p.options.logFileName;
            }
            p.fileLogSystem =
                file::FileLogSystem::create(logFileName2, _context);
        }

        void App::_settingsInit(const std::string& settingsFileName)
        {
            TLRENDER_P();
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                p.settingsFileName = settingsFileName;
            }
            p.settings = play::Settings::create(
                p.settingsFileName, p.options.resetSettings, _context);

            p.settings->setDefaultValue("Files/RecentMax", 10);

            p.settings->setDefaultValue("Cache/Size", 1);
            p.settings->setDefaultValue("Cache/ReadAhead", 2.0);
            p.settings->setDefaultValue("Cache/ReadBehind", 0.5);

            p.settings->setDefaultValue(
                "FileSequence/Audio", timeline::FileSequenceAudio::BaseName);
            p.settings->setDefaultValue(
                "FileSequence/AudioFileName", std::string());
            p.settings->setDefaultValue(
                "FileSequence/AudioDirectory", std::string());
            p.settings->setDefaultValue("FileSequence/MaxDigits", 9);

            p.settings->setDefaultValue("SequenceIO/ThreadCount", 16);

#if defined(TLRENDER_FFMPEG)
            p.settings->setDefaultValue("FFmpeg/YUVToRGBConversion", false);
            p.settings->setDefaultValue("FFmpeg/ThreadCount", 0);
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            p.settings->setDefaultValue(
                "USD/renderWidth", p.options.usdRenderWidth);
            p.settings->setDefaultValue(
                "USD/complexity", p.options.usdComplexity);
            p.settings->setDefaultValue("USD/drawMode", p.options.usdDrawMode);
            p.settings->setDefaultValue(
                "USD/enableLighting", p.options.usdEnableLighting);
            p.settings->setDefaultValue("USD/sRGB", p.options.usdSRGB);
            p.settings->setDefaultValue(
                "USD/stageCacheCount", p.options.usdStageCache);
            p.settings->setDefaultValue(
                "USD/diskCacheByteCount", p.options.usdDiskCache);
#endif // TLRENDER_USD

#if defined(TLRENDER_BMD)
            bmd::DevicesModelData bmdDevicesModelData;
            p.settings->setDefaultValue(
                "BMD/DeviceIndex", bmdDevicesModelData.deviceIndex);
            p.settings->setDefaultValue(
                "BMD/DisplayModeIndex", bmdDevicesModelData.displayModeIndex);
            p.settings->setDefaultValue(
                "BMD/PixelTypeIndex", bmdDevicesModelData.pixelTypeIndex);
            p.settings->setDefaultValue(
                "BMD/DeviceEnabled", bmdDevicesModelData.deviceEnabled);
            const auto i = bmdDevicesModelData.boolOptions.find(
                bmd::Option::_444SDIVideoOutput);
            p.settings->setDefaultValue(
                "BMD/444SDIVideoOutput",
                i != bmdDevicesModelData.boolOptions.end() ? i->second : false);
            p.settings->setDefaultValue(
                "BMD/HDRMode", bmdDevicesModelData.hdrMode);
            p.settings->setDefaultValue(
                "BMD/HDRData", bmdDevicesModelData.hdrData);
#endif // TLRENDER_BMD

            p.settings->setDefaultValue("FileBrowser/NativeFileDialog", true);

            p.settings->setDefaultValue(
                "Performance/TimerMode", timeline::PlayerOptions().timerMode);
            p.settings->setDefaultValue(
                "Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settings->setDefaultValue("Performance/AudioRequestCount", 16);

            p.settings->setDefaultValue("Misc/ToolTipsEnabled", true);
        }

        void App::_modelsInit()
        {
            TLRENDER_P();

            p.contextObject.reset(new qt::ContextObject(_context));

            p.timeUnitsModel = timeline::TimeUnitsModel::create(_context);

            p.timeObject.reset(new qt::TimeObject(p.timeUnitsModel));

            p.filesModel = play::FilesModel::create(_context);

            p.recentFilesModel = ui::RecentFilesModel::create(_context);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setOCIOOptions(p.options.ocioOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            p.viewportModel = play::ViewportModel::create(p.settings, _context);

            p.renderModel = play::RenderModel::create(p.settings, _context);

            p.audioModel = play::AudioModel::create(p.settings, _context);
        }

        void App::_devicesInit()
        {
            TLRENDER_P();
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice = bmd::OutputDevice::create(_context);
            if (0)
            {
                auto overlayImage = image::Image::create(
                    1920, 1080, image::PixelType::ARGB_4444_Premult);
                QImage* bmdOverlayImage = new QImage(
                    overlayImage->getData(), 1920, 1080,
                    QImage::Format_ARGB4444_Premultiplied);
                bmdOverlayImage->fill(QColor(0, 0, 255, 63));
                p.bmdOutputDevice->setOverlay(overlayImage);
            }
            p.bmdDevicesModel = bmd::DevicesModel::create(_context);
            p.bmdDevicesModel->setDeviceIndex(
                p.settings->getValue<int>("BMD/DeviceIndex"));
            p.bmdDevicesModel->setDisplayModeIndex(
                p.settings->getValue<int>("BMD/DisplayModeIndex"));
            p.bmdDevicesModel->setPixelTypeIndex(
                p.settings->getValue<int>("BMD/PixelTypeIndex"));
            p.bmdDevicesModel->setDeviceEnabled(
                p.settings->getValue<bool>("BMD/DeviceEnabled"));
            bmd::BoolOptions deviceBoolOptions;
            deviceBoolOptions[bmd::Option::_444SDIVideoOutput] =
                p.settings->getValue<bool>("BMD/444SDIVideoOutput");
            p.bmdDevicesModel->setBoolOptions(deviceBoolOptions);
            p.bmdDevicesModel->setHDRMode(static_cast<bmd::HDRMode>(
                p.settings->getValue<int>("BMD/HDRMode")));
            std::string s = p.settings->getValue<std::string>("BMD/HDRData");
            if (!s.empty())
            {
                auto json = nlohmann::json::parse(s);
                image::HDRData hdrData;
                try
                {
                    from_json(json, hdrData);
                }
                catch (const std::exception&)
                {
                }
                p.bmdDevicesModel->setHDRData(hdrData);
            }
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            TLRENDER_P();

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name) { _settingsUpdate(name); });

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    p.filesModel->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _filesUpdate(value); });
            p.activeObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    p.filesModel->observeActive(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _activeUpdate(value); });
            p.layersObserver = observer::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                { _layersUpdate(value); });
            p.compareTimeObserver =
                observer::ValueObserver<timeline::CompareTimeMode>::create(
                    p.filesModel->observeCompareTime(),
                    [this](timeline::CompareTimeMode value)
                    {
                        if (_p->player)
                        {
                            _p->player->setCompareTime(value);
                        }
                    });

            p.recentFilesMaxObserver = observer::ValueObserver<size_t>::create(
                p.recentFilesModel->observeRecentMax(), [this](size_t value)
                { _p->settings->setValue("Files/RecentMax", value); });
            p.recentFilesObserver = observer::ListObserver<file::Path>::create(
                p.recentFilesModel->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    std::vector<std::string> fileNames;
                    for (const auto& i : value)
                    {
                        fileNames.push_back(i.get());
                    }
                    _p->settings->setValue("Files/Recent", fileNames);
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                p.audioModel->observeVolume(),
                [this](float) { _audioUpdate(); });
            p.muteObserver = observer::ValueObserver<bool>::create(
                p.audioModel->observeMute(), [this](bool) { _audioUpdate(); });
            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                p.audioModel->observeSyncOffset(),
                [this](double) { _audioUpdate(); });

#if defined(TLRENDER_BMD)
            p.bmdDevicesObserver =
                observer::ValueObserver<bmd::DevicesModelData>::create(
                    p.bmdDevicesModel->observeData(),
                    [this](const bmd::DevicesModelData& value)
                    {
                        TLRENDER_P();

                        bmd::DeviceConfig config;
                        config.deviceIndex = value.deviceIndex - 1;
                        config.displayModeIndex = value.displayModeIndex - 1;
                        config.pixelType =
                            value.pixelTypeIndex >= 0 &&
                                    value.pixelTypeIndex <
                                        value.pixelTypes.size()
                                ? value.pixelTypes[value.pixelTypeIndex]
                                : bmd::PixelType::kNone;
                        config.boolOptions = value.boolOptions;
                        p.bmdOutputDevice->setConfig(config);
                        p.bmdOutputDevice->setEnabled(value.deviceEnabled);
                        p.bmdOutputVideoLevels = value.videoLevels;
                        timeline::DisplayOptions displayOptions =
                            p.viewportModel->getDisplayOptions();
                        displayOptions.videoLevels = p.bmdOutputVideoLevels;
                        p.bmdOutputDevice->setDisplayOptions({displayOptions});
                        p.bmdOutputDevice->setHDR(value.hdrMode, value.hdrData);

                        p.settings->setValue(
                            "BMD/DeviceIndex", value.deviceIndex);
                        p.settings->setValue(
                            "BMD/DisplayModeIndex", value.displayModeIndex);
                        p.settings->setValue(
                            "BMD/PixelTypeIndex", value.pixelTypeIndex);
                        p.settings->setValue(
                            "BMD/DeviceEnabled", value.deviceEnabled);
                        const auto i = value.boolOptions.find(
                            bmd::Option::_444SDIVideoOutput);
                        p.settings->setValue(
                            "BMD/444SDIVideoOutput",
                            i != value.boolOptions.end() ? i->second : false);
                        p.settings->setValue("BMD/HDRMode", value.hdrMode);
                        p.settings->setValue("BMD/HDRData", value.hdrData);
                    });

            p.bmdActiveObserver = observer::ValueObserver<bool>::create(
                p.bmdOutputDevice->observeActive(),
                [this](bool value)
                {
                    _p->bmdDeviceActive = value;
                    _audioUpdate();
                });
            p.bmdSizeObserver = observer::ValueObserver<math::Size2i>::create(
                p.bmdOutputDevice->observeSize(),
                [this](const math::Size2i& value)
                {
                    // std::cout << "output device size: " << value <<
                    // std::endl;
                });
            p.bmdFrameRateObserver =
                observer::ValueObserver<otime::RationalTime>::create(
                    p.bmdOutputDevice->observeFrameRate(),
                    [this](const otime::RationalTime& value)
                    {
                        // std::cout << "output device frame rate: " << value <<
                        // std::endl;
                    });

            p.compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    p.filesModel->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { _p->bmdOutputDevice->setCompareOptions(value); });
            p.ocioOptionsObserver =
                observer::ValueObserver<timeline::OCIOOptions>::create(
                    p.colorModel->observeOCIOOptions(),
                    [this](const timeline::OCIOOptions& value)
                    { _p->bmdOutputDevice->setOCIOOptions(value); });
            p.lutOptionsObserver =
                observer::ValueObserver<timeline::LUTOptions>::create(
                    p.colorModel->observeLUTOptions(),
                    [this](const timeline::LUTOptions& value)
                    { _p->bmdOutputDevice->setLUTOptions(value); });
            p.imageOptionsObserver =
                observer::ValueObserver<timeline::ImageOptions>::create(
                    p.renderModel->observeImageOptions(),
                    [this](const timeline::ImageOptions& value)
                    { _p->bmdOutputDevice->setImageOptions({value}); });
            p.displayOptionsObserver =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    p.viewportModel->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        timeline::DisplayOptions tmp = value;
                        tmp.videoLevels = _p->bmdOutputVideoLevels;
                        _p->bmdOutputDevice->setDisplayOptions({tmp});
                    });
            p.backgroundOptionsObserver =
                observer::ValueObserver<timeline::BackgroundOptions>::create(
                    p.viewportModel->observeBackgroundOptions(),
                    [this](const timeline::BackgroundOptions& value)
                    { _p->bmdOutputDevice->setBackgroundOptions(value); });
#endif // TLRENDER_BMD
        }

        void App::_inputFilesInit()
        {
            TLRENDER_P();
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    open(QString::fromUtf8(p.options.compareFileName.c_str()));
                    p.filesModel->setCompareOptions(p.options.compareOptions);
                    p.filesModel->setB(0, true);
                }

                open(
                    QString::fromUtf8(p.options.fileName.c_str()),
                    QString::fromUtf8(p.options.audioFileName.c_str()));

                if (p.player)
                {
                    if (p.options.speed > 0.0)
                    {
                        p.player->setSpeed(p.options.speed);
                    }
                    if (time::isValid(p.options.inOutRange))
                    {
                        p.player->setInOutRange(p.options.inOutRange);
                        p.player->seek(p.options.inOutRange.start_time());
                    }
                    if (time::isValid(p.options.seek))
                    {
                        p.player->seek(p.options.seek);
                    }
                    p.player->setLoop(p.options.loop);
                    p.player->setPlayback(p.options.playback);
                }
            }
        }

        void App::_windowsInit()
        {
            TLRENDER_P();

            p.mainWindow.reset(new MainWindow(this));
            const math::Size2i windowSize =
                p.settings->getValue<math::Size2i>("MainWindow/Size");
            p.mainWindow->resize(windowSize.w, windowSize.h);
            p.mainWindow->show();

            connect(
                p.mainWindow.get(), &QObject::destroyed,
                [this](QObject*)
                {
                    _p->mainWindow.take();
                    if (_p->secondaryWindow)
                    {
                        _p->secondaryWindow->close();
                    }
                });

            connect(
                p.mainWindow->viewport(), &Viewport::viewPosAndZoomChanged,
                [this](const math::Vector2i& pos, double zoom) {
                    _viewUpdate(
                        pos, zoom, _p->mainWindow->viewport()->hasFrameView());
                });
            connect(
                p.mainWindow->viewport(), &Viewport::frameViewChanged,
                [this](bool value)
                {
                    _viewUpdate(
                        _p->mainWindow->viewport()->viewPos(),
                        _p->mainWindow->viewport()->viewZoom(), value);
                });
        }

        io::Options App::_ioOptions() const
        {
            TLRENDER_P();
            io::Options out;

            out["SequenceIO/ThreadCount"] = string::Format("{0}").arg(
                p.settings->getValue<int>("SequenceIO/ThreadCount"));

#if defined(TLRENDER_FFMPEG)
            out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").arg(
                p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            out["FFmpeg/ThreadCount"] = string::Format("{0}").arg(
                p.settings->getValue<int>("FFmpeg/ThreadCount"));
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << p.settings->getValue<int>("USD/renderWidth");
                out["USD/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<float>("USD/complexity");
                out["USD/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<usd::DrawMode>("USD/drawMode");
                out["USD/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/enableLighting");
                out["USD/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/sRGB");
                out["USD/sRGB"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/stageCacheCount");
                out["USD/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/diskCacheByteCount");
                out["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
        }

        void App::_timerUpdate()
        {
#if defined(TLRENDER_BMD)
            if (_p && _p->bmdOutputDevice)
            {
                _p->bmdOutputDevice->tick();
            }
#endif // TLRENDER_BMD
        }

        void App::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            const auto split = string::split(name, '/');
            if (!split.empty() || name.empty())
            {
                auto ioSystem = _context->getSystem<io::System>();
                const auto& names = ioSystem->getNames();
                bool match = false;
                if (!split.empty())
                {
                    match = std::find(names.begin(), names.end(), split[0]) !=
                            names.end();
                }
                if (match || name.empty())
                {
                    const auto ioOptions = _ioOptions();
                    if (p.player)
                    {
                        p.player->setIOOptions(ioOptions);
                    }
                }
            }
            if ("Cache/Size" == name || "Cache/ReadAhead" == name ||
                "Cache/ReadBehind" == name || name.empty())
            {
                _cacheUpdate();
            }
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                auto fileBrowserSystem =
                    _context->getSystem<qtwidget::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
            if ("Files/RecentMax" == name || name.empty())
            {
                p.recentFilesModel->setRecentMax(
                    p.settings->getValue<int>("Files/RecentMax"));
            }
            if ("Files/Recent" == name || name.empty())
            {
                std::vector<file::Path> recentPaths;
                for (const auto& recentFile :
                     p.settings->getValue<std::vector<std::string> >(
                         "Files/Recent"))
                {
                    recentPaths.push_back(file::Path(recentFile));
                }
                p.recentFilesModel->setRecent(recentPaths);
            }
            if ("Misc/ToolTipsEnabled" == name || name.empty())
            {
                if (p.settings->getValue<bool>("Misc/ToolTipsEnabled"))
                {
                    removeEventFilter(p.toolTipsFilter.get());
                    p.toolTipsFilter.reset();
                }
                else
                {
                    p.toolTipsFilter.reset(new qt::ToolTipsFilter(this));
                    installEventFilter(p.toolTipsFilter.get());
                }
            }
        }

        void App::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& files)
        {
            TLRENDER_P();

            std::vector<std::shared_ptr<timeline::Timeline> > timelines(
                files.size());
            for (size_t i = 0; i < files.size(); ++i)
            {
                const auto j =
                    std::find(p.files.begin(), p.files.end(), files[i]);
                if (j != p.files.end())
                {
                    timelines[i] = p.timelines[j - p.files.begin()];
                }
            }

            for (size_t i = 0; i < files.size(); ++i)
            {
                if (!timelines[i])
                {
                    try
                    {
                        timeline::Options options;
                        options.fileSequenceAudio =
                            p.settings->getValue<timeline::FileSequenceAudio>(
                                "FileSequence/Audio");
                        options.fileSequenceAudioFileName =
                            p.settings->getValue<std::string>(
                                "FileSequence/AudioFileName");
                        options.fileSequenceAudioDirectory =
                            p.settings->getValue<std::string>(
                                "FileSequence/AudioDirectory");
                        options.videoRequestCount =
                            p.settings->getValue<size_t>(
                                "Performance/VideoRequestCount");
                        options.audioRequestCount =
                            p.settings->getValue<size_t>(
                                "Performance/AudioRequestCount");
                        options.ioOptions = _ioOptions();
                        options.pathOptions.maxNumberDigits =
                            p.settings->getValue<size_t>(
                                "FileSequence/MaxDigits");
                        auto otioTimeline =
                            files[i]->audioPath.isEmpty()
                                ? timeline::create(
                                      files[i]->path, _context, options)
                                : timeline::create(
                                      files[i]->path, files[i]->audioPath,
                                      _context, options);
                        if (0)
                        {
                            timeline::toMemoryReferences(
                                otioTimeline, files[i]->path.getDirectory(),
                                timeline::ToMemoryReference::Shared,
                                options.pathOptions);
                        }
                        timelines[i] = timeline::Timeline::create(
                            otioTimeline, _context, options);
                        for (const auto& video :
                             timelines[i]->getIOInfo().video)
                        {
                            files[i]->videoLayers.push_back(video.name);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                }
            }

            p.files = files;
            p.timelines = timelines;
        }

        void App::_activeUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >&
                activeFiles)
        {
            TLRENDER_P();

            QSharedPointer<qt::TimelinePlayer> player;
            if (!activeFiles.empty())
            {
                if (!p.activeFiles.empty() &&
                    activeFiles[0] == p.activeFiles[0])
                {
                    player = p.player;
                }
                else
                {
                    auto i = std::find(
                        p.files.begin(), p.files.end(), activeFiles[0]);
                    if (i != p.files.end())
                    {
                        if (auto timeline = p.timelines[i - p.files.begin()])
                        {
                            try
                            {
                                timeline::PlayerOptions playerOptions;
                                playerOptions.cache.readAhead =
                                    time::invalidTime;
                                playerOptions.cache.readBehind =
                                    time::invalidTime;
                                playerOptions.timerMode =
                                    p.settings->getValue<timeline::TimerMode>(
                                        "Performance/TimerMode");
                                playerOptions.audioBufferFrameCount =
                                    p.settings->getValue<size_t>(
                                        "Performance/AudioBufferFrameCount");
                                player.reset(new qt::TimelinePlayer(
                                    timeline::Player::create(
                                        timeline, _context, playerOptions),
                                    _context, this));
                            }
                            catch (const std::exception& e)
                            {
                                _log(e.what(), log::Type::Error);
                            }
                        }
                    }
                }
            }
            if (player)
            {
                std::vector<std::shared_ptr<timeline::Timeline> > compare;
                for (size_t i = 1; i < activeFiles.size(); ++i)
                {
                    auto j = std::find(
                        p.files.begin(), p.files.end(), activeFiles[i]);
                    if (j != p.files.end())
                    {
                        auto timeline = p.timelines[j - p.files.begin()];
                        compare.push_back(timeline);
                    }
                }
                player->setCompare(compare);
                player->setCompareTime(p.filesModel->getCompareTime());
            }

            p.activeFiles = activeFiles;
            p.player = player;
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setPlayer(
                p.player ? p.player->player() : nullptr);
#endif // TLRENDER_BMD

            _layersUpdate(p.filesModel->observeLayers()->get());
            _cacheUpdate();
            _audioUpdate();

            Q_EMIT playerChanged(p.player);
        }

        void App::_layersUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (auto player = p.player)
            {
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                if (!value.empty() && value.size() == p.files.size() &&
                    !p.activeFiles.empty())
                {
                    auto i = std::find(
                        p.files.begin(), p.files.end(), p.activeFiles.front());
                    if (i != p.files.end())
                    {
                        videoLayer = value[i - p.files.begin()];
                    }
                    for (size_t j = 1; j < p.activeFiles.size(); ++j)
                    {
                        i = std::find(
                            p.files.begin(), p.files.end(), p.activeFiles[j]);
                        if (i != p.files.end())
                        {
                            compareVideoLayers.push_back(
                                value[i - p.files.begin()]);
                        }
                    }
                }
                player->setVideoLayer(videoLayer);
                player->setCompareVideoLayers(compareVideoLayers);
            }
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(
                p.settings->getValue<size_t>("Cache/Size") * memory::gigabyte);

            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = otime::RationalTime(
                p.settings->getValue<double>("Cache/ReadAhead"), 1.0);
            cacheOptions.readBehind = otime::RationalTime(
                p.settings->getValue<double>("Cache/ReadBehind"), 1.0);
            if (p.player)
            {
                p.player->setCacheOptions(cacheOptions);
            }
        }

        void
        App::_viewUpdate(const math::Vector2i& pos, double zoom, bool frame)
        {
            TLRENDER_P();
            float scale = 1.F;
            const QSize& size = p.mainWindow->viewport()->size() *
                                p.mainWindow->devicePixelRatio();
            if (p.secondaryWindow)
            {
                const QSize& secondarySize =
                    p.secondaryWindow->size() *
                    p.secondaryWindow->devicePixelRatio();
                if (size.isValid() && secondarySize.isValid())
                {
                    scale = secondarySize.width() /
                            static_cast<float>(size.width());
                }
                p.secondaryWindow->setView(pos * scale, zoom * scale, frame);
            }
#if defined(TLRENDER_BMD)
            scale = 1.F;
            const math::Size2i& bmdSize = p.bmdOutputDevice->getSize();
            if (size.isValid() && bmdSize.isValid())
            {
                scale = bmdSize.w / static_cast<float>(size.width());
            }
            p.bmdOutputDevice->setView(pos * scale, zoom * scale, frame);
#endif // TLRENDER_BMD
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            const float volume = p.audioModel->getVolume();
            const bool mute = p.audioModel->isMuted();
            const double audioOffset = p.audioModel->getSyncOffset();
            if (p.player)
            {
                p.player->setVolume(volume);
                p.player->setMute(mute || p.bmdDeviceActive);
                p.player->setAudioOffset(audioOffset);
            }
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setVolume(volume);
            p.bmdOutputDevice->setMute(mute);
            p.bmdOutputDevice->setAudioOffset(audioOffset);
#endif // TLRENDER_BMD
        }
    } // namespace play_qt
} // namespace tl
