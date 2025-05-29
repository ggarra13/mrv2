// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/App.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        std::vector<std::shared_ptr<app::ICmdLineArg> >
        getCmdLineArgs(Options& options)
        {
            return {app::CmdLineValueArg<std::string>::create(
                options.fileName, "input",
                "Timeline, movie, image sequence, or folder.", true)};
        }

        std::vector<std::shared_ptr<app::ICmdLineOption> > getCmdLineOptions(
            Options& options, const std::string& logFileName,
            const std::string& settingsFileName)
        {
            return {
                app::CmdLineValueOption<std::string>::create(
                    options.audioFileName, {"-audio", "-a"},
                    "Audio file name."),
                app::CmdLineValueOption<std::string>::create(
                    options.compareFileName, {"-b"},
                    "A/B comparison \"B\" file name."),
                app::CmdLineValueOption<timeline::CompareMode>::create(
                    options.compareOptions.mode, {"-compare", "-c"},
                    "A/B comparison mode.",
                    string::Format("{0}").arg(options.compareOptions.mode),
                    string::join(timeline::getCompareModeLabels(), ", ")),
                app::CmdLineValueOption<math::Vector2f>::create(
                    options.compareOptions.wipeCenter, {"-wipeCenter", "-wc"},
                    "A/B comparison wipe center.",
                    string::Format("{0}").arg(
                        options.compareOptions.wipeCenter)),
                app::CmdLineValueOption<float>::create(
                    options.compareOptions.wipeRotation,
                    {"-wipeRotation", "-wr"}, "A/B comparison wipe rotation.",
                    string::Format("{0}").arg(
                        options.compareOptions.wipeRotation)),
                app::CmdLineValueOption<double>::create(
                    options.speed, {"-speed"}, "Playback speed."),
                app::CmdLineValueOption<timeline::Playback>::create(
                    options.playback, {"-playback", "-p"}, "Playback mode.",
                    string::Format("{0}").arg(options.playback),
                    string::join(timeline::getPlaybackLabels(), ", ")),
                app::CmdLineValueOption<timeline::Loop>::create(
                    options.loop, {"-loop", "-lp"}, "Playback loop mode.",
                    string::Format("{0}").arg(options.loop),
                    string::join(timeline::getLoopLabels(), ", ")),
                app::CmdLineValueOption<otime::RationalTime>::create(
                    options.seek, {"-seek"}, "Seek to the given time."),
                app::CmdLineValueOption<otime::TimeRange>::create(
                    options.inOutRange, {"-inOutRange"},
                    "Set the in/out points range."),
                app::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.fileName, {"-ocio"},
                    "OpenColorIO configuration file name (e.g., config.ocio)."),
                app::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.input, {"-ocioInput"},
                    "OpenColorIO input name."),
                app::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.display, {"-ocioDisplay"},
                    "OpenColorIO display name."),
                app::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.view, {"-ocioView"},
                    "OpenColorIO view name."),
                app::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.look, {"-ocioLook"},
                    "OpenColorIO look name."),
                app::CmdLineValueOption<std::string>::create(
                    options.lutOptions.fileName, {"-lut"}, "LUT file name."),
                app::CmdLineValueOption<timeline::LUTOrder>::create(
                    options.lutOptions.order, {"-lutOrder"},
                    "LUT operation order.",
                    string::Format("{0}").arg(options.lutOptions.order),
                    string::join(timeline::getLUTOrderLabels(), ", ")),
#if defined(TLRENDER_USD)
                app::CmdLineValueOption<int>::create(
                    options.usdRenderWidth, {"-usdRenderWidth"},
                    "USD render width.",
                    string::Format("{0}").arg(options.usdRenderWidth)),
                app::CmdLineValueOption<float>::create(
                    options.usdComplexity, {"-usdComplexity"},
                    "USD render complexity setting.",
                    string::Format("{0}").arg(options.usdComplexity)),
                app::CmdLineValueOption<usd::DrawMode>::create(
                    options.usdDrawMode, {"-usdDrawMode"}, "USD draw mode.",
                    string::Format("{0}").arg(options.usdDrawMode),
                    string::join(usd::getDrawModeLabels(), ", ")),
                app::CmdLineValueOption<bool>::create(
                    options.usdEnableLighting, {"-usdEnableLighting"},
                    "USD enable lighting.",
                    string::Format("{0}").arg(options.usdEnableLighting)),
                app::CmdLineValueOption<bool>::create(
                    options.usdSRGB, {"-usdSRGB"},
                    "USD enable sRGB color space.",
                    string::Format("{0}").arg(options.usdSRGB)),
                app::CmdLineValueOption<size_t>::create(
                    options.usdStageCache, {"-usdStageCache"},
                    "USD stage cache size.",
                    string::Format("{0}").arg(options.usdStageCache)),
                app::CmdLineValueOption<size_t>::create(
                    options.usdDiskCache, {"-usdDiskCache"},
                    "USD disk cache size in gigabytes. A size of zero disables "
                    "the disk cache.",
                    string::Format("{0}").arg(options.usdDiskCache)),
#endif // TLRENDER_USD
                app::CmdLineValueOption<std::string>::create(
                    options.logFileName, {"-logFile"}, "Log file name.",
                    string::Format("{0}").arg(logFileName)),
                app::CmdLineFlagOption::create(
                    options.resetSettings, {"-resetSettings"},
                    "Reset settings to defaults."),
                app::CmdLineValueOption<std::string>::create(
                    options.settingsFileName, {"-settings"},
                    "Settings file name.",
                    string::Format("{0}").arg(settingsFileName)),
            };
        }
    } // namespace play
} // namespace tl
