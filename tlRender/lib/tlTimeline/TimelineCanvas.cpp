// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/TimelinePrivate.h>

#include <filesystem>

#include <tlTimeline/Util.h>
#include <tlTimeline/ZipPrivate.h>

#include <tlIO/SequenceIO.h>
#include <tlIO/System.h>

#include <tlCore/URL.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>
#include <tlCore/LogSystem.h>
#include <tlCore/Time.h>
#include <tlCore/Path.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

#include <algorithm>

namespace tl
{
    namespace timeline
    {
        //! Get the OTIO spatial coordinates of a clip's active media
        //! reference.
        std::optional<math::Box2f> getClipBounds(const OTIO_NS::Clip* otioClip)
        {
            return getMediaReferenceBounds(otioClip->media_reference());
        }

        //! Get the union of the OTIO spatial coordinates of every media
        //! reference on a clip. The canvas is built from this rather than from
        //! the active reference, so that changing the active media reference
        //! leaves the canvas unchanged.
        std::optional<math::Box2f> getClipBoundsUnion(const OTIO_NS::Clip* otioClip)
        {
            std::optional<math::Box2f> out;
            for (const auto& i : otioClip->media_references())
            {
                if (const auto bounds = getMediaReferenceBounds(i.second))
                {
                    out = out.has_value() ?
                          math::expand(out.value(), bounds.value()) :
                          bounds.value();
                }
            }
            return out;
        }

        //! Convert OTIO spatial coordinates into image space.
        //!
        //! The OTIO coordinates are unit-less, so they are scaled by the
        //! pixels per unit established from the first clip that has them;
        //! bounds of "0, 0, 1920, 1080" and "0, 0, 16, 9" describe the same
        //! area and must give the same result. The Y axis is also flipped,
        //! since OTIO is Y-up and image space is Y-down.
        std::optional<math::Box2f> toImageSpace(
            const std::optional<math::Box2f>& bounds,
            double scale)
        {
            std::optional<math::Box2f> out;
            if (bounds.has_value())
            {
                const auto& min = bounds.value().min;
                const auto& max = bounds.value().max;
                out = math::Box2f(
                    math::Vector2f(min.x * scale, -max.y * scale),
                    math::Vector2f(max.x * scale, -min.y * scale));
            }
            return out;
        }

        //! Place a clip's spatial coordinates into image space.
        //!
        //! The bounds are passed in rather than read from the clip, since the
        //! caller decides whether they describe the media reference being read
        //! or the union of every reference on the clip.
        //!
        //! With Spatial::Normalize a clip that has no spatial coordinates is
        //! given the reference size, so that clips of differing resolutions
        //! are displayed at the same size. This covers timelines that were not
        //! authored with spatial coordinates at all.
        std::optional<math::Box2f> getSpatialBounds(
            const std::optional<math::Box2f>& clipBounds,
            Spatial spatial,
            const math::Size2i& normalizeSize,
            double scale)
        {
            std::optional<math::Box2f> out;
            if (Spatial::kNone == spatial)
            {
                return out;
            }
            out = toImageSpace(clipBounds, scale);
            if (!out.has_value() &&
                Spatial::Normalize == spatial &&
                normalizeSize.isValid())
            {
                out = math::Box2f(
                    math::Vector2f(0.F, -static_cast<float>(normalizeSize.h)),
                    math::Vector2f(static_cast<float>(normalizeSize.w), 0.F));
            }
            return out;
        }

        //! Get the OTIO spatial coordinates of a media reference. These are
        //! optional; media without them is laid out from the image size.
        //! The coordinates are returned as authored, in the OTIO coordinate
        //! system: unit-less and Y-up.
        std::optional<math::Box2f> getMediaReferenceBounds(
            const OTIO_NS::MediaReference* otioMediaReference)
        {
            std::optional<math::Box2f> out;
            if (otioMediaReference)
            {
                const auto bounds = otioMediaReference->available_image_bounds();
                if (bounds.has_value())
                {
                    const auto& min = bounds.value().min;
                    const auto& max = bounds.value().max;
                    out = math::Box2f(
                        math::Vector2f(min.x, min.y),
                        math::Vector2f(max.x, max.y));
                }
            }
            return out;
        }

        //! Get a clip's box within the timeline canvas.
        std::optional<math::Box2f> getCanvasBox(
            const std::optional<math::Box2f>& clipBounds,
            Spatial spatial,
            const math::Size2i& normalizeSize,
            double scale,
            const math::Vector2f& offset)
        {
            std::optional<math::Box2f> out;
            if (const auto bounds = getSpatialBounds(
                    clipBounds,
                    spatial,
                    normalizeSize,
                    scale))
            {
                out = bounds.value() + offset;
            }
            return out;
        }

        void Timeline::_getCanvas()
        {
            TLRENDER_P();
            // The OTIO spatial coordinates describe a single canvas shared by the
            // whole timeline, so the extent is taken from every clip rather than
            // from the clips visible at one time. This keeps the render size
            // stable as playback moves between clips.
            // Built from the largest media reference resolution rather than from
            // the reference that is active, so that the canvas does not cap a
            // switch to a higher resolution reference; see _getVideoInfo().
            if (p.maxVideoSize.isValid())
            {
                p.normalizeSize.w = p.maxVideoSize.w;
                p.normalizeSize.h = p.maxVideoSize.h;
            }
            else if (!p.ioInfo.video.empty())
            {
                p.normalizeSize.w = p.ioInfo.video[0].size.w;
                p.normalizeSize.h = p.ioInfo.video[0].size.h;
            }
            else
            {
                p.normalizeSize = math::Size2i();
            }
            // p.normalizeSize = p.maxVideoSize.isValid() ?
            //                   p.maxVideoSize :
            //                   (!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : math::Size2i());
            const math::Size2i& normalizeSize = p.normalizeSize;
            const auto otioClips = p.otioTimeline.value->find_children<OTIO_NS::Clip>();

            // The coordinates are unit-less, so a reference is needed to map them
            // onto a pixel size. Take it from the first clip that has coordinates,
            // which is not necessarily the first clip in the timeline, together
            // with the resolution the timeline is working at.
            //
            // This uses the active media reference rather than the union of all of
            // them, unlike the canvas below. The coordinates of a clip's
            // references describe the same area, so any of them gives the same
            // scale; taking the union here would only matter for a clip whose
            // references were authored inconsistently, where the active one is the
            // better guide.
            if (normalizeSize.isValid())
            {
                for (const auto& otioClip : otioClips)
                {
                    if (const auto bounds = getClipBounds(otioClip))
                    {
                        const float w = bounds.value().getSize().w;
                        if (w > 0.F)
                        {
                            p.boundsScale = normalizeSize.w / w;
                            break;
                        }
                    }
                }
            }

            std::optional<math::Box2f> canvas;
            for (const auto& otioClip : otioClips)
            {
                // Report the coordinates as they were authored, so the numbers in
                // the file can be seen alongside the canvas derived from them.
                if (const auto authored = getClipBounds(otioClip))
                {
                    p.ioInfo.tags[string::Format("OTIO Image Bounds {0}").
                                  arg(otioClip->name())] =
                        string::Format("{0}, {1}, {2}, {3}").
                        arg(authored.value().min.x).
                        arg(authored.value().min.y).
                        arg(authored.value().max.x).
                        arg(authored.value().max.y);
                }
                // Cover every media reference, not just the active one, so that
                // changing the active media reference cannot place a clip outside
                // the canvas.
                if (const auto bounds = getSpatialBounds(
                        getClipBoundsUnion(otioClip),
                        p.options.spatial,
                        normalizeSize,
                        p.boundsScale))
                {
                    canvas = canvas.has_value() ?
                             math::expand(canvas.value(), bounds.value()) :
                             bounds.value();
                }
            }
            if (canvas.has_value())
            {
                const math::Size2f size = canvas.value().getSize();
                if (size.w > 0.F && size.h > 0.F)
                {
                    p.canvasOffset = -canvas.value().min;
                    p.canvasSize = math::Size2i(
                        static_cast<int>(std::round(size.w)),
                        static_cast<int>(std::round(size.h)));
                    p.ioInfo.tags["OTIO Canvas"] =
                        string::Format("{0}").arg(p.canvasSize);
                    p.ioInfo.tags["OTIO Pixels Per Unit"] =
                        string::Format("{0}").arg(p.boundsScale);
                }
            }
            else if (p.maxVideoSize.isValid())
            {
                // No clip in this timeline carries OTIO spatial coordinates,
                // so there is no authored canvas to derive from. Fall back to
                // the largest native resolution found across every clip in the
                // timeline (see _getMaxVideoSize()), so that
                // VideoFrame::canvasSize is still valid
                // and getInfos() (CompareOptions.cpp) substitutes it for the
                // raw per-frame image size on every frame -- giving
                // getRenderSize() one stable value for the whole timeline
                // instead of tracking whichever clip happens to be playing.
                p.canvasOffset = math::Vector2f();
                p.canvasSize = p.maxVideoSize;
            }
        }
    }
}
