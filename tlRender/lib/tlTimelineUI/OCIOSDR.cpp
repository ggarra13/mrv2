// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "OCIOSDR.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <vector>

namespace tl
{
    namespace TIMELINEUI
    {
        namespace
        {
            //! Lowercase and keep only letters and digits, so that
            //! "Rec 709", "Rec.709" and "rec-709" compare equal.
            std::string normalize(const std::string& s)
            {
                std::string out;
                for (unsigned char c : s)
                {
                    if (std::isalnum(c))
                        out.push_back(static_cast<char>(std::tolower(c)));
                }
                return out;
            }

            bool has(const std::string& normalized, const char* token)
            {
                return normalized.find(token) != std::string::npos;
            }

            bool isHDRDisplay(const std::string& display)
            {
                const std::string d = normalize(display);
                return has(d, "st2084") || has(d, "rec2100") ||
                       has(d, "hlg") || has(d, "pq") || has(d, "hdr");
            }

            bool isHDRView(const std::string& view)
            {
                const std::string v = normalize(view);
                return has(v, "hdr") || has(v, "st2084") || has(v, "hlg");
            }

            std::vector<std::string> getDisplays(
                const OCIO::ConstConfigRcPtr& config)
            {
                std::vector<std::string> out;
                for (int i = 0; i < config->getNumDisplays(); ++i)
                    out.push_back(config->getDisplay(i));
                return out;
            }

            std::vector<std::string> getViews(
                const OCIO::ConstConfigRcPtr& config,
                const std::string& display)
            {
                std::vector<std::string> out;
                const int n = config->getNumViews(display.c_str());
                for (int i = 0; i < n; ++i)
                    out.push_back(config->getView(display.c_str(), i));
                return out;
            }

            //! Find the SDR display that goes with an HDR one.
            std::string pickSDRDisplay(const OCIO::ConstConfigRcPtr& config)
            {
                const auto displays = getDisplays(config);

                // Preferred names, in order.
                for (const char* wanted :
                     {"srgbdisplay", "srgb", "rec709display", "rec1886display",
                      "rec709", "rec1886"})
                {
                    for (const auto& d : displays)
                        if (normalize(d) == wanted)
                            return d;
                }
                // Anything sRGB-ish that is not HDR.
                for (const auto& d : displays)
                    if (has(normalize(d), "srgb") && !isHDRDisplay(d))
                        return d;

                // The config default, if it is SDR.
                const std::string def = config->getDefaultDisplay();
                if (!def.empty() && !isHDRDisplay(def))
                    return def;

                // First SDR display.
                for (const auto& d : displays)
                    if (!isHDRDisplay(d))
                        return d;
                return std::string();
            }

            //! Rewrite an HDR view name the way an SDR one would be named.
            //!   "ACES 2.0 - HDR 1000 nits (P3 D65)"
            //!     -> "ACES 2.0 - SDR 100 nits (Rec.709)"
            std::string rewriteView(const std::string& view)
            {
                std::string out = std::regex_replace(
                    view,
                    std::regex(R"(HDR\s*\d+\s*nits)", std::regex::icase),
                    "SDR 100 nits");
                out = std::regex_replace(
                    out,
                    std::regex(
                        R"(\(\s*(P3|Rec\.?\s*2020|Rec\.?\s*2100)[^)]*\))",
                        std::regex::icase),
                    "(Rec.709)");
                // "HDR Video" -> "SDR Video" (ACES 1.x naming).
                out = std::regex_replace(
                    out, std::regex(R"(HDR)", std::regex::icase), "SDR");
                return out;
            }

            //! Text before the " - " (or the first "("), normalized.
            //! "ACES 2.0 - HDR 1000 nits" -> "aces20".
            std::string viewFamily(const std::string& view)
            {
                size_t end = view.find(" - ");
                if (end == std::string::npos)
                    end = view.find('(');
                return normalize(view.substr(0, end));
            }

            std::string pickSDRView(
                const OCIO::ConstConfigRcPtr& config,
                const std::string& sdrDisplay, const std::string& hdrView)
            {
                const auto views = getViews(config, sdrDisplay);
                if (views.empty())
                    return std::string();

                // 1) Exact match of the rewritten name.
                const std::string wanted = normalize(rewriteView(hdrView));
                for (const auto& v : views)
                    if (normalize(v) == wanted)
                        return v;

                // 2) Best scoring SDR view, preferring the same family
                // (ACES 2.0, ACES 1.0, AgX, ...).
                const std::string family = viewFamily(hdrView);
                int bestScore = 0;
                std::string best;
                for (const auto& v : views)
                {
                    if (isHDRView(v))
                        continue;
                    const std::string n = normalize(v);
                    int score = 0;
                    if (!family.empty() && viewFamily(v) == family)
                        score += 100;
                    if (has(n, "sdr"))
                        score += 20;
                    if (has(n, "100nits"))
                        score += 10;
                    if (has(n, "709"))
                        score += 5;
                    if (score > bestScore)
                    {
                        bestScore = score;
                        best = v;
                    }
                }
                if (!best.empty())
                    return best;

                // 3) The default view of the SDR display.
                return config->getDefaultView(sdrDisplay.c_str());
            }

            OCIO::ConstConfigRcPtr loadConfig(const std::string& fileName)
            {
                return fileName.empty()
                           ? OCIO::GetCurrentConfig()
                           : OCIO::Config::CreateFromFile(fileName.c_str());
            }
        } // namespace

        bool isHDRDisplayView(
            const std::string& display, const std::string& view)
        {
            return isHDRDisplay(display) || isHDRView(view);
        }

        std::pair<std::string, std::string> toSDRDisplayView(
            const OCIO::ConstConfigRcPtr& config, const std::string& display,
            const std::string& view)
        {
            if (!config || !isHDRDisplayView(display, view))
                return {display, view};

            std::string sdrDisplay = display;
            if (isHDRDisplay(display))
            {
                sdrDisplay = pickSDRDisplay(config);
                if (sdrDisplay.empty())
                    return {display, view};
            }
            const std::string sdrView = pickSDRView(config, sdrDisplay, view);
            if (sdrView.empty())
                return {display, view};
            return {sdrDisplay, sdrView};
        }

        timeline::OCIOOptions toSDR(const timeline::OCIOOptions& in)
        {
            // Cheap early out, this is called while drawing.
            if (!in.enabled || !isHDRDisplayView(in.display, in.view))
                return in;

            static std::mutex mutex;
            static std::map<std::string, std::pair<std::string, std::string> >
                cache;

            const std::string key =
                in.fileName + '\n' + in.display + '\n' + in.view;

            std::pair<std::string, std::string> result;
            {
                std::lock_guard<std::mutex> lock(mutex);
                auto i = cache.find(key);
                if (i == cache.end())
                {
                    result = {in.display, in.view};
                    try
                    {
                        result = toSDRDisplayView(
                            loadConfig(in.fileName), in.display, in.view);
                    }
                    catch (const std::exception&)
                    {
                        // Keep the original; also cached so we do not
                        // reload a broken config every frame.
                    }
                    i = cache.emplace(key, result).first;
                }
                result = i->second;
            }

            timeline::OCIOOptions out = in;
            out.display = result.first;
            out.view = result.second;
            return out;
        }

        void OCIOCPUProcessor::setOptions(
            const timeline::OCIOOptions& value, bool forceSDR)
        {
            if (value == _options && forceSDR == _forceSDR)
                return;
            _options = value;
            _forceSDR = forceSDR;
            _cpu.reset();

            // An empty input colorspace cannot be resolved here.
            if (!value.enabled || value.input.empty() ||
                value.display.empty() || value.view.empty())
                return;

            try
            {
                const auto sdr = forceSDR ? toSDR(value) : value;
                const auto config = loadConfig(sdr.fileName);

                auto transform = OCIO::DisplayViewTransform::Create();
                transform->setSrc(sdr.input.c_str());
                transform->setDisplay(sdr.display.c_str());
                transform->setView(sdr.view.c_str());

                auto pipeline = OCIO::LegacyViewingPipeline::Create();
                pipeline->setDisplayViewTransform(transform);
                if (!sdr.look.empty())
                {
                    pipeline->setLooksOverrideEnabled(true);
                    pipeline->setLooksOverride(sdr.look.c_str());
                }
                auto processor = pipeline->getProcessor(config);
                _cpu = processor->getOptimizedCPUProcessor(
                    OCIO::BIT_DEPTH_UINT8, OCIO::BIT_DEPTH_UINT8,
                    OCIO::OPTIMIZATION_DEFAULT);
            }
            catch (const std::exception&)
            {
                _cpu.reset(); // thumbnails stay untouched
            }
        }

        const timeline::OCIOOptions& OCIOCPUProcessor::getOptions() const
        {
            return _options;
        }

        bool OCIOCPUProcessor::getForceSDR() const
        {
            return _forceSDR;
        }

        bool OCIOCPUProcessor::isValid() const
        {
            return static_cast<bool>(_cpu);
        }

        void OCIOCPUProcessor::apply(
            uint8_t* rgba, int width, int height) const
        {
            if (!_cpu || !rgba || width <= 0 || height <= 0)
                return;
            try
            {
                OCIO::PackedImageDesc img(
                    rgba, width, height, OCIO::CHANNEL_ORDERING_RGBA,
                    OCIO::BIT_DEPTH_UINT8, 1, 4,
                    static_cast<ptrdiff_t>(width) * 4);
                _cpu->apply(img);
            }
            catch (const std::exception&)
            {
            }
        }

        std::shared_ptr<const OCIOCPUProcessor> getOCIOCPUProcessor(
            const timeline::OCIOOptions& options, bool forceSDR)
        {
            static std::mutex mutex;
            static std::shared_ptr<const OCIOCPUProcessor> current;

            std::lock_guard<std::mutex> lock(mutex);
            if (!current || current->getOptions() != options ||
                current->getForceSDR() != forceSDR)
            {
                auto processor = std::make_shared<OCIOCPUProcessor>();
                processor->setOptions(options, forceSDR);
                current = processor;
            }
            return current;
        }
    } // namespace TIMELINEUI
} // namespace tl
