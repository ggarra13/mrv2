// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvIO.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"


#include <set>
#include <fstream>
#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;

#include <FL/fl_utf8.h>

#include <tlTimeline/Util.h>

#include <tlDraw/Annotation.h>

#include <tlIO/System.h>

#include <tlCore/Path.h>
#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/editAlgorithm.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>





namespace
{
    const char* kModule = "edit";
}

namespace mrv
{
    using otime::RationalTime;
    using otime::TimeRange;

    using otio::Clip;
    using otio::Composition;
    using otio::Gap;
    using otio::Item;
    using otio::Timeline;
    using otio::Track;
    using otio::Transition;

    namespace
    {
        struct FrameInfo
        {
            int trackIndex;
            double rate;
            std::string kind;
            otio::SerializableObject::Retainer<Item> item;
        };

        //! Frames copied.  We store in a vector to copy from multiple tracks,
        //! (Audio and Video for example).
        static std::vector<FrameInfo> copiedFrames;

        //! Undo/Redo queue element
        struct UndoRedo
        {
            std::string json;
            std::string fileName;
            std::vector<std::shared_ptr<draw::Annotation>> annotations;
        };

        static std::vector<UndoRedo> undoBuffer;
        static std::vector<UndoRedo> redoBuffer;

        std::vector<Composition*> getTracks(otio::Timeline* timeline)
        {
            std::vector<Composition*> out;
            auto tracks = timeline->tracks()->children();
            for (auto child : tracks)
            {
                auto composition =
                    otio::dynamic_retainer_cast<Composition>(child);
                if (!composition)
                    continue;
                out.push_back(composition);
            }
            return out;
        }

        std::vector<Composition*> getTracks(TimelinePlayer* player)
        {
            auto timeline = player->getTimeline();
            return getTracks(timeline);
        }

        RationalTime getTime(TimelinePlayer* player)
        {
            const auto& timeline_range = player->timeRange();
            const auto& startTime = timeline_range.start_time();
            const auto time = player->currentTime() - startTime;
            return time;
        }

        const otio::Timeline* createTimelineFromString(const std::string& s)
        {
            otio::ErrorStatus error;
            auto timeline = dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(s, &error));
            if (!timeline)
            {
                LOG_DEBUG("Could not crete timeline object:");
                LOG_ERROR(error.full_description);
                LOG_ERROR(".json string that failed:");
                LOG_ERROR(s);
                return nullptr;
            }
            return timeline;
        }

        // \@todo: darby needs to provide this from timelineUI
        std::vector<Item*> getSelectedItems()
        {
            std::vector<Item*> out;
            return out;
        }

        int getIndex(const otio::Composable* composable)
        {
            auto parent = composable->parent();
            return parent->index_of_child(composable);
        }

        void updateTimeline(
            otio::Timeline* timeline, const RationalTime& time, ViewerUI* ui)
        {
            auto player = ui->uiView->getTimelinePlayer();
            timeline->set_global_start_time(std::nullopt);
            player->setTimeline(timeline);
            const double rate = player->defaultSpeed();
            player->setSpeed(rate);
            player->setInOutRange(player->timeRange());
            ui->uiTimeline->setTimelinePlayer(player);
            ui->uiTimeline->redraw();

            // Set the start and end frame
            const auto one_frame = RationalTime(1.0, rate);
            const auto startTime = RationalTime(0.0, rate);
            auto endTime = startTime + timeline->duration() - one_frame;
            endTime = endTime.rescaled_to(rate);
            TimelineClass* c = ui->uiTimeWindow;
            c->uiStartFrame->setTime(startTime);
            c->uiEndFrame->setTime(endTime);
            player->seek(time);
        }

        //! Return whether a timeline has all empty tracks.
        bool hasEmptyTracks(otio::Stack* stack)
        {
            auto tracks = stack->children();
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->children().size() > 0)
                    return false;
            }
            return true;
        }

        // Routine to verify a clip is available and it is not an .otio file.
        bool verifySourceItem(int index, ViewerUI* ui)
        {
            auto model = ui->app->filesModel();
            const auto& sourceItems = model->observeFiles()->get();
            if (index < 0 || index >= sourceItems.size())
            {
                LOG_ERROR(_("Invalid index for add clip to timeline."));
                return false;
            }
            return true;
        }

        //! Auxiliary function to copy a frame from a track and item.
        void copy_frame_from_track(
            Composition* composition, Item* item, const RationalTime& time)
        {
            auto track = dynamic_cast<Track*>(composition);
            if (!track)
            {
                LOG_ERROR(composition->name() << " is not a track.");
                return;
            }
            auto transition = dynamic_cast<Transition*>(item);
            if (transition)
            {
                return;
            }

            otio::ErrorStatus errorStatus;
            FrameInfo frame;

            auto parent = composition->parent();
            frame.trackIndex = parent->index_of_child(composition);
            frame.kind = track->kind();
            auto trackRange = track->trimmed_range();
            double rate = trackRange.duration().rate();
            frame.rate = rate;

            auto clonedItem = dynamic_cast<Item*>(item->clone());
            if (!clonedItem)
                return;

            auto clip_range = item->trimmed_range(&errorStatus);
            if (is_error(errorStatus))
            {
                LOG_DEBUG(item->name() << " has no trimmed_range.");
                return;
            }
            auto track_range = item->trimmed_range_in_parent(&errorStatus);
            if (is_error(errorStatus))
            {
                LOG_DEBUG(item->name() << " is not attached to a track.");
                return;
            }
            auto one_frame = RationalTime(1.0, time.rate());
            auto range = TimeRange(
                time - track_range.value().start_time() +
                    clip_range.start_time(),
                one_frame);
            clonedItem->set_source_range(range);
            frame.item = clonedItem;
            copiedFrames.push_back(frame);
        }

        std::vector<std::shared_ptr<draw::Annotation> > deepCopyAnnotations(
            const std::vector<std::shared_ptr<draw::Annotation> >&
                originalAnnotations)
        {
            // First, do a deep copy of all annotations.
            std::vector<std::shared_ptr<draw::Annotation>> annotations;
            std::vector< draw::Annotation > flatAnnotations;
            for (const auto& annotation : originalAnnotations)
            {
                flatAnnotations.push_back(*annotation.get());
            }
            Message json = flatAnnotations;
            for (const auto& j : json)
            {
                std::shared_ptr< draw::Annotation > tmp =
                    draw::messageToAnnotation(j);
                annotations.push_back(tmp);
            }

            return annotations;
        }

        static size_t otioIndex = 1;
        file::Path savedPath, savedAudioPath;

        //! This routine makes paths absolute if possible.
        //! It uses the information from the current media item.
        void makePathsAbsolute(otio::Timeline* timeline, ViewerUI* ui)
        {
            auto stack = timeline->tracks();
            auto model = ui->app->filesModel();
            auto tracks = stack->children();
            auto item = model->observeA()->get();
            if (!item)
                return;
            auto path = item->path;
            auto audioPath = item->audioPath.isEmpty() ? path : item->audioPath;
            std::string directory, audioDirectory;

            if (file::isTemporaryEDL(path))
            {
                int videoClips = 0;
                int audioClips = 0;
                for (int i = 0; i < tracks.size(); ++i)
                {
                    auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                    if (!track)
                        continue;
                    if (track->kind() == otio::Track::Kind::video)
                    {
                        for (auto child : track->children())
                        {
                            auto clip =
                                otio::dynamic_retainer_cast<Clip>(child);
                            if (!clip)
                                continue;
                            ++videoClips;
                        }
                    }
                    else if (track->kind() == otio::Track::Kind::audio)
                    {
                        for (auto child : track->children())
                        {
                            auto clip =
                                otio::dynamic_retainer_cast<Clip>(child);
                            if (!clip)
                                continue;
                            ++audioClips;
                        }
                    }
                }
                if (videoClips == 1)
                    path = savedPath;
                if (audioClips == 1)
                    audioPath = savedAudioPath;
            }

            char currentDir[4096];
            if (fl_getcwd(currentDir, 4096) == nullptr)
            {
                LOG_ERROR(_("Could not get current path."));
            }

            if (path.isAbsolute())
            {
                directory = path.getDirectory();
            }
            else
            {
                directory = currentDir;
                directory += '/' + path.getDirectory();
            }

            if (audioPath.isAbsolute())
            {
                audioDirectory = audioPath.getDirectory();
            }
            else
            {
                audioDirectory = currentDir;
                audioDirectory += '/' + audioPath.getDirectory();
            }

            file::PathOptions options;
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video)
                {
                    for (auto child : track->children())
                    {
                        auto clip = otio::dynamic_retainer_cast<Clip>(child);
                        if (!clip)
                            continue;
                        auto media = clip->media_reference();
                        if (auto ref =
                                dynamic_cast<otio::ExternalReference*>(media))
                        {
                            file::Path urlPath(ref->target_url());
                            if (!urlPath.isAbsolute())
                            {
                                urlPath = timeline::getPath(
                                    media, directory, options);
                                ref->set_target_url(urlPath.get());
                            }
                        }
                        else if (
                            auto ref =
                                dynamic_cast<otio::ImageSequenceReference*>(
                                    media))
                        {
                            file::Path urlPath(ref->target_url_base());
                            if (!urlPath.isAbsolute())
                            {
                                urlPath = timeline::getPath(
                                    media, directory, options);
                                ref->set_target_url_base(
                                    urlPath.getDirectory());
                            }
                        }
                    }
                }
                else if (track->kind() == otio::Track::Kind::audio)
                {
                    for (auto child : track->children())
                    {
                        auto clip = otio::dynamic_retainer_cast<Clip>(child);
                        if (!clip)
                            continue;
                        auto media = clip->media_reference();
                        if (auto ref =
                                dynamic_cast<otio::ExternalReference*>(media))
                        {
                            file::Path urlPath(ref->target_url());
                            if (!urlPath.isAbsolute())
                            {
                                urlPath = timeline::getPath(
                                    media, audioDirectory, options);
                                ref->set_target_url(urlPath.get());
                            }
                        }
                    }
                }
            }
            savedPath = path;
            savedAudioPath = audioPath;
        }

        //! This routine tries to change all paths of a timeline, to make them
        //! relative to the otioFile location.
        void makePathsRelative(otio::Stack* stack, const std::string& otioFile)
        {
            auto tracks = stack->children();
            fs::path otioFilePath(otioFile);
            file::PathOptions options;
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                for (auto child : track->children())
                {
                    auto clip = otio::dynamic_retainer_cast<Clip>(child);
                    if (!clip)
                        continue;
                    auto media = clip->media_reference();
                    if (auto ref =
                            dynamic_cast<otio::ExternalReference*>(media))
                    {
                        file::Path urlPath(ref->target_url());
                        urlPath = getRelativePath(urlPath, otioFilePath);
                        ref->set_target_url(urlPath.get());
                    }
                    else if (
                        auto ref =
                            dynamic_cast<otio::ImageSequenceReference*>(media))
                    {
                        file::Path urlPath(
                            ref->target_url_base() + "/" + ref->name_prefix());
                        urlPath = getRelativePath(urlPath, otioFilePath);
                        ref->set_target_url_base(urlPath.getDirectory());
                    }
                }
            }
        }

        std::string _otioFilename(ViewerUI* ui)
        {
            char buf[256];
#ifdef _WIN32
            snprintf(buf, 256, "EDL0x%p.%zu.otio", ui, otioIndex);
#else
            snprintf(buf, 256, "EDL%p.%zu.otio", ui, otioIndex);
#endif
            auto out = tmppath() + '/' + buf;
            return out;
        }

        void toOtioFile(const otio::Timeline* timeline, ViewerUI* ui)
        {
            auto model = ui->app->filesModel();
            int index = model->observeAIndex()->get();
            if (index < 0)
                return;

            auto destItem = model->observeA()->get();
            auto path = destItem->path;
            auto stack = timeline->tracks();
            auto tracks = stack->children();
            if (tracks.size() < 1)
                return;

            bool create = false;
            std::string otioFile;
            if (file::isTemporaryEDL(path))
            {
                otioFile = path.get();
            }
            else
            {
                create = true;
                otioFile = otioFilename(ui);
            }

            bool refreshCache = hasEmptyTracks(stack);

            timeline->to_json_file(otioFile);
            destItem->path = file::Path(otioFile);

            if (refreshCache)
            {
                refresh_file_cache_cb(nullptr, ui);
            }
            else if (create)
            {
                panel::refreshThumbnails();
            }
        }

        //! Change clips' source range to use the highest video and audio
        //! sample rate.  Also returns the largest time range for the timeline.
        void sanitizeVideoAndAudioRates(
            otio::Timeline* timeline, TimeRange& timeRange, double& videoRate,
            double& sampleRate)
        {
            timeRange = time::invalidTimeRange;
            auto stack = timeline->tracks();
            auto tracks = stack->children();
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video)
                {
                    for (auto child : track->children())
                    {
                        auto clip = otio::dynamic_retainer_cast<Item>(child);
                        if (!clip)
                            continue;
                        auto range = clip->trimmed_range();
                        if (range.duration().rate() > videoRate)
                            videoRate = range.duration().rate();
                    }
                }
                else if (track->kind() == otio::Track::Kind::audio)
                {
                    for (auto child : track->children())
                    {
                        auto clip = otio::dynamic_retainer_cast<Item>(child);
                        if (!clip)
                            continue;
                        auto range = clip->trimmed_range();
                        if (range.duration().rate() > sampleRate)
                            sampleRate = range.duration().rate();
                    }
                }
            }

            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video)
                {
                    for (auto child : track->children())
                    {
                        auto item = otio::dynamic_retainer_cast<Item>(child);
                        if (!item)
                            continue;
                        if (videoRate > 0)
                        {
                            auto range = item->trimmed_range();
                            auto start = range.start_time()
                                             .rescaled_to(videoRate)
                                             .round();
                            auto duration =
                                range.duration().rescaled_to(videoRate).round();
                            range = TimeRange(start, duration);
                            item->set_source_range(range);
                        }
                    }
                    const TimeRange range = track->trimmed_range();
                    if (range.duration() >= timeRange.duration())
                    {
                        timeRange = range;
                    }
                }
                else if (track->kind() == otio::Track::Kind::audio)
                {
                    for (auto child : track->children())
                    {
                        auto item = otio::dynamic_retainer_cast<Item>(child);
                        if (!item)
                            continue;
                        if (sampleRate > 0)
                        {
                            auto range = item->trimmed_range();
                            auto start = range.start_time()
                                             .rescaled_to(sampleRate)
                                             .round();
                            auto duration = range.duration()
                                                .rescaled_to(sampleRate)
                                                .round();
                            range = TimeRange(start, duration);
                            duration = duration.rescaled_to(videoRate);
                            item->set_source_range(range);
                        }
                    }
                }
            }
        }

        //! Remove annotations that are not in the time range.
        std::vector<std::shared_ptr<draw::Annotation>> removeAnnotations(
            const TimeRange& range,
            const std::vector<std::shared_ptr<draw::Annotation>>& annotations)
        {
            std::vector<std::shared_ptr<draw::Annotation>> out;
            // Add annotations that are for all frames.
            for (auto a : annotations)
            {
                if (a->allFrames)
                    out.push_back(a);
            }
            // Append annotations that intersect the range.
            for (auto a : annotations)
            {
                if (range.contains(a->time))
                {
                    out.push_back(a);
                }
            }
            return out;
        }

        //! Offset annotations by offset time.  Used in cut/insert frame.
        std::vector<std::shared_ptr<draw::Annotation>> offsetAnnotations(
            const RationalTime& time, const RationalTime& offset,
            const std::vector<std::shared_ptr<draw::Annotation>>& annotations)
        {
            std::vector<std::shared_ptr<draw::Annotation>> out;
            // Append annotations that come before.
            for (auto a : annotations)
            {
                if (a->allFrames || a->time < time)
                    out.push_back(a);
            }
            // Append annotations that come after.
            for (auto a : annotations)
            {
                if (a->time > time + offset)
                {
                    out.push_back(a);
                    out.back()->time += offset;
                }
            }
            return out;
        }

        //! Routine used to add annotations to the end of the timeline,
        //! once a clip is added.
        std::vector<std::shared_ptr<draw::Annotation>> addAnnotations(
            const RationalTime& duration,
            const std::vector<std::shared_ptr<draw::Annotation>>&
                playerAnnotations,
            const TimeRange& clipRange,
            const std::vector<std::shared_ptr<draw::Annotation>>&
                clipAnnotations)
        {
            std::vector<std::shared_ptr<draw::Annotation>> out;

            // Shallow copy player annotations
            for (auto a : playerAnnotations)
            {
                out.push_back(a);
            }

            // First, do a deep copy of all clip annotations.
            auto annotations = deepCopyAnnotations(clipAnnotations);
            for (auto& a : annotations)
            {
                auto& time = a->time;
                if (!clipRange.contains(time))
                    continue;
                time += duration;
                out.push_back(a);
            }
            return out;
        }

        //! Switches to EDL clip for timeline processing during an Undo/Redo.
        bool switchToEDL(const std::string& fileName, ViewerUI* ui)
        {
            auto model = ui->app->filesModel();
            auto Aindex = model->observeAIndex()->get();
            auto Aitem = model->observeA()->get();
            if (!Aitem)
                return false;

            if (Aitem->path.get() == fileName)
                return true;

            // Not on undo/redo EDL item, look for it.
            int idx = -1;
            auto items = model->observeFiles()->get();
            for (int i = 0; i < items.size(); ++i)
            {
                if (items[i]->path.get() == fileName)
                {
                    idx = i;
                    break;
                }
            }

            if (idx == -1)
            {
                /* xgettext:c++-format */
                std::string err =
                    string::Format(_("EDL item '{0}' no longer loaded.  "
                                     "Cannot undo or redo."))
                        .arg(fileName);
                LOG_ERROR(err);
                return false;
            }
            else
            {
                model->setA(idx);
                return true;
            }
        }

        //! Return the EDL name for the Undo/Redo queue.
        std::string getEDLName(ViewerUI* ui)
        {
            auto model = ui->app->filesModel();
            auto Aitem = model->observeA()->get();

            const file::Path path = Aitem->path;
            if (!file::isTemporaryEDL(path))
                return _otioFilename(ui);

            return path.get();
        }

    } // anonymous namespace

    std::string otioFilename(ViewerUI* ui)
    {
        auto out = _otioFilename(ui);
        ++otioIndex;
        return out;
    }

    file::Path getRelativePath(const file::Path& path, const fs::path& fileName)
    {
        fs::path filePath = path.get();
        // Make file absolute, then remove it, leaving directory
        fs::path directory = fs::absolute(fileName).parent_path();
        fs::path relative = fs::relative(filePath, directory);
        std::string file = relative.u8string();
        if (file.empty())
            return path;
        return file::Path(file);
    }

    void toOtioFile(TimelinePlayer* player, ViewerUI* ui)
    {
        auto timeline = player->getTimeline();
        updateTimeline(timeline, player->currentTime(), ui);
        toOtioFile(timeline, ui);
    }

    void makePathsAbsolute(TimelinePlayer* player, ViewerUI* ui)
    {
        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        makePathsAbsolute(timeline, ui);
    }

    void edit_store_undo(TimelinePlayer* player, ViewerUI* ui)
    {
        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        auto view = ui->uiView;

        makePathsAbsolute(timeline, ui);

        const std::string state = timeline->to_json_string();
        if (!undoBuffer.empty())
        {
            // Don't store anything if no change.
            if (undoBuffer.back().json == state)
            {
                return;
            }
        }

        toOtioFile(timeline, ui);
        UndoRedo buffer;
        buffer.json = state;
        buffer.fileName = getEDLName(ui);

        player = ui->uiView->getTimelinePlayer();
        buffer.annotations = player->getAllAnnotations();
        undoBuffer.push_back(buffer);

        ui->uiUndoEdit->activate();
    }

    void edit_clear_redo(ViewerUI* ui)
    {
        redoBuffer.clear();
        ui->uiRedoEdit->deactivate();
    }

    bool edit_has_undo()
    {
        return !undoBuffer.empty();
    }

    bool edit_has_redo()
    {
        return !redoBuffer.empty();
    }

    void edit_store_redo(TimelinePlayer* player, ViewerUI* ui)
    {   
        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        auto view = ui->uiView;
        const std::string state = timeline->to_json_string();
        if (!redoBuffer.empty())
        {
            // Don't store anything if no change.
            if (redoBuffer.back().json == state)
            {
                return;
            }
        }

        toOtioFile(timeline, ui);
        UndoRedo buffer;
        buffer.json = state;
        buffer.fileName = getEDLName(ui);
        player = ui->uiView->getTimelinePlayer();
        buffer.annotations = player->getAllAnnotations();
        redoBuffer.push_back(buffer);
        ui->uiRedoEdit->activate();
    }

    void edit_copy_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        player->stop();

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        makePathsAbsolute(timeline, ui);

        const auto time = getTime(player);

        copiedFrames.clear();

        otio::ErrorStatus errorStatus;
        auto tracks = timeline->tracks()->children();
        for (auto child : tracks)
        {
            auto composition = otio::dynamic_retainer_cast<Composition>(child);
            if (!composition)
                continue;
            auto item = otio::dynamic_retainer_cast<Item>(
                composition->child_at_time(time, &errorStatus));
            if (!item || otio::is_error(errorStatus))
                continue;
            copy_frame_from_track(composition, item, time);
        }

        tcp->pushMessage("Edit/Frame/Copy", time);
    }

    void edit_cut_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        edit_copy_frame_cb(m, ui);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        auto tracks = timeline->tracks()->children();

        const auto startTime = player->timeRange().start_time();
        const auto time = player->currentTime() - startTime;
        const auto one_frame = RationalTime(1.0, time.rate());
        const auto half_frame = RationalTime(0.4, time.rate());
        const RationalTime out_time = time + one_frame;

        edit_store_undo(player, ui);

        otio::ErrorStatus errorStatus;
        for (const auto& frame : copiedFrames)
        {
            const int trackIndex = frame.trackIndex;
            if (trackIndex < 0 ||
                static_cast<size_t>(trackIndex) >= tracks.size())
                continue;

            auto track = otio::dynamic_retainer_cast<Track>(tracks[trackIndex]);
            if (!track)
                continue;
            auto item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            if (!item)
                continue;

            // Cut first at current time
            otio::algo::slice(track, time);

            // Cut again at current time + 1 frame
            otio::algo::slice(track, out_time);

            // Adjust time by almost half a frame to avoid rounding issues in
            // the audio tracks.
            auto trackTime = time + half_frame;

            // Get the cut item
            item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(trackTime, &errorStatus));
            if (!item)
                continue;

            int index = track->index_of_child(item);
            auto children_size = track->children().size();
            if (index < 0 || static_cast<size_t>(index) >= children_size)
                continue;

            // Remove the cut item (ie. one frame).
            track->remove_child(index);
        }

        auto annotations =
            offsetAnnotations(time, -one_frame, player->getAllAnnotations());
        player->setAllAnnotations(annotations);
        edit_clear_redo(ui);

        updateTimeline(timeline, time, ui);
        toOtioFile(timeline, ui);


        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();
        
        tcp->pushMessage("Edit/Frame/Cut", time);
    }

    void edit_paste_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        player->stop();

        const auto time = getTime(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        auto stack = timeline->tracks();
        if (!stack)
            return;

        auto tracks = stack->children();

        edit_store_undo(player, ui);

        double videoRate = 0.F, sampleRate = 0.F;

        for (const auto& frame : copiedFrames)
        {
            if (frame.kind == Track::Kind::video)
            {
                if (frame.rate > videoRate)
                    videoRate = frame.rate;
            }
            else if (frame.kind == Track::Kind::audio)
            {
                if (frame.rate > sampleRate)
                    sampleRate = frame.rate;
            }
        }

        TimeRange timeRange;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        const RationalTime scaledTime = time.rescaled_to(videoRate);
        const TimeRange range(scaledTime, RationalTime(1.0, videoRate));

        for (auto& frame : copiedFrames)
        {
            auto item = dynamic_cast<Item*>(frame.item->clone());
            if (!item)
                continue;
            const int trackIndex = frame.trackIndex;
            if (trackIndex < 0 ||
                static_cast<size_t>(trackIndex) >= tracks.size())
                continue;

            auto track = otio::dynamic_retainer_cast<Track>(tracks[trackIndex]);
            if (!track)
                continue;

            if (track->kind() != frame.kind)
                continue;

            auto track_range = track->trimmed_range();
            auto rate = track_range.duration().rate();
            TimeRange rescaledRange = range;
            if (rate > range.duration().rate())
            {
                rescaledRange = TimeRange(
                    range.start_time().rescaled_to(rate),
                    range.duration().rescaled_to(rate));
            }

            otio::ErrorStatus errorStatus;
            otio::algo::overwrite(
                item, track, rescaledRange, true, nullptr, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                /* xgettext:c++-format */
                std::string err =
                    string::Format(_("Could not paste {0}.  Error {1}."))
                        .arg(track->kind())
                        .arg(errorStatus.full_description);
                LOG_ERROR(err);
            }
            frame.item = item;
        }

        edit_clear_redo(ui);

        updateTimeline(timeline, scaledTime, ui);

        toOtioFile(timeline, ui);

        panel::redrawThumbnails();
        
        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Frame/Paste", time);
    }

    void edit_insert_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        player->stop();

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        const auto time = getTime(player);
        auto tracks = timeline->tracks()->children();

        edit_store_undo(player, ui);

        double videoRate = 0.F, sampleRate = 0.F;
        for (const auto& frame : copiedFrames)
        {
            if (frame.kind == Track::Kind::video)
            {
                if (frame.rate > videoRate)
                    videoRate = frame.rate;
            }
            else if (frame.kind == Track::Kind::audio)
            {
                if (frame.rate > sampleRate)
                    sampleRate = frame.rate;
            }
        }

        TimeRange timeRange;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        const RationalTime scaledTime = time.rescaled_to(videoRate);

        for (auto& frame : copiedFrames)
        {
            auto item = dynamic_cast<Item*>(frame.item->clone());
            if (!item)
                continue;
            const int trackIndex = frame.trackIndex;
            if (trackIndex < 0 ||
                static_cast<size_t>(trackIndex) >= tracks.size())
                continue;

            auto track = otio::dynamic_retainer_cast<Track>(tracks[trackIndex]);
            if (track->kind() != frame.kind)
                continue;

            otio::algo::insert(item, track, scaledTime);
            frame.item = item;
        }

        const RationalTime one_frame(1.0, scaledTime.rate());
        auto annotations = offsetAnnotations(
            scaledTime, one_frame, player->getAllAnnotations());
        player->setAllAnnotations(annotations);

        edit_clear_redo(ui);

        updateTimeline(timeline, scaledTime, ui);
        toOtioFile(timeline, ui);

        panel::redrawThumbnails();
        
        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Frame/Insert", time);
    }

    
    void edit_slice_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        player->stop();

        edit_store_undo(player, ui);

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        bool remove_undo = true;
        otio::ErrorStatus errorStatus;
        for (auto track : tracks)
        {
            auto item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            if (!item)
                continue;
            auto cut_range =
                item->trimmed_range_in_parent(&errorStatus).value();

            // For slicing, if we slice at the same point of another slice,
            // we may get an useless undo.  We avoid it.
            if (cut_range.start_time() == time ||
                cut_range.end_time_exclusive() == time)
                continue;
            remove_undo = false;
            otio::algo::slice(track, time);
        }

        // If we sliced on the start or end of all clips, we don't need to
        // store the undo.
        if (remove_undo)
            undoBuffer.pop_back();

        edit_clear_redo(ui);

        player->setTimeline(timeline);
        toOtioFile(timeline, ui);
        
        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Slice", time);
    }

    void edit_remove_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        auto stack = timeline->tracks();

        edit_store_undo(player, ui);

        const auto half_frame = RationalTime(0.4, time.rate());

        for (auto track : tracks)
        {
            // Adjust time by almost half a frame to avoid rounding issues in
            // the audio tracks.
            auto trackTime = time + half_frame;

            otio::algo::remove(track, trackTime, false);
        }

        updateTimeline(timeline, time, ui);

        player = ui->uiView->getTimelinePlayer();

        auto annotations =
            removeAnnotations(player->timeRange(), player->getAllAnnotations());

        player->setAllAnnotations(annotations);

        ui->uiTimeline->setTimelinePlayer(player);

        toOtioFile(timeline, ui);

        edit_clear_redo(ui);

        panel::redrawThumbnails();
        
        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Remove", time);
    }

    void edit_insert_audio_clip_cb(ViewerUI* ui, const std::string& audioFile)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        tl::file::Path audioPath(audioFile);

        // Find first video clip at current time.
        int clipIndex = -1;
        otio::ErrorStatus errorStatus;
        otio::Clip* clip = nullptr;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::video)
                continue;

            clip = otio::dynamic_retainer_cast<Clip>(
                track->child_at_time(time, &errorStatus));
            if (!clip)
                continue;

            clipIndex = track->index_of_child(clip);
            break;
        }

        if (!clip || clipIndex < 0)
            return;

        edit_store_undo(player, ui);

        auto clipRange = clip->trimmed_range();
        auto range = clip->trimmed_range_in_parent().value();

        // Check if no audio tracks.  If that's the case, add one audio track
        bool refreshMedia = false;
        bool hasAudioTrack = false;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (track->kind() != otio::Track::Kind::audio)
                continue;

            hasAudioTrack = true;
            break;
        }

        bool modified = false;
        if (!hasAudioTrack)
        {
            auto stack = timeline->tracks();

            // Append a new audio track
            auto track = new otio::Track(
                "Audio", std::nullopt, otio::Track::Kind::audio);
            stack->append_child(track, &errorStatus);
            if (is_error(errorStatus))
            {
                LOG_DEBUG("stack->append_child(track) failed with:");
                LOG_ERROR(errorStatus.full_description);
            }

            modified = true;
            refreshMedia = true;

            updateTimeline(timeline, time, ui);
            compositions = getTracks(player);
        }

        auto context = App::app->getContext();
        auto ioSystem = context->getSystem<io::System>();

        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (track->kind() != otio::Track::Kind::audio)
                continue;

            auto sampleRate = track->trimmed_range().duration().rate();

            auto rangeInTrack = otime::TimeRange(
                range.start_time().rescaled_to(sampleRate),
                range.duration().rescaled_to(sampleRate));

            auto audioItem = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));

            if (audioItem)
            {
                auto audioRange = audioItem->trimmed_range_in_parent().value();
                if (audioRange == rangeInTrack &&
                    otio::dynamic_retainer_cast<otio::Clip>(audioItem))
                    continue;

                if (audioRange == rangeInTrack)
                    continue;
            }

            int audioIndex = track->index_of_child(audioItem);

            const timeline::Options options;
            if (auto read = ioSystem->read(audioPath, options.ioOptions))
            {
                const auto info = read->getInfo().get();
                if (!info.audio.isValid())
                    continue;

                otio::Clip* audioClip = new otio::Clip;
                audioClip->set_source_range(info.audioTime);
                audioClip->set_media_reference(new otio::ExternalReference(
                                                   audioPath.get(),
                                                   info.audioTime));

                if (audioIndex < 0 || audioIndex >= track->children().size())
                {
                    track->append_child(audioClip, &errorStatus);
                    if (is_error(errorStatus))
                    {
                        LOG_DEBUG(
                            "track->append_child(audioClip) failed with:");
                        LOG_ERROR(errorStatus.full_description);
                    }
                }
                else
                {
                    track->insert_child(audioIndex, audioClip, &errorStatus);
                    if (is_error(errorStatus))
                    {
                        LOG_DEBUG(
                            "track->insert_child(audioClip) "
                            << audioIndex << " failed with:");
                        LOG_ERROR(errorStatus.full_description);
                    }
                }

                modified = true;
                break;
            }
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
        {
            edit_clear_redo(ui);

            if (refreshMedia)
                refresh_media_cb(nullptr, ui);
        }
        
        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Audio Clip/Insert", audioFile);
    }

    void insert_audio_clip_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        std::string audioFile = open_audio_file(nullptr);
        if (audioFile.empty())
            return;
        edit_insert_audio_clip_cb(ui, audioFile);
    }

    void edit_insert_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        // Find first video item at current time.
        int itemIndex = -1;
        otio::ErrorStatus errorStatus;
        otio::Item* item = nullptr;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::video)
                continue;

            item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            if (!item)
                continue;

            itemIndex = track->index_of_child(item);
            break;
        }

        if (!item || itemIndex < 0)
            return;

        edit_store_undo(player, ui);

        auto itemRange = item->trimmed_range();
        auto range = item->trimmed_range_in_parent().value();

        // Check if no audio tracks.  If that's the case, add one audio track
        bool hasAudioTrack = false;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (track->kind() != otio::Track::Kind::audio)
                continue;

            hasAudioTrack = true;
            break;
        }

        bool modified = false;
        if (!hasAudioTrack)
        {
            auto stack = timeline->tracks();

            // Append a new audio track
            auto track = new otio::Track(
                "Audio", std::nullopt, otio::Track::Kind::audio);
            stack->append_child(track, &errorStatus);
            if (is_error(errorStatus))
            {
                LOG_DEBUG("stack->append_child(track) failed with:");
                LOG_ERROR(errorStatus.full_description);
            }

            modified = true;
            updateTimeline(timeline, time, ui);
            compositions = getTracks(player);
        }

        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (track->kind() != otio::Track::Kind::audio)
                continue;

            auto sampleRate = track->trimmed_range().duration().rate();

            auto rangeInTrack = otime::TimeRange(
                range.start_time().rescaled_to(sampleRate),
                range.duration().rescaled_to(sampleRate));

            auto audioItem = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));

            if (audioItem)
            {
                auto audioRange = audioItem->trimmed_range_in_parent().value();
                if (audioRange == rangeInTrack &&
                    otio::dynamic_retainer_cast<otio::Gap>(audioItem))
                    continue;

                if (audioRange == rangeInTrack)
                    continue;
            }

            modified = true;

            int audioIndex = track->index_of_child(audioItem);
            auto audioClipRange = otime::TimeRange(
                itemRange.start_time().rescaled_to(sampleRate),
                itemRange.duration().rescaled_to(sampleRate));
            otio::Gap* gap = new Gap(audioClipRange);
            if (audioIndex < 0 || audioIndex >= track->children().size())
            {
                track->append_child(gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("track->append_child(gap) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
            else
            {
                track->insert_child(audioIndex, gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG(
                        "track->insert_child(gap) " << audioIndex
                                                    << " failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();
        
        tcp->pushMessage("Edit/Audio Gap/Insert", time);
    }

    void edit_remove_audio_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        edit_store_undo(player, ui);

        bool modified = false;
        otio::ErrorStatus errorStatus;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::audio)
                continue;

            auto clip = otio::dynamic_retainer_cast<Clip>(
                track->child_at_time(time, &errorStatus));
            if (!clip)
                continue;

            int clipIndex = track->index_of_child(clip);
            if (clipIndex < 0 || clipIndex >= track->children().size())
                continue;

            modified = true;
            track->remove_child(clipIndex);
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Audio Clip/Remove", time);
    }

    void edit_remove_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        edit_store_undo(player, ui);

        bool modified = false;
        otio::ErrorStatus errorStatus;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::audio)
                continue;

            auto gap = otio::dynamic_retainer_cast<Gap>(
                track->child_at_time(time, &errorStatus));
            if (!gap)
                continue;

            int gapIndex = track->index_of_child(gap);
            if (gapIndex < 0 || gapIndex >= track->children().size())
                continue;

            modified = true;
            track->remove_child(gapIndex);
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Audio Gap/Remove", time);
    }
    
    void edit_insert_video_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        // Find first video clip at current time.
        int clipIndex = -1;
        otio::ErrorStatus errorStatus;
        otio::Clip* clip = nullptr;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::video)
                continue;

            clip = otio::dynamic_retainer_cast<Clip>(
                track->child_at_time(time, &errorStatus));
            if (!clip)
                continue;

            clipIndex = track->index_of_child(clip);
            break;
        }

        if (!clip || clipIndex < 0)
            return;

        edit_store_undo(player, ui);

        auto clipRange = clip->trimmed_range();
        auto range = clip->trimmed_range_in_parent().value();

        bool modified = false;
        
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (track->kind() != otio::Track::Kind::video)
                continue;

            auto rate = track->trimmed_range().duration().rate();

            auto rangeInTrack = otime::TimeRange(
                range.start_time().rescaled_to(rate),
                range.duration().rescaled_to(rate));

            auto videoItem = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));

            if (videoItem)
            {
                // If already a gap in the range, skip it.
                auto videoRange = videoItem->trimmed_range_in_parent().value();
                if (videoRange == rangeInTrack &&
                    otio::dynamic_retainer_cast<otio::Gap>(videoItem))
                    continue;
            }

            modified = true;

            int videoIndex = track->index_of_child(videoItem);
            auto videoClipRange = otime::TimeRange(
                clipRange.start_time().rescaled_to(rate),
                clipRange.duration().rescaled_to(rate));
            otio::Gap* gap = new Gap(videoClipRange);
            if (videoIndex < 0 || videoIndex >= track->children().size())
            {
                track->append_child(gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("track->append_child(gap) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
            else
            {
                track->insert_child(videoIndex, gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG(
                        "track->insert_child(gap) " << videoIndex
                                                    << " failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();
        
        tcp->pushMessage("Edit/Video Gap/Insert", time);
    }

    
    void edit_remove_video_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        edit_store_undo(player, ui);

        bool modified = false;
        otio::ErrorStatus errorStatus;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::video)
                continue;

            auto gap = otio::dynamic_retainer_cast<Gap>(
                track->child_at_time(time, &errorStatus));
            if (!gap)
                continue;

            int gapIndex = track->index_of_child(gap);
            if (gapIndex < 0 || gapIndex >= track->children().size())
                continue;

            modified = true;
            track->remove_child(gapIndex);
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();

        tcp->pushMessage("Edit/Video Gap/Remove", time);
    }

    void _addTransition(const otio::Item* left,
                        const otio::Item* right)
    {
        auto track = left->parent();
        if (track != right->parent())
        {
            LOG_ERROR(_("Items selected must be on the same track."));
            return;
        }

        auto left_range =  left->trimmed_range_in_parent().value();
        auto right_range = right->trimmed_range_in_parent().value();
        if (left_range.start_time() > right_range.start_time())
        {
            const  auto tmp = left;
            left = right;
            right = tmp;

            const auto tmp_range = left_range;
            left_range = right_range;
            right_range = tmp_range;
        }
        
        if (left_range.end_time_exclusive() != right_range.start_time())
        {
            std::string err = string::Format(
                _("Items selected must be contiguous on the track. "
                  "Left {0}.  Right {0}."))
                              .arg(left_range)
                              .arg(right_range);
            LOG_ERROR(err);
            return;
        }
        
        int left_index = track->index_of_child(left);
        if (left_index < 0 || left_index >= track->children().size())
        {
            throw std::runtime_error("Internal error: left item not in track!");
        }
        
        int right_index = track->index_of_child(right);
        if (right_index < 0 || right_index >= track->children().size())
        {
            throw std::runtime_error("Internal error: right item not in track!");
        }

        double left_rate = left_range.duration().rate();
        double right_rate = right_range.duration().rate();
        
        auto in_offset = RationalTime(std::min(
                                          std::max(1.0, left_range.duration().value() /
                                                   2.0), left_rate / 2.0), left_rate);
        auto out_offset = RationalTime(std::min(
                                           std::max(1.0, right_range.duration().value() /
                                                    2.0), right_rate / 2.0), right_rate);

        otio::Transition* transition =
            new otio::Transition("", "SMPTE_Dissolve", in_offset, out_offset);
        track->insert_child(left_index + 1, transition);
    }
    
    void edit_add_transition_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();
        if (!timeline)
            return;

        makePathsAbsolute(timeline, ui);

        
        auto selection = ui->uiTimeline->getSelectedItems();
        if (selection.size() != 2 && selection.size() != 4)
        {
            std::string err =
                string::Format(_("Please select two or four contiguous items.  "
                                 "Selected {0}.")).arg(selection.size());
            LOG_ERROR(err);
            return;
        }

        const otio::Item* left_video = nullptr;
        const otio::Item* right_video = nullptr;
        const otio::Item* left_audio = nullptr;
        const otio::Item* right_audio = nullptr;

        if (selection.size() == 2)
        {
            left_video = selection[0];
            right_video = selection[1];
            _addTransition(left_video, right_video);
        }
        else
        {
            // Video
            for (auto& item : selection)
            {
                auto composition = item->parent();
                auto track = dynamic_cast<otio::Track*>(composition);
                if (!track)
                    continue;

                if (track->kind() == otio::Track::Kind::video)
                {
                    if (!left_video) left_video = item;
                    else right_video = item;
                }
                else if (track->kind() == otio::Track::Kind::audio)
                {
                    if (!left_audio) left_audio = item;
                    else right_audio = item;
                }
            }

            if (left_video == nullptr || right_video == nullptr)
            {
                LOG_ERROR(_("Please select two contiguous video clips."));
                return;
            }

            if (left_audio == nullptr || right_audio == nullptr)
            {
                LOG_ERROR(_("Please select two contiguous audio clips."));
                return;
            }

            _addTransition(left_video, right_video);
            _addTransition(left_audio, right_audio);
        }
        
        const auto& time = getTime(player);
        updateTimeline(timeline, time, ui);
        toOtioFile(timeline, ui);
    }
    
    void edit_undo_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (undoBuffer.empty())
            return;

        tcp->pushMessage("Edit/Undo", 0);

        auto buffer = undoBuffer.back();
        undoBuffer.pop_back();
        if (undoBuffer.empty())
            ui->uiUndoEdit->deactivate();

        if (!switchToEDL(buffer.fileName, ui))
            return;

        // We must get player again, as we might have changed clips.
        player = ui->uiView->getTimelinePlayer();
        edit_store_redo(player, ui);

        const auto otioTimeline = createTimelineFromString(buffer.json);
        if (!otioTimeline)
            return;

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            otioTimeline);

        TimeRange timeRange;
        double videoRate = 0.F, sampleRate = 0.F;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        player->setAllAnnotations(buffer.annotations);
        updateTimeline(timeline, player->currentTime(), ui);

        toOtioFile(timeline, ui);

        panel::redrawThumbnails();
    }

    void edit_redo_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (redoBuffer.empty())
            return;

        tcp->pushMessage("Edit/Redo", 0);

        auto buffer = redoBuffer.back();
        redoBuffer.pop_back();
        if (redoBuffer.empty())
            ui->uiRedoEdit->deactivate();

        if (!switchToEDL(buffer.fileName, ui))
            return;

        // We must get player again, as we might have changed clips.
        player = ui->uiView->getTimelinePlayer();
        edit_store_undo(player, ui);

        auto stack = player->getTimeline()->tracks();
        const bool refreshCache = hasEmptyTracks(stack);

        const auto otioTimeline = createTimelineFromString(buffer.json);
        if (!otioTimeline)
            return;

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            otioTimeline);

        TimeRange timeRange;
        double videoRate = 0.F, sampleRate = 0.F;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        player->setAllAnnotations(buffer.annotations);
        updateTimeline(timeline, player->currentTime(), ui);

        toOtioFile(timeline, ui);

        panel::redrawThumbnails();

        if (refreshCache)
            refresh_file_cache_cb(nullptr, ui);
    }

    void shiftAnnotations(
        const otime::TimeRange& range, const otime::RationalTime& insertTime,
        const bool previous, ViewerUI* ui)
    {
        using namespace draw;

        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return;

        // First, deep copy the annotations.
        const auto& originalAnnotations = player->getAllAnnotations();
        auto annotations = deepCopyAnnotations(originalAnnotations);

        // Then, adjust the annotations within the range and store the
        // annotations that are out of range and should be skipped.
        std::set<std::shared_ptr<draw::Annotation>> skipAnnotations;
        const auto& startTime = range.start_time();
        const auto& rangeDuration = range.duration();
        for (auto& annotation : annotations)
        {
            if (annotation->allFrames)
                continue;

            if (previous)
            {
                if (annotation->time <= range.start_time() ||
                    annotation->time >= insertTime)
                    skipAnnotations.insert(annotation);
                else if (range.contains(annotation->time))
                {
                    const auto offset = annotation->time - startTime;
                    annotation->time = insertTime + offset - rangeDuration;
                    skipAnnotations.insert(annotation);
                }
            }
            else
            {
                if (range.contains(annotation->time))
                {
                    const auto offset = annotation->time - startTime;
                    annotation->time = insertTime + offset;
                    skipAnnotations.insert(annotation);
                }
            }
        }

        // Finally, shift the other annotations.
        if (previous)
        {
            for (auto& annotation : annotations)
            {
                if (annotation->allFrames)
                    continue;

                if (skipAnnotations.find(annotation) != skipAnnotations.end())
                    continue;

                annotation->time -= rangeDuration;
            }
        }
        else
        {
            auto endTime = range.end_time_exclusive();
            for (auto& annotation : annotations)
            {
                if (annotation->allFrames)
                    continue;

                if (skipAnnotations.find(annotation) != skipAnnotations.end())
                    continue;

                if (annotation->time >= insertTime &&
                    annotation->time < endTime)
                {
                    annotation->time += range.duration();
                }
            }
        }

        player->setAllAnnotations(annotations);

        App::unsaved_edits = true;
        ui->uiMain->update_title_bar();
        
        view->redraw();
    }

    otio::SerializableObject::Retainer<otio::Timeline>
    createEmptyTimeline(ViewerUI* ui)
    {
        otio::SerializableObject::Retainer<otio::Timeline> otioTimeline =
            new otio::Timeline("EDL");

        auto videoTrack =
            new otio::Track("Video", std::nullopt, otio::Track::Kind::video);

        otio::ErrorStatus errorStatus;
        auto stack = new otio::Stack;
        stack->append_child(videoTrack, &errorStatus);
        if (is_error(errorStatus))
        {
            LOG_DEBUG("stack->append_child(videoTrack) failed with:");
            LOG_ERROR(errorStatus.full_description);
        }

        otioTimeline->set_tracks(stack);

        return otioTimeline;
    }

    void create_new_timeline_cb(ViewerUI* ui)
    {
        const std::string file = otioFilename(ui);

        otio::ErrorStatus errorStatus;
        auto timeline = createEmptyTimeline(ui);
        timeline->to_json_file(file, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            /* xgettext:c++-format */
            std::string error =
                string::Format(_("Could not save .otio file: {0}"))
                    .arg(errorStatus.full_description);
            tcp->unlock();
            throw std::runtime_error(error);
        }
        ui->app->open(file);
        tcp->unlock();
    }

    void save_timeline_to_disk(
        otio::Timeline* timeline, const std::string& otioFile,
        bool makeRelativePaths)
    {
        const std::string& s = timeline->to_json_string();
        const auto otioTimeline = createTimelineFromString(s);
        if (!otioTimeline)
            return;

        otio::SerializableObject::Retainer<otio::Timeline> out(otioTimeline);
        makePathsAbsolute(out, App::ui);
        auto stack = out->tracks();
        if (makeRelativePaths)
            makePathsRelative(stack, otioFile);
        otio::ErrorStatus errorStatus;
        out->to_json_file(otioFile, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            /* xgettext:c++-format */
            std::string err = string::Format(_("Error saving {0}. {1}"))
                                  .arg(otioFile)
                                  .arg(errorStatus.full_description);
            LOG_ERROR(err);
        }
        else
        {
            App::unsaved_edits = false;
        }
    }

    void save_timeline_to_disk(const std::string& otioFile)
    {

        ViewerUI* ui = App::ui;
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto model = ui->app->filesModel();
        auto Aitem = model->observeA()->get();
        if (!Aitem)
            return;

        file::Path path = Aitem->path;
        bool makeRelativePaths = false;
        if (file::isTemporaryEDL(path))
        {
            makeRelativePaths = true;
        }

        auto timeline = player->getTimeline();
        if (timeline->duration().value() <= 0.0)
        {
            LOG_ERROR(_("Empty EDL file.  Not saving."));
            return;
        }

        save_timeline_to_disk(timeline, otioFile, makeRelativePaths);
    }

    void save_timeline_to_disk_cb(Fl_Menu_* m, ViewerUI* ui)
    {

        auto otioFile = save_otio(nullptr);
        if (otioFile.empty())
            return;

        save_timeline_to_disk(otioFile);
    }

    void add_clip_to_new_timeline_cb(const int index, ViewerUI* ui)
    {
        create_new_timeline_cb(ui);
        add_clip_to_timeline_cb(index, ui);
    }

    void addTimelineToEDL(
        otio::Timeline* destTimeline, const otio::Timeline* sourceTimeline,
        const TimeRange& inOutRange, const TimeRange& timeRange)
    {
        otio::ErrorStatus errorStatus;
        auto globalStartTime =
            RationalTime(0.0, sourceTimeline->duration().rate());
        auto startTimeOpt = sourceTimeline->global_start_time();
        if (startTimeOpt.has_value())
            globalStartTime = startTimeOpt.value();

        const auto& sourceVideoTracks = sourceTimeline->video_tracks();
        const auto& sourceAudioTracks = sourceTimeline->audio_tracks();

        auto destStack = destTimeline->tracks();

        auto destVideoTracks = destTimeline->video_tracks();
        auto destStartTime = time::invalidTime;
        for (size_t i = 0; i < destVideoTracks.size(); ++i)
        {
            auto track = destVideoTracks[i];
            if (!track)
                continue; // should never happen
            auto duration = track->trimmed_range().duration();
            if (duration > destStartTime)
                destStartTime = duration;
        }

        if (destStartTime.strictly_equal(time::invalidTime))
        {
            destStartTime = RationalTime(0.0, 24.0);
        }

        // Then, append video tracks
        for (size_t i = 0; i < sourceVideoTracks.size(); ++i)
        {
            auto destTracks = destTimeline->video_tracks();
            otio::Track* track;
            if (i >= destTracks.size())
            {
                // Append a new video track
                track = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                destStack->append_child(track, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("destStack->append_child(track) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
            else
            {
                track = destTracks[i];
            }

            // If track duration is smaller than start time (ie. usually smaller
            // than all video tracks), add a gap filling the difference.
            auto destTrackDuration = track->duration();
            auto duration = destStartTime - destTrackDuration;
            if (duration.value() > 0.0)
            {
                auto gapRange =
                    TimeRange(RationalTime(0.0, duration.rate()), duration);
                auto gap = new otio::Gap(gapRange);
                track->append_child(gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("track->append_child(gap) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }

            // Now, append all video children.
            const auto& children = sourceVideoTracks[i]->children();
            double videoRate = track->duration().rate();
            if (sourceVideoTracks[i]->duration().rate() > videoRate)
                videoRate = sourceVideoTracks[i]->duration().rate();
            for (const auto& child : children)
            {
                auto clone = child->clone();
                auto item = dynamic_cast<Item*>(clone);
                if (item)
                {
                    auto srcItem = otio::dynamic_retainer_cast<Item>(child);
                    auto itemTrackRange =
                        srcItem->trimmed_range_in_parent().value();
                    auto itemRange = srcItem->trimmed_range();
                    if (itemRange.duration().rate() > videoRate)
                        videoRate = itemRange.duration().rate();
                    itemTrackRange = TimeRange(
                        itemTrackRange.start_time().rescaled_to(videoRate),
                        itemTrackRange.duration().rescaled_to(videoRate));
                    auto videoInOutRange = TimeRange(
                        inOutRange.start_time().rescaled_to(videoRate),
                        inOutRange.duration().rescaled_to(videoRate));
                    auto videoGlobalRange = TimeRange(
                        itemTrackRange.start_time().rescaled_to(videoRate) +
                            globalStartTime.rescaled_to(videoRate),
                        itemTrackRange.duration().rescaled_to(videoRate));

                    // file::PathOptions options;
                    // auto clip = otio::dynamic_retainer_cast<Clip>(child);
                    // file::Path path;
                    // if (clip)
                    //     path = timeline::getPath(clip->media_reference(),
                    //                              "",
                    //                              options);

                    // std::cerr << "---------------- " << path.get(-1,
                    // file::PathType::FileName)
                    //           << std::endl;
                    // std::cerr << "      itemRange=" << itemRange
                    //           << std::endl;
                    // std::cerr << " itemTrackRange=" << itemTrackRange
                    //           << std::endl;
                    // std::cerr << "     inOutRange=" << inOutRange <<
                    // std::endl; std::cerr << "    globalRange=" <<
                    // videoGlobalRange
                    //           << std::endl;
                    // std::cerr << "videoInOutRange=" << videoInOutRange <<
                    // std::endl;

                    if (videoInOutRange.intersects(videoGlobalRange))
                    {
                        // Set local clip start and duration.
                        auto startTime =
                            itemRange.start_time().rescaled_to(videoRate);
                        auto duration =
                            itemRange.duration().rescaled_to(videoRate);

                        // If user changed the in point, adjust start time.
                        if (videoInOutRange.start_time() >
                            timeRange.start_time().rescaled_to(videoRate))
                        {
                            startTime = timeline::toVideoMediaTime(
                                videoInOutRange.start_time(), videoGlobalRange,
                                itemRange, videoRate);
                            auto endTime = timeline::toVideoMediaTime(
                                videoInOutRange.end_time_exclusive(),
                                videoGlobalRange, itemRange, videoRate);

                            // Clamp the in / out points to item range
                            if (startTime < itemRange.start_time())
                                startTime = itemRange.start_time();
                            if (endTime > itemRange.end_time_exclusive())
                                endTime = itemRange.end_time_exclusive();
                            duration = endTime - startTime;
                        }

                        // If user changed the out point, adjust duration.
                        if (videoInOutRange.duration() < duration)
                        {
                            duration = videoInOutRange.duration();
                        }
                        const TimeRange clipRange(startTime, duration);
                        item->set_source_range(clipRange);
                        track->append_child(item, &errorStatus);
                        if (is_error(errorStatus))
                        {
                            LOG_DEBUG("track->append_child(item) failed with:");
                            LOG_ERROR(errorStatus.full_description);
                        }
                    }
                }
                else
                {
                    // auto transition = dynamic_cast<Transition*>(clone);
                    // if (transition)
                    // {
                    //     track->append_child(transition);
                    //     if (is_error(errorStatus))
                    //     {
                    //         LOG_DEBUG("track->append_child(transition) failed
                    //         with:"); LOG_ERROR(errorStatus.full_description);
                    //     }
                    // }
                    // else
                    // {
                    //     LOG_ERROR("Unknown child " << child->name());
                    // }
                }
            }
        }

        // Finally, append audio tracks
        if (sourceAudioTracks.size() == 0)
        {
            auto destTracks = destTimeline->audio_tracks();
            for (size_t i = 0; i < destTracks.size(); ++i)
            {
                otio::Track* track = destTracks[i];
                // If track duration is smaller than start time
                // (ie. usually smaller than video), add a gap filling the
                // difference.
                auto destTrackDuration = track->duration();
                auto duration = destStartTime - destTrackDuration;
                if (duration.value() > 0.0)
                {
                    auto gapRange =
                        TimeRange(RationalTime(0.0, duration.rate()), duration);
                    auto gap = new otio::Gap(gapRange);
                    track->append_child(gap, &errorStatus);
                    if (is_error(errorStatus))
                    {
                        LOG_DEBUG("track->append_child(gap) failed with:");
                        LOG_ERROR(errorStatus.full_description);
                    }
                }
            }
        }

        for (size_t i = 0; i < sourceAudioTracks.size(); ++i)
        {
            auto destTracks = destTimeline->audio_tracks();
            otio::Track* track;
            if (i >= destTracks.size())
            {
                // Append a new audio track
                track = new otio::Track(
                    "Audio", std::nullopt, otio::Track::Kind::audio);
                destStack->append_child(track);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("destStack->append_child(track) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }
            else
            {
                track = destTracks[i];
            }

            // If track duration is smaller than start time (ie. usually smaller
            // than video), add a gap filling the difference.
            auto destTrackDuration = track->duration();
            auto duration = destStartTime - destTrackDuration;
            if (duration.value() > 0.0)
            {
                auto gapRange =
                    TimeRange(RationalTime(0.0, duration.rate()), duration);
                auto gap = new otio::Gap(gapRange);
                track->append_child(gap, &errorStatus);
                if (is_error(errorStatus))
                {
                    LOG_DEBUG("track->append_child(gap) failed with:");
                    LOG_ERROR(errorStatus.full_description);
                }
            }

            // Now, append all audio children.
            const auto& children = sourceAudioTracks[i]->children();
            double sampleRate = track->duration().rate();
            if (sourceAudioTracks[i]->duration().rate() > sampleRate)
                sampleRate = sourceAudioTracks[i]->duration().rate();
            for (const auto& child : children)
            {
                auto clone = child->clone();
                auto item = dynamic_cast<Item*>(clone);
                if (item)
                {
                    auto srcItem = otio::dynamic_retainer_cast<Item>(child);
                    auto itemTrackRange =
                        srcItem->trimmed_range_in_parent().value();
                    auto itemRange = srcItem->trimmed_range();
                    if (itemRange.duration().rate() > sampleRate)
                        sampleRate = itemRange.duration().rate();
                    auto audioInOutRange = TimeRange(
                        inOutRange.start_time().rescaled_to(sampleRate),
                        inOutRange.duration().rescaled_to(sampleRate));
                    auto audioGlobalRange = TimeRange(
                        itemTrackRange.start_time().rescaled_to(sampleRate) +
                            globalStartTime.rescaled_to(sampleRate),
                        itemTrackRange.duration().rescaled_to(sampleRate));

                    if (audioInOutRange.intersects(audioGlobalRange))
                    {
                        // Set local clip start and duration.
                        auto startTime =
                            itemRange.start_time().rescaled_to(sampleRate);
                        auto duration =
                            itemRange.duration().rescaled_to(sampleRate);

                        // If user changed the in point, adjust start time.
                        if (audioInOutRange.start_time() >
                            timeRange.start_time().rescaled_to(sampleRate))
                        {
                            // Calculate the in / out points in clip space.
                            // The use of toVideoMediaTime instead of
                            // toAudioMediaTime is not a typo.
                            startTime = timeline::toVideoMediaTime(
                                audioInOutRange.start_time(), audioGlobalRange,
                                itemRange, sampleRate);
                            auto endTime = timeline::toVideoMediaTime(
                                audioInOutRange.end_time_exclusive(),
                                audioGlobalRange, itemRange, sampleRate);

                            // Clamp the in / out points to item range
                            if (startTime < itemRange.start_time())
                                startTime = itemRange.start_time();
                            if (endTime > itemRange.end_time_exclusive())
                                endTime = itemRange.end_time_exclusive();
                            duration = endTime - startTime;
                        }

                        // If user changed the out point, adjust duration.
                        if (audioInOutRange.duration() < duration)
                        {
                            duration = audioInOutRange.duration();
                        }
                        const TimeRange clipRange(startTime, duration);
                        item->set_source_range(clipRange);
                        track->append_child(item, &errorStatus);
                        if (is_error(errorStatus))
                        {
                            LOG_DEBUG("track->append_child(item) failed with:");
                            LOG_ERROR(errorStatus.full_description);
                        }
                    }
                }
                else
                {
                    // auto transition = dynamic_cast<Transition*>(clone);
                    // if (transition)
                    // {
                    //     track->append_child(transition, &errorStatus);
                    //     if (is_error(errorStatus))
                    //     {
                    //         LOG_DEBUG("track->append_child(transition) failed
                    //         with:"); LOG_ERROR(errorStatus.full_description);
                    //     }
                    // }
                    // selse
                    // {
                    //     LOG_ERROR("Unknown child " << child->name());
                    // }
                }
            }
        }
    }

    void addClipToTimeline(
        const int sourceIndex, const int destIndex,
        otio::Timeline* destTimeline, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        auto numFiles = model->observeFiles()->getSize();

        if (sourceIndex < 0 || sourceIndex >= numFiles)
        {
            LOG_ERROR(
                "Source index out of range" << sourceIndex
                                            << " max=" << numFiles);
            return;
        }
        if (destIndex < 0 || destIndex >= numFiles)
        {
            LOG_ERROR(
                "Destination index out of range" << destIndex
                                                 << " max=" << numFiles);
            return;
        }

        model->setA(sourceIndex);
        auto sourceItem = model->observeA()->get();
        if (!sourceItem)
            return;

        auto inOutRange = sourceItem->inOutRange;
        auto timeRange = sourceItem->timeRange;

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();
        if (!timeline)
        {
            LOG_ERROR("No timeline in player");
            return;
        }

        // Make a copy of the timeline, so we don't modify the original in
        // place.
        const std::string& s = timeline->to_json_string();
        const auto otioTimeline = createTimelineFromString(s);
        if (!otioTimeline)
            return;

        otio::SerializableObject::Retainer<otio::Timeline> sourceTimeline(
            otioTimeline);

        makePathsAbsolute(sourceTimeline, ui);

        model->setA(destIndex);

        addTimelineToEDL(destTimeline, sourceTimeline, inOutRange, timeRange);
    }

    void add_clip_to_timeline_cb(const int index, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto destTimeline = player->getTimeline();
        if (!destTimeline)
            return;

        auto time = getTime(player);

        if (!verifySourceItem(index, ui))
            return;

        auto model = ui->app->filesModel();

        const auto sourceItems = model->observeFiles()->get();

        auto sourceItem = sourceItems[index];

        auto destIndex = model->observeAIndex()->get();
        auto destItem = model->observeA()->get();
        if (!destItem)
        {
            LOG_ERROR(_("Destination file is invalid."));
            return;
        }

        if (!file::isTemporaryEDL(destItem->path))
        {
            LOG_ERROR(_("You can only add clips to an .otio EDL playlist."));
            return;
        }

        file::Path path = sourceItem->path;
        file::Path audioPath =
            sourceItem->audioPath.isEmpty() ? path : sourceItem->audioPath;

        if (!path.isAbsolute())
        {
            char currentDir[4096];
            if (fl_getcwd(currentDir, 4096) == nullptr)
            {
                LOG_ERROR(_("Could not get current path."));
                return;
            }

            std::string fullpath = currentDir;
            fullpath += '/' + path.get();
            path = file::Path(fullpath);
        }

        tcp->pushMessage("Add Clip to Timeline", index);
        tcp->lock();

        try
        {

            edit_store_undo(player, ui);

            const auto timelineDuration = destTimeline->duration();
            auto stack = destTimeline->tracks();
            auto tracks = stack->children();

            double videoRate = 0.F;
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video)
                {
                    if (track->trimmed_range().duration().rate() > videoRate)
                        videoRate = track->trimmed_range().duration().rate();
                }
            }

            auto annotations = addAnnotations(
                timelineDuration.rescaled_to(videoRate),
                player->getAllAnnotations(), sourceItem->inOutRange,
                sourceItem->annotations);

            bool emptyTracks = hasEmptyTracks(destTimeline->tracks());

            addClipToTimeline(index, destIndex, destTimeline, ui);

            //
            // Sanity check on video and sample rate.
            //
            videoRate = 0.F;
            double sampleRate = 0.F;
            otime::TimeRange timeRange;
            sanitizeVideoAndAudioRates(
                destTimeline, timeRange, videoRate, sampleRate);

            toOtioFile(destTimeline, ui);

            destItem->timeRange = timeRange;
            destItem->inOutRange = timeRange;
            destItem->speed = videoRate;

            if (emptyTracks)
            {
                model->setA(index);
                model->setA(destIndex);
            }

            ui->uiView->valid(0); // needed
            ui->uiView->redraw(); // needed

            player = ui->uiView->getTimelinePlayer();
            if (!player)
            {
                LOG_ERROR("No destination player");
                return;
            }

            auto time = timelineDuration.rescaled_to(videoRate) +
                        sourceItem->currentTime.rescaled_to(videoRate);
            updateTimeline(destTimeline, time, ui);

            player->setAllAnnotations(annotations);
            ui->uiTimeline->frameView();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        tcp->unlock();
    }

    void edit_move_clip_annotations(
        const std::vector<tl::timeline::MoveData>& moves, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();
        if (!timeline)
            return;
        
        edit_store_undo(player, ui);

        // If an undo only operation, return immediately.
        if (moves.size() == 1 && moves[0].type == tl::timeline::MoveType::UndoOnly)
            return;

        const auto& startTimeOpt = timeline->global_start_time();
        otime::RationalTime startTime(0.0, timeline->duration().rate());
        if (startTimeOpt.has_value())
        {
            startTime = startTimeOpt.value();
            offsetAnnotations(
                startTime, -startTime, player->getAllAnnotations());
        }

        const auto& stack = timeline->tracks();
        const auto& tracks = stack->children();

        
        for (const auto& move : moves)
        {
            if (move.type == tl::timeline::MoveType::Transition)
            {
                if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        tracks[move.fromTrack]))
                {
                    auto child = track->children()[move.fromOtioIndex];
                    auto transition = otio::dynamic_retainer_cast<otio::Transition>(child);
                    if (!transition)
                    {
                        LOG_ERROR("Invalid otio transition index");
                        continue;
                    }
                    if (move.in_offset.value() < 1.F || move.out_offset.value() < 1.F)
                    {
                        std::cerr << "ORIG in_offset=" << transition->in_offset()
                                  << std::endl;
                        std::cerr << "ORIG out_offset=" << transition->out_offset()
                                  << std::endl;
                        std::cerr << "NEW  in_offset=" << move.in_offset
                                  << std::endl;
                        std::cerr << "NEW  out_offset=" << move.out_offset
                                  << std::endl;
                        LOG_ERROR("Move offsets are invalid - Ignoring");
                        continue;
                    }
                    transition->set_in_offset(move.in_offset);
                    transition->set_out_offset(move.out_offset);
                }
                continue;
            }
            
            if (move.toIndex < 0 || move.toTrack < 0 ||
                move.toTrack >= tracks.size())
            {
                LOG_ERROR("Invalid TO track or index");
                continue;
            }
            if (move.fromIndex < 0 || move.fromTrack < 0 ||
                move.fromTrack >= tracks.size())
            {
                LOG_ERROR("Invalid FROM track or index");
                continue;
            }

            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.fromTrack]))
            {
                if (track->kind() != otio::Track::Kind::video)
                    continue;
            }

            int toIndex = move.toOtioIndex;
            if (move.fromTrack == move.toTrack && move.fromIndex < move.toIndex)
            {
                --toIndex;
            }

            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.fromTrack]))
            {
                auto child = track->children()[move.fromOtioIndex];
                auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                if (!item)
                {
                    LOG_ERROR(
                        "From track="
                        << move.fromTrack << " item=" << move.fromIndex
                        << " otio=" << move.fromOtioIndex
                        << " name=" << child->name() << " not an item ");
                    continue;
                }

                auto rate = track->trimmed_range().duration().rate();

                auto oldRange = item->trimmed_range_in_parent().value();

                oldRange = otime::TimeRange(
                    oldRange.start_time().rescaled_to(rate),
                    oldRange.duration().rescaled_to(rate));

                if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        tracks[move.toTrack]))
                {
                    auto child = track->children()[toIndex];
                    auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                    if (!item)
                    {
                        LOG_ERROR(
                            "To track=" << move.toTrack << " item=" << toIndex
                                        << " otio=" << move.toOtioIndex
                                        << " name=" << child->name()
                                        << " not an item");
                        continue;
                    }

                    auto insertRange = item->trimmed_range_in_parent().value();

                    otime::RationalTime insertTime;
                    bool previous = toIndex > move.fromIndex;
                    if (previous)
                    {
                        insertTime = insertRange.end_time_exclusive();
                    }
                    else
                    {
                        insertTime = insertRange.start_time();
                    }

                    insertTime = insertTime.rescaled_to(rate);

                    //
                    // Shift annotations
                    //
                    shiftAnnotations(oldRange, insertTime, previous, ui);
                }
            }
        }

        // Finally, remove transitions from both from and to clips
        otio::ErrorStatus errorStatus;
        for (const auto& move : moves)
        {
            if (move.type == tl::timeline::MoveType::Transition)
                continue;
            std::vector<int> fromOtioIndexes;
            std::vector<int> toOtioIndexes;
            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.fromTrack]))
            {
                auto child = track->children()[move.fromOtioIndex];

                auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                if (item)
                {
                    const auto neighbors =
                        track->neighbors_of(item, &errorStatus);
                    if (auto transition = dynamic_cast<otio::Transition*>(
                            neighbors.second.value))
                    {
                        const int index = track->index_of_child(transition);
                        fromOtioIndexes.push_back(index);
                    }

                    if (auto transition = dynamic_cast<otio::Transition*>(
                            neighbors.first.value))
                    {
                        const int index = track->index_of_child(transition);
                        fromOtioIndexes.push_back(index);
                    }
                }

                std::sort(
                    fromOtioIndexes.begin(), fromOtioIndexes.end(),
                    std::greater<int>());
                for (const auto index : fromOtioIndexes)
                {
                    track->remove_child(index);
                }
            }
            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.toTrack]))
            {
                if (move.toOtioIndex >= track->children().size())
                    continue;

                auto child = track->children()[move.toOtioIndex];

                auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                if (!item)
                    continue;

                const auto neighbors = track->neighbors_of(item, &errorStatus);
                if (auto transition =
                        dynamic_cast<otio::Transition*>(neighbors.second.value))
                {
                    const int index = track->index_of_child(transition);
                    toOtioIndexes.push_back(index);
                }

                if (auto transition =
                        dynamic_cast<otio::Transition*>(neighbors.first.value))
                {
                    const int index = track->index_of_child(transition);
                    toOtioIndexes.push_back(index);
                }

                std::sort(
                    toOtioIndexes.begin(), toOtioIndexes.end(),
                    std::greater<int>());
                for (const auto index : toOtioIndexes)
                {
                    track->remove_child(index);
                }
            }
        }

        // Refresh edit mode in case there are no transition tracks.
        set_edit_mode_cb(editMode, ui);
    }

    bool replaceClipPath(tl::file::Path clipPath, ViewerUI* ui)
    {
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return false;

        auto timeline = player->getTimeline();
        if (!timeline)
        {
            LOG_ERROR("No timeline in player");
            return false;
        }

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        otio::ErrorStatus errorStatus;
        otio::Clip* clip = nullptr;
        int clipIndex = -1;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            // Find first video track
            if (track->kind() != otio::Track::Kind::video)
                continue;

            clip = otio::dynamic_retainer_cast<Clip>(
                track->child_at_time(time, &errorStatus));
            if (!clip)
                continue;

            clipIndex = track->index_of_child(clip);
            break;
        }

        if (clipIndex < 0 || !clip)
            return false;

        auto media = clip->media_reference();
        if (auto ref = dynamic_cast<otio::ExternalReference*>(media))
        {
            ref->set_target_url(clipPath.get());
        }
        else if (auto ref = dynamic_cast<otio::ImageSequenceReference*>(media))
        {
            ref->set_target_url_base(clipPath.getDirectory());
            ref->set_name_prefix(clipPath.getBaseName());
        }
        else
        {
            LOG_ERROR(_("Unknown media reference"));
            return false;
        }

        makePathsAbsolute(timeline, ui);

        toOtioFile(timeline, ui);

        refresh_file_cache_cb(nullptr, ui);

        return true;
    }

    //! Get Active Tracks.
    bool getActiveTracks(
        std::vector<std::string>& tracks, std::vector<bool>& tracksActive,
        ViewerUI* ui)
    {
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return false;

        auto timeline = player->getTimeline();
        if (!timeline)
        {
            LOG_ERROR("No timeline in player");
            return false;
        }

        auto compositions = getTracks(player);

        otio::ErrorStatus errorStatus;
        otio::Clip* clip = nullptr;
        unsigned trackIndex = 0;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            ++trackIndex;

            /* xgettext:c++-format */
            const std::string name = tl::string::Format(_("Track #{0} - {1}"))
                                         .arg(trackIndex)
                                         .arg(track->name());

            bool active = track->enabled();

            tracks.push_back(name);
            tracksActive.push_back(active);
        }

        return true;
    }

    bool toggleTrack(unsigned trackIndex, ViewerUI* ui)
    {
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return false;

        auto timeline = player->getTimeline();
        if (!timeline)
        {
            LOG_ERROR("No timeline in player");
            return false;
        }

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        std::vector<int> audioMutedTracks;
        otio::ErrorStatus errorStatus;
        unsigned index = 0;
        for (auto composition : compositions)
        {
            auto track = dynamic_cast<otio::Track*>(composition);
            if (!track)
                continue;

            if (trackIndex != index)
            {
                ++index;
                continue;
            }

            bool enabled = track->enabled();
            enabled ^= true;
            track->set_enabled(enabled);

            if (track->kind() == otio::Track::Kind::audio)
            {
                audioMutedTracks.push_back(!enabled);
            }
            break;
        }

        player->player()->setChannelMute(audioMutedTracks);

        makePathsAbsolute(timeline, ui);

        updateTimeline(timeline, time, ui);
        toOtioFile(timeline, ui);

        refresh_file_cache_cb(nullptr, ui);

        return true;
    }

    /// \@todo: REFACTOR THIS PLEASE
    EditMode editMode = EditMode::kTimeline;
    EditMode previousEditMode = EditMode::kTimeline;
    int      editModeH = 30;
    const int kMinEditModeH = 30;

    void save_edit_mode_state(ViewerUI* ui)
    {
        int H = ui->uiTimelineGroup->h();

        if (!ui->uiBottomBar->visible())
        {
            editMode = EditMode::kNone;
        }
        else if (H > kMinEditModeH)
        {
            editMode = EditMode::kSaved;
            editModeH = H;
        }
        else
        {
            editMode = EditMode::kTimeline;
            editModeH = kMinEditModeH;
        }
    }

    int calculate_edit_viewport_size(ViewerUI* ui)
    { //
#ifdef OPENGL_BACKEND
        const timelineui::DisplayOptions& displayOptions =
            ui->uiTimeline->getDisplayOptions();
#endif

#ifdef VULKAN_BACKEND
        const timelineui_vk::DisplayOptions& displayOptions =
            ui->uiTimeline->getDisplayOptions();
#endif

#if 0
        std::cerr << " trackInfo=" << displayOptions.trackInfo << std::endl;
        std::cerr << "  clipInfo=" << displayOptions.clipInfo << std::endl;
        std::cerr << "thumbnails=" << displayOptions.thumbnails << std::endl;
        std::cerr << "thumbnailsHeight="
                  << displayOptions.thumbnailHeight << std::endl;
        std::cerr << "waveformHeight=" << displayOptions.waveformHeight << std::endl;
        std::cerr << "      fontSize=" << displayOptions.fontSize << std::endl;
        std::cerr << " clipRectScale=" << displayOptions.clipRectScale << std::endl;
        std::cerr << "   transitions=" << displayOptions.transitions << std::endl;
        std::cerr << "       markers=" << displayOptions.markers << std::endl;
#endif

        // This specifies whether to show Video Only or Video and Audio.
        const int editView = ui->uiPrefs->uiPrefsEditView->value();

        // Timeline's pixels_per_unit would get reset to 1 when timeline was
        // hidden.
        // const float pixels_unit = ui->uiTimeline->pixels_per_unit();
        const float pixels_unit = ui->uiView->pixels_per_unit();

        // Some constants, as Darby does not yet expose this in tlRender.
        const int kMargin = 4;
        const int kTrackInfoHeight = 20 + kMargin;
        const int kClipInfoHeight = 16 + kMargin;
        const int kTransitionsHeight = 30;
        const int kAudioGapOnlyHeight = 20;
        const int kMarkerHeight = 24;

        const int kVideoHeight = displayOptions.thumbnailHeight;
        const int kAudioHeight = displayOptions.waveformHeight + kMargin;

        int H = kMinEditModeH; // timeline height
        if (editMode == EditMode::kTimeline)
            return H;
        else if (editMode == EditMode::kSaved)
            return editModeH;

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return H;

        // Accumulated variables counting total height of each track.
        int videoHeight = 0;
        int audioHeight = 0;
        int markersHeight = 0;
        int transitionsHeight = 0;

        auto timeline = player->getTimeline();
        if (!timeline)
            return H;

        // Check first if the timeline is an audio only timeline.
        bool audioOnly = true;
        for (const auto& child : timeline->tracks()->children())
        {
            if (const auto* track = dynamic_cast<otio::Track*>(child.value))
            {
                if (otio::Track::Kind::video == track->kind())
                {
                    audioOnly = false;
                    break;
                }
            }
        }

        for (const auto& child : timeline->tracks()->children())
        {
            if (const auto* track = dynamic_cast<otio::Track*>(child.value))
            {
                if (!track->enabled())
                    continue;

                bool visibleTrack = false;
                if (otio::Track::Kind::video == track->kind())
                {
                    if (displayOptions.trackInfo)
                        videoHeight += kTrackInfoHeight;
                    if (displayOptions.clipInfo)
                        videoHeight += kClipInfoHeight;
                    if (displayOptions.thumbnails)
                        videoHeight += kVideoHeight / pixels_unit;
                    visibleTrack = true;
                }
                else if (
                    otio::Track::Kind::audio == track->kind() &&
                    (editView >= 1 || audioOnly))
                {
                    if (track->children().size() > 0)
                    {
                        if (displayOptions.trackInfo)
                            audioHeight += kTrackInfoHeight;
                        if (displayOptions.clipInfo)
                            audioHeight += kClipInfoHeight;
                        if (displayOptions.thumbnails)
                        {
                            bool hasWaveform = false;
                            for (const auto& trackChild : track->children())
                            {
                                if (const auto& clip =
                                        otio::dynamic_retainer_cast<Clip>(
                                            trackChild))
                                {
                                    hasWaveform = true;
                                    break;
                                }
                            }
                            if (hasWaveform)
                                audioHeight += kAudioHeight / pixels_unit;
                            else
                                audioHeight +=
                                    kAudioGapOnlyHeight / pixels_unit;
                        }

                        visibleTrack = true;
                    }
                }
                // Handle Markers
                if (displayOptions.markers && visibleTrack)
                {
                    int markerSizeForTrack = 0;
                    for (const auto& child : track->children())
                    {
                        auto item = otio::dynamic_retainer_cast<Item>(child);
                        if (!item)
                            continue;

                        int markerSizeForItem = 0;
                        for (const auto& marker : item->markers())
                        {
                            markerSizeForItem += kMarkerHeight;
                        }
                        if (markerSizeForItem > markerSizeForTrack)
                            markerSizeForTrack = markerSizeForItem;
                    }
                    markersHeight += markerSizeForTrack;
                }
                // Handle transitions
                if (displayOptions.transitions && visibleTrack)
                {
                    bool found = false;
                    for (const auto& child : track->children())
                    {
                        if (const auto& transition =
                                dynamic_cast<otio::Transition*>(child.value))
                        {
                            bool visibleTrack = false;
                            if (otio::Track::Kind::video == track->kind())
                            {
                                visibleTrack = true;
                            }
                            else if (
                                otio::Track::Kind::audio == track->kind() &&
                                (editView >= 1 || audioOnly))
                            {
                                if (track->children().size() > 0)
                                {
                                    visibleTrack = true;
                                }
                            }
                            if (visibleTrack)
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found)
                    {
                        transitionsHeight += kTransitionsHeight;
                    }
                }
            }
        }

        // Now add up all heights divided by the pixels unit.
        H += videoHeight + audioHeight + markersHeight + transitionsHeight;

#if 0
        std::cerr << "    kMargin=" << kMargin << std::endl;
        std::cerr << "videoHeight=" << videoHeight << std::endl;
        std::cerr << "audioHeight=" << audioHeight << std::endl;
        std::cerr << "transitionsHeight=" << transitionsHeight << std::endl;
        std::cerr << "markersHeight=" << markersHeight << std::endl;
        std::cerr << "FINAL H=" << H << std::endl;
#endif

        // Sanity check... make sure we don't go bigger than the max.
        const Fl_Tile* tile = ui->uiTileGroup;
        const int tileH = tile->h();
        // Shift the view up to see the video thumbnails and audio waveforms.
        // We need to substrac dragbar and timeline to leave room.
        const int maxTileHeight = tileH - kMinEditModeH;

        if (H >= maxTileHeight)
        {
            H = maxTileHeight;
        }
        return H;
    }

    void set_edit_button(EditMode mode, ViewerUI* ui)
    {
        const int kDragBarHeight = 8;
        
        Fl_Button* b = ui->uiEdit;

        bool active = (mode == EditMode::kFull || mode == EditMode::kSaved);

        if (active)
        {
            int savedH = calculate_edit_viewport_size(ui);
            int H = ui->uiTimelineGroup->h();
            if (H == kMinEditModeH || !ui->uiTimelineGroup->visible())
                active = false;
        }

        b->value(active);
        if (active)
        {
            b->labelcolor(fl_rgb_color(255, 255, 255));
        }
        else
        {
            b->labelcolor(FL_FOREGROUND_COLOR);
        }
        b->redraw();
        
        if (active && ui->uiTimeline->isEditable())
        {
            ui->uiEditGroup->show();
            ui->uiActionGroup->hide();
        }
        else
        {
            ui->uiEditGroup->hide();
            ui->uiActionGroup->show();
        }
    }

    void set_edit_mode_cb(EditMode mode, ViewerUI* ui)
    {
        // Store previous editmode
        previousEditMode = editMode;
        if (editMode == previousEditMode && editMode == EditMode::kNone)
            previousEditMode = EditMode::kTimeline;

        // This is the main tile position.
        Fl_Tile* tileGroup = ui->uiTileGroup;
        int tileGroupY = tileGroup->y();
        int tileGroupH = tileGroup->h();

        // This is the timeline group within the tile.
        Fl_Group* TimelineGroup = ui->uiTimelineGroup;
        int oldY = TimelineGroup->y();
        int newY = oldY;

        // This is the main viewport with the action items.
        Fl_Flex* viewGroup = ui->uiViewGroup;
        
        // Set some defaults
        int H = kMinEditModeH;            // min. timeline height
        int viewGroupH = tileGroupH - H;  // max. viewport with timeline
        
        auto player = ui->uiView->getTimelinePlayer();
        if (mode == EditMode::kFull && player)
        {
            // If full editing, save the edit mode
            H = calculate_edit_viewport_size(ui);
            viewGroupH = tileGroupH - H;
            newY = tileGroupY + viewGroupH;
            editMode = mode;
            editModeH = H;
            ui->uiBottomBar->show();
        }
        else if (mode == EditMode::kSaved)
        {
            H = editModeH;
            viewGroupH = tileGroupH - H;
            newY = tileGroupY + viewGroupH;
            editMode = mode;
            ui->uiBottomBar->show();
        }
        else if (mode == EditMode::kNone)
        {
            // \@note:  When going to presentation mode, we set the timeline
            //          mode to None.  We calculate the viewGroupH (height),
            //          but we must not calculate H as 0, as that would collapse
            //          the timeline and can crash the X11 server.
            viewGroupH = tileGroupH;
            H = kMinEditModeH; // timeline height
            newY = oldY;
            editMode = mode;
            ui->uiBottomBar->hide();
        }
        else
        {
            // Timeline mode
            H = kMinEditModeH; // timeline height
            viewGroupH = tileGroupH - H;
            newY = tileGroupY + viewGroupH;
            editMode = mode;
            ui->uiBottomBar->show();
        }


#if 0
        std::cerr << "1 TimelineGroup->visible()="
                  << TimelineGroup->visible() << std::endl;
        std::cerr << "1    editMode=" << editMode << std::endl;
        std::cerr << "1  tileGroupY=" << tileGroupY << std::endl;
        std::cerr << "1        oldY=" << oldY - tileGroupY << std::endl;
        std::cerr << "1        newY=" << newY - tileGroupY << std::endl;
        std::cerr << "1  tileGroupH=" << tileGroupH << std::endl;
        std::cerr << "1  viewGroupH=" << viewGroupH << std::endl;
        std::cerr << "1   timelineH=" << H << std::endl;
#endif

        // \@bug:
        // This mess is to work around macOS issues.  Unhiding TimelineGroup
        // should be enough to also unhide the timeline, but it seemed not
        // to work on macOS.
#if 0
        if (ui->uiBottomBar->visible())
        {
            TimelineGroup->show();
        }
        else if (ui->uiMain->visible())
        {
            TimelineGroup->hide();
        }
#else
        if (ui->uiMain->visible())
        {
            if (ui->uiBottomBar->visible())
            {
                if (!ui->uiTimelineGroup->visible())
                {
                    TimelineGroup->show();
                }
                if (!ui->uiTimeline->visible())
                {
                    ui->uiTimeline->show();
                }
            }
            else
            {
                if (ui->uiTimeline->visible())
                {
                    ui->uiTimeline->hide();
                }
                if (ui->uiTimelineGroup->visible())
                    TimelineGroup->hide();
            }
        }
#endif
        
        // \@note: We do a resize instead of a move_intersection as:
        //         it is faster and we must avoid collapsing the timeline group
        //         to 0 when going into presentation mode (we internally keep
        //         it as kMinEditModeH.
        viewGroup->resize(
            viewGroup->x(), viewGroup->y(), viewGroup->w(), viewGroupH);
        TimelineGroup->resize(TimelineGroup->x(), newY, TimelineGroup->w(), H);

#if 0
        std::cerr << "3 TimelineGroup->visible()="
                  << TimelineGroup->visible() << std::endl;
        std::cerr << "3 TimelineGroup->x()="
                  << TimelineGroup->x() << std::endl;
        std::cerr << "3 TimelineGroup->y()="
                  << TimelineGroup->y() << std::endl;
        std::cerr << "3 TimelineGroup->w()="
                  << TimelineGroup->w() << std::endl;
        std::cerr << "3 TimelineGroup->h()="
                  << TimelineGroup->h() << std::endl;
#endif
        
        viewGroup->layout();

        tileGroup->init_sizes();

        ui->uiRegion->layout();

#if 0
        std::cerr << "6 TimelineGroup->visible()="
                  << TimelineGroup->visible() << std::endl;
        std::cerr << "6 TimelineGroup->x()="
                  << TimelineGroup->x() << std::endl;
        std::cerr << "6 TimelineGroup->y()="
                  << TimelineGroup->y() << std::endl;
        std::cerr << "6 TimelineGroup->w()="
                  << TimelineGroup->w() << std::endl;
        std::cerr << "6 TimelineGroup->h()="
                  << TimelineGroup->h() << std::endl;
#endif

        ui->uiView->valid(0);
        ui->uiView->refresh();
        if (ui->uiTimeline->shown())
        {
            ui->uiTimeline->valid(0);
            ui->uiTimeline->refresh();
        }
        if (ui->uiSecondary && ui->uiSecondary->viewport())
        {
            auto view = ui->uiSecondary->viewport();
            view->valid(0);
            view->refresh();
        }

        // This is needed as XWayland and Wayland would leave traces of the
        // toolbar icons.
        if (desktop::XWayland() || desktop::Wayland())
            TimelineGroup->redraw();

        // Change the edit button status
        set_edit_button(editMode, ui);

        // EditMode::kNone is used when we go to presentation mode.
        if (mode != EditMode::kNone)
        {
            Message msg;
            msg["command"] = "setEditMode";
            msg["value"] = mode;
            msg["height"] = H;
            tcp->pushMessage(msg);
        }
    }

} // namespace mrv
