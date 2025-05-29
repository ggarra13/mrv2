// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlBaseApp/CmdLine.h>

#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/OCIOOptions.h>
#include <tlTimeline/Player.h>

#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

namespace tl
{
    namespace play
    {
        //! Application options.
        struct Options
        {
            std::string fileName;
            std::string audioFileName;
            std::string compareFileName;
            timeline::CompareOptions compareOptions;
            double speed = 0.0;
            timeline::Playback playback = timeline::Playback::Stop;
            timeline::Loop loop = timeline::Loop::Loop;
            otime::RationalTime seek = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;

#if defined(TLRENDER_USD)
            int usdRenderWidth = 1920;
            float usdComplexity = 1.F;
            usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
            bool usdEnableLighting = true;
            bool usdSRGB = true;
            size_t usdStageCache = 10;
            size_t usdDiskCache = 0;
#endif // TLRENDER_USD

            std::string logFileName;
            bool resetSettings = false;
            std::string settingsFileName;
        };

        //! Get the application command line arguments.
        std::vector<std::shared_ptr<app::ICmdLineArg> >
        getCmdLineArgs(Options&);

        //! Get the application command line options.
        std::vector<std::shared_ptr<app::ICmdLineOption> > getCmdLineOptions(
            Options&, const std::string& logFileName,
            const std::string& settingsFileName);
    } // namespace play
} // namespace tl
