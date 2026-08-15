// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            CompareMode, "A", "B", "Wipe", "Overlay", "Difference",
            "Multiply", "Add", "Horizontal", "Vertical", "Tile", "Butterfly");
        TLRENDER_ENUM_SERIALIZE_IMPL(CompareMode);

        TLRENDER_ENUM_IMPL(CompareTimeMode, "Relative", "Absolute");
        TLRENDER_ENUM_SERIALIZE_IMPL(CompareTimeMode);

        namespace
        {
            //! Get the image information used to lay out each video frame,
            //! taken from the first layer that has an image.
            //!
            //! When the frame has a canvas from the OTIO spatial coordinates,
            //! the canvas size is substituted for the image size. The canvas
            //! spans the whole timeline, so the layout stays the same no
            //! matter which clips are visible or what resolution they were
            //! rendered at; the layers are positioned within it when they are
            //! drawn. The canvas already describes the display geometry, so
            //! the pixel aspect ratio is not applied a second time.
            std::vector<image::Info> getInfos(const std::vector<VideoFrame>& videoFrame)
            {
                std::vector<image::Info> out;
                for (const auto& i : videoFrame)
                {
                    image::Info info;
                    for (const auto& layer : i.layers)
                    {
                        if (layer.image)
                        {
                            info = layer.image->getInfo();
                            break;
                        }
                        else if (layer.imageB)
                        {
                            info = layer.imageB->getInfo();
                            break;
                        }
                    }
                    if (i.canvasSize.isValid())
                    {
                        info.size.w = i.canvasSize.w;
                        info.size.h = i.canvasSize.h;
                        info.size.pixelAspectRatio = 1.F;
                    }
                    out.push_back(info);
                }
                return out;
            }
        }

        std::vector<math::Box2i> getBounds(
            const CompareOptions& options,
            const AspectRatioOptions& aspectRatioOptions,
            const std::vector<image::Info>& infos)
        {
            std::vector<math::Box2i> out;
            const size_t count = infos.size();
            switch (options.mode)
            {
            case CompareMode::Horizontal:
            {
                math::Size2i size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(math::Box2i(0, 0, size.w, size.h));
                }
                if (options.fitToA && count > 1)
                {
                    out.push_back(getBox(
                                      math::Box2i(size.w, 0, size.w, size.h),
                                      infos[1],
                                      aspectRatioOptions,
                                      BoxHAlign::Left));
                }
                else if (count > 1)
                {
                    const math::Size2i sizeB =
                        getRenderSize(infos[1], aspectRatioOptions);
                    out.push_back(math::Box2i(size.w, 0, sizeB.w, sizeB.h));
                }
                break;
            }
            case CompareMode::Vertical:
            {
                math::Size2i size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(math::Box2i(0, 0, size.w, size.h));
                }
                if (options.fitToA && count > 1)
                {
                    out.push_back(getBox(
                                      math::Box2i(0, size.h, size.w, size.h),
                                      infos[1],
                                      aspectRatioOptions,
                                      BoxHAlign::Center,
                                      BoxVAlign::Top));
                }
                else if (count > 1)
                {
                    const math::Size2i sizeB =
                        getRenderSize(infos[1], aspectRatioOptions);
                    out.push_back(math::Box2i(0, size.h, sizeB.w, sizeB.h));
                }
                break;
            }
            case CompareMode::Tile:
                if (count > 0)
                {
                    const int cols = std::max(1, static_cast<int>(std::ceil(std::sqrt(count))));
                    math::Size2i size;
                    if (options.fitToA)
                    {
                        size = getRenderSize(infos[0], aspectRatioOptions);
                    }
                    else
                    {
                        for (size_t i = 0; i < count; ++i)
                        {
                            const math::Size2i size2 =
                                getRenderSize(infos[i], aspectRatioOptions);
                            size.w = std::max(size.w, size2.w);
                            size.h = std::max(size.h, size2.h);
                        }
                    }
                    int c = 0;
                    int x = 0;
                    int y = 0;
                    for (size_t i = 0; i < count; ++i)
                    {
                        out.push_back(math::Box2i(x, y, size.w, size.h));
                        if (++c >= cols)
                        {
                            c = 0;
                            x = 0;
                            y += size.h;
                        }
                        else
                        {
                            x += size.w;
                        }
                    }
                }
                break;
            default:
                if (options.fitToA && count > 0)
                {
                    const math::Size2i size = getRenderSize(infos[0],
                                                            aspectRatioOptions);
                    for (size_t i = 0; i < count; ++i)
                    {
                        out.push_back(math::Box2i(0, 0, size.w, size.h));
                    }
                }
                else if (count > 0)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        const math::Size2i size =
                            getRenderSize(infos[i],
                                          aspectRatioOptions);
                        out.push_back(math::Box2i(0, 0, size.w, size.h));
                    }
                }
                break;
            }
            return out;
        }

        std::vector<math::Box2i> getBoxes(
            const CompareOptions& options,
            const AspectRatioOptions& aspectRatioOptions,
            const std::vector<image::Info>& infos)
        {
            std::vector<math::Box2i> out;
            const std::vector<math::Box2i> bounds =
                getBounds(options, aspectRatioOptions, infos);
            for (size_t i = 0; i < bounds.size() && i < infos.size(); ++i)
            {
                out.push_back(getBox(bounds[i], infos[i], aspectRatioOptions));
            }
            return out;
        }

        std::vector<math::Box2i> getBoxes(
            const CompareOptions& options,
            const AspectRatioOptions& aspectRatioOptions,
            const std::vector<VideoFrame>& videoFrame)
        {
            return getBoxes(options, aspectRatioOptions, getInfos(videoFrame));
        }

        std::vector<math::Box2i> getBoxes(
            const CompareOptions& options,
            const std::vector<DisplayOptions>& display,
            const std::vector<VideoFrame>& videoFrame)
        {
            return getBoxes(options,
                            !display.empty() ? display.front().aspect :
                            AspectRatioOptions(),
                            getInfos(videoFrame));
        }
        std::vector<math::Box2i> getBoxes(
            const CompareMode mode,
            const std::vector<DisplayOptions>& display,
            const std::vector<VideoFrame>& videoFrame)
        {
            CompareOptions options;
            options.mode = mode;
            return getBoxes(options, display, videoFrame);
        }

        math::Size2i getRenderSize(
            const CompareOptions& options,
            const AspectRatioOptions& aspectRatioOptions,
            const std::vector<image::Info>& infos)
        {
            math::Size2i out;
            const auto bounds = getBounds(options, aspectRatioOptions, infos);
            const auto bbox = math::bbox(bounds);
            switch (options.mode)
            {
            case CompareMode::A:
                if (!bounds.empty())
                {
                    out.w = bounds[0].w();
                    out.h = bounds[0].h();
                }
                break;
            case CompareMode::B:
                if (bounds.size() > 1)
                {
                    out.w = bounds[1].w();
                    out.h = bounds[1].h();
                }
                break;
            default:
                out.w = bbox.w();
                out.h = bbox.h();
                break;
            }
            return out;
        }

        math::Size2i getRenderSize(
            const CompareOptions& options,
            const AspectRatioOptions& aspectRatioOptions,
            const std::vector<VideoFrame>& videoFrame)
        {
            return getRenderSize(options, aspectRatioOptions,
                                 getInfos(videoFrame));
        }

        //! Get the render size for the given compare mode.
        math::Size2i getRenderSize(
            const CompareOptions& options,
            const std::vector<DisplayOptions>& display,
            const std::vector<VideoFrame>& videoFrame)
        {
            return getRenderSize(options,
                                 !display.empty() ? display.front().aspect :
                                 AspectRatioOptions(),
                                 videoFrame);
        }

        otime::RationalTime getCompareTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& compareTimeRange, CompareTimeMode mode)
        {
            otime::RationalTime out;
            switch (mode)
            {
            case CompareTimeMode::Relative:
            {
                const otime::RationalTime relativeTime =
                    sourceTime - sourceTimeRange.start_time();
                const otime::RationalTime relativeTimeRescaled =
                    relativeTime.rescaled_to(compareTimeRange.duration().rate())
                        .floor();
                out = compareTimeRange.start_time() + relativeTimeRescaled;
                break;
            }
            case CompareTimeMode::Absolute:
                out = sourceTime.rescaled_to(compareTimeRange.duration().rate())
                          .floor();
                break;
            default:
                break;
            }
            return out;
        }
    } // namespace timeline
} // namespace tl
