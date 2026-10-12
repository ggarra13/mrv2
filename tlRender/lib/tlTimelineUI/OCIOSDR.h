// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "Namespace.h"

#include <tlTimeline/OCIOOptions.h> // adjust to wherever OCIOOptions lives

#include <OpenColorIO/OpenColorIO.h>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace tl
{
    namespace TIMELINEUI
    {
        namespace OCIO = OCIO_NAMESPACE;

        //! Get whether a display / view pair is an HDR one
        //! (ST2084 / PQ / HLG / Rec.2100 displays, "HDR" views).
        bool isHDRDisplayView(
            const std::string& display, const std::string& view);

        //! Convert an HDR display / view pair to the closest SDR one that
        //! actually exists in the given OCIO config.
        //!
        //! For example:
        //!   ST2084-P3-D65 / ACES 2.0 - HDR 1000 nits (P3 D65)
        //! becomes:
        //!   sRGB - Display / ACES 2.0 - SDR 100 nits (Rec.709)
        //!
        //! SDR pairs are returned unchanged.  If no SDR equivalent can be
        //! found in the config, the config's default SDR display / view is
        //! returned.
        std::pair<std::string, std::string> toSDRDisplayView(
            const OCIO::ConstConfigRcPtr&, const std::string& display,
            const std::string& view);

        //! Convert OCIO options with an HDR display / view to SDR options.
        //! The config is loaded from options.fileName (or $OCIO if empty) and
        //! results are cached, so this is cheap to call every frame.
        timeline::OCIOOptions toSDR(const timeline::OCIOOptions&);

        //! CPU OCIO processor for 8-bit RGBA images.  It is meant for widgets
        //! that use neither OpenGL nor Vulkan (like FLTK image thumbnails),
        //! and for caching OCIO'd thumbnails so they are not re-processed on
        //! every redraw.
        class OCIOCPUProcessor
        {
        public:
            //! Set the options.  The processor is only rebuilt when they
            //! change.  If forceSDR is true, an HDR display / view is
            //! converted to SDR (see toSDR()).
            void setOptions(
                const timeline::OCIOOptions&, bool forceSDR = true);

            const timeline::OCIOOptions& getOptions() const;
            bool getForceSDR() const;

            //! Get whether there is something to apply.
            bool isValid() const;

            //! Apply the transform in place to a tightly packed RGBA_U8
            //! image.  Thread safe.
            void apply(uint8_t* rgba, int width, int height) const;

        private:
            timeline::OCIOOptions _options;
            bool _forceSDR = true;
            OCIO::ConstCPUProcessorRcPtr _cpu;
        };

        //! Get a processor shared by everybody (all the clips of a
        //! timeline).  It is rebuilt only when the options change, and the
        //! returned object is never modified afterwards.  Never null; check
        //! isValid().
        std::shared_ptr<const OCIOCPUProcessor> getOCIOCPUProcessor(
            const timeline::OCIOOptions&, bool forceSDR = true);
    } // namespace TIMELINEUI
} // namespace tl
