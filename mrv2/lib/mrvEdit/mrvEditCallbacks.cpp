
// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <set>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

#include <FL/fl_utf8.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/editAlgorithm.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>

#include <tlCore/Path.h>
#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <tlIO/System.h>

#include <tlTimeline/Util.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHome.h"

#include "mrvDraw/Annotation.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvMoveData.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

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

        std::vector<Composition*>
        getTracks(TimelinePlayer* player, const RationalTime& time)
        {
            std::vector<Composition*> out;
            auto timeline = player->getTimeline();

            otio::ErrorStatus errorStatus;
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

        RationalTime getTime(TimelinePlayer* player)
        {
            const auto& timeline_range = player->timeRange();
            const auto& startTime = timeline_range.start_time();
            const auto time = player->currentTime() - startTime;
            return time;
        }

        // @todo: darby needs to provide this from timelineUI
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

        bool verifySourceItem(int index, ViewerUI* ui)
        {
            auto model = ui->app->filesModel();
            const auto& sourceItems = model->observeFiles()->get();
            if (index < 0 || index >= sourceItems.size())
            {
                LOG_ERROR(_("Invalid index for add clip to timeline."));
                return false;
            }

            auto sourceItem = sourceItems[index];
            if (sourceItem->path.getExtension() == ".otio")
            {
                LOG_ERROR(_("Currently, you cannot add an .otio file to an "
                            "EDL playlist."));
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

            auto clip_range = item->trimmed_range();
            auto track_range = item->trimmed_range_in_parent(&errorStatus);
            if (is_error(errorStatus))
            {
                LOG_ERROR(item->name() << " is not attached to a track.");
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

            if (isTemporaryEDL(path))
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

        std::string otioFilename(ViewerUI* ui)
        {
            auto out = _otioFilename(ui);
            ++otioIndex;
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
            if (isTemporaryEDL(path))
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
                refresh_file_cache_cb(nullptr, ui);
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
                        auto clip = otio::dynamic_retainer_cast<Item>(child);
                        if (!clip)
                            continue;
                        if (videoRate > 0)
                        {
                            auto range = clip->trimmed_range();
                            auto start = time::round(
                                range.start_time().rescaled_to(videoRate));
                            auto duration = time::round(
                                range.duration().rescaled_to(videoRate));
                            range = TimeRange(start, duration);
                            clip->set_source_range(range);
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
                        auto clip = otio::dynamic_retainer_cast<Item>(child);
                        if (!clip)
                            continue;
                        if (sampleRate > 0)
                        {
                            auto range = clip->trimmed_range();
                            auto start = time::round(
                                range.start_time().rescaled_to(sampleRate));
                            auto duration = time::round(
                                range.duration().rescaled_to(sampleRate));
                            range = TimeRange(start, duration);
                            duration = duration.rescaled_to(videoRate);
                            clip->set_source_range(range);
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
            if (!isTemporaryEDL(path))
                return _otioFilename(ui);

            return path.get();
        }

    } // anonymous namespace

    file::Path getRelativePath(const file::Path& path, const fs::path& fileName)
    {
        fs::path filePath = path.get();
        // Make file absolue, then remove it, leaving directory
        fs::path directory = fs::absolute(fileName).parent_path();
        fs::path relative = fs::relative(filePath, directory);
        std::string file = relative.generic_string();
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
    }

    void edit_clear_redo(ViewerUI* ui)
    {
        redoBuffer.clear();
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
    }

    void edit_remove_undo()
    {
        undoBuffer.pop_back();
    }

    void edit_copy_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        player->stop();

        auto timeline = player->getTimeline();
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

            // Adjust time by almsot half a frame to avoid rounding issues in
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
        auto stack = timeline->tracks();
        if (!stack)
            return;

        auto tracks = stack->children();

        edit_store_undo(player, ui);

        double videoRate = 0.F, sampleRate = 0.F;
        ;
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

        tcp->pushMessage("Edit/Frame/Paste", time);
    }

    void edit_insert_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        player->stop();

        auto timeline = player->getTimeline();
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
        const auto& tracks = getTracks(player, time);

        auto timeline = player->getTimeline();

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

        tcp->pushMessage("Edit/Slice", time);
    }

    void edit_remove_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player, time);

        auto timeline = player->getTimeline();
        auto stack = timeline->tracks();

        edit_store_undo(player, ui);

        const auto half_frame = RationalTime(0.4, time.rate());

        for (auto track : tracks)
        {
            // Adjust time by almsot half a frame to avoid rounding issues in
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

        tcp->pushMessage("Edit/Remove", time);
    }

    void edit_trim_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_slip_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_slide_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_ripple_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_roll_cb(Fl_Menu_* m, ViewerUI* ui) {}

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

        if (!switchToEDL(buffer.fileName, ui))
            return;

        // We must get player again, as we might have changed clips.
        player = ui->uiView->getTimelinePlayer();
        edit_store_redo(player, ui);

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(buffer.json)));

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

        if (!switchToEDL(buffer.fileName, ui))
            return;

        // We must get player again, as we might have changed clips.
        player = ui->uiView->getTimelinePlayer();
        edit_store_undo(player, ui);

        auto stack = player->getTimeline()->tracks();
        const bool refreshCache = hasEmptyTracks(stack);

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(buffer.json)));

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

        // Then, adjust the annotations within the range.
        std::set<std::shared_ptr<draw::Annotation>> skipAnnotations;
        for (auto& annotation : annotations)
        {
            if (annotation->allFrames)
                continue;

            if (range.contains(annotation->time))
            {
                auto offset = annotation->time - range.start_time();
                annotation->time = insertTime + offset;
                skipAnnotations.insert(annotation);
            }
            else if (previous)
            {
                if (annotation->time < range.start_time())
                    skipAnnotations.insert(annotation);
            }
        }

        // Finally, move the annotations.
        if (previous)
        {
            for (auto& annotation : annotations)
            {
                if (annotation->allFrames)
                    continue;

                if (skipAnnotations.find(annotation) != skipAnnotations.end())
                    continue;

                if (annotation->time < insertTime)
                {
                    annotation->time -= range.duration();
                }
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

                if (annotation->time > insertTime && annotation->time < endTime)
                {
                    annotation->time += range.duration();
                }
            }
        }

        player->setAllAnnotations(annotations);
        view->redraw();
    }

    otio::SerializableObject::Retainer<otio::Timeline>
    create_empty_timeline(ViewerUI* ui)
    {
        otio::SerializableObject::Retainer<otio::Timeline> otioTimeline =
            new otio::Timeline("EDL");
        auto videoTrack =
            new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
        auto audioTrack =
            new otio::Track("Audio", otio::nullopt, otio::Track::Kind::audio);
        auto stack = new otio::Stack;
        stack->append_child(videoTrack);
        stack->append_child(audioTrack);
        otioTimeline->set_tracks(stack);

        return otioTimeline;
    }

    void create_empty_timeline_cb(Fl_Menu_*, ViewerUI* ui)
    {
        tcp->pushMessage("Create Empty Timeline", 0);
        tcp->lock();
        const std::string file = otioFilename(ui);

        auto timeline = create_empty_timeline(ui);

        otio::ErrorStatus errorStatus;
        timeline->to_json_file(file, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not save .otio file: {0}"))
                    .arg(errorStatus.full_description);
            tcp->unlock();
            throw std::runtime_error(error);
        }

        ui->app->open(file);
        tcp->unlock();
    }

    void create_new_timeline_cb(Fl_Menu_*, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        int Aindex = model->observeAIndex()->get();
        if (Aindex < 0)
            return;

        if (!verifySourceItem(Aindex, ui))
            return;

        tcp->pushMessage("Create New Timeline", Aindex);
        tcp->lock();

        const std::string file = otioFilename(ui);

        otio::ErrorStatus errorStatus;
        auto timeline = create_empty_timeline(ui);
        timeline->to_json_file(file, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not save .otio file: {0}"))
                    .arg(errorStatus.full_description);
            tcp->unlock();
            throw std::runtime_error(error);
        }
        ui->app->open(file);
        add_clip_to_timeline(Aindex, ui);
        tcp->unlock();
    }

    void save_timeline_to_disk(
        otio::Timeline* timeline, const std::string& otioFile,
        bool makeRelativePaths)
    {
        const std::string s = timeline->to_json_string();
        otio::SerializableObject::Retainer<otio::Timeline> out(
            dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));
        makePathsAbsolute(out, App::ui);
        auto stack = out->tracks();
        if (makeRelativePaths)
            makePathsRelative(stack, otioFile);
        otio::ErrorStatus errorStatus;
        out->to_json_file(otioFile, &errorStatus);
        if (otio::is_error(errorStatus))
        {
            std::string err = string::Format(_("Error saving {0}. {1}"))
                                  .arg(otioFile)
                                  .arg(errorStatus.full_description);
            LOG_ERROR(err);
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
        file::Path path = Aitem->path;
        bool makeRelativePaths = false;
        if (isTemporaryEDL(path))
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

    void add_clip_to_timeline(const int index, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();

        auto time = getTime(player);

        if (!verifySourceItem(index, ui))
            return;

        auto model = ui->app->filesModel();

        const auto sourceItems = model->observeFiles()->get();

        auto sourceItem = sourceItems[index];

        auto destItem = model->observeA()->get();
        if (!destItem)
        {
            LOG_ERROR(_("Destination file is invalid."));
            return;
        }

        if (!isTemporaryEDL(destItem->path))
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

            auto timelineDuration = timeline->duration();

            edit_store_undo(player, ui);

            auto Aindex = model->observeAIndex()->get();

            auto stack = timeline->tracks();
            auto tracks = stack->children();
            otio::ErrorStatus errorStatus;
            double videoRate = 0.F;
            double sampleRate = 0.F;
            int videoTrackIndex = -1;
            int audioTrackIndex = -1;
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video &&
                    videoTrackIndex == -1)
                {
                    videoTrackIndex = i;
                    videoRate = track->trimmed_range().duration().rate();
                }

                if (track->kind() == otio::Track::Kind::audio &&
                    audioTrackIndex == -1)
                {
                    audioTrackIndex = i;
                    sampleRate = track->trimmed_range().duration().rate();
                }
            }
            if (videoTrackIndex == -1 && audioTrackIndex == -1)
            {
                throw std::runtime_error(
                    _("Neither video nor audio tracks found."));
            }

            TimeRange timeRange;
            auto sourceDuration = sourceItem->inOutRange.duration();
            if (sourceDuration.rate() > videoRate)
            {
                videoRate = sourceDuration.rate();

                sanitizeVideoAndAudioRates(
                    timeline, timeRange, videoRate, sampleRate);
                player = ui->uiView->getTimelinePlayer();
                if (!player)
                    return;

                updateTimeline(timeline, player->currentTime(), ui);
            }

            auto annotations = addAnnotations(
                timelineDuration.rescaled_to(videoRate),
                player->getAllAnnotations(), sourceItem->inOutRange,
                sourceItem->annotations);

            const auto& context = ui->app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();
            if (auto read = ioSystem->read(path))
            {
                const auto info = read->getInfo().get();
                auto audioInfo = info;
                if (path != audioPath)
                {
                    read = ioSystem->read(audioPath);
                    audioInfo = read->getInfo().get();
                }
                bool isSequence =
                    io::FileType::Sequence ==
                        ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();

                RationalTime videoDuration(0.F, videoRate);

                if (!info.video.empty() && videoTrackIndex != -1)
                {
                    auto track = otio::dynamic_retainer_cast<Track>(
                        tracks[videoTrackIndex]);
                    auto clip = new otio::Clip;
                    TimeRange mediaRange(info.videoTime);
                    if (isSequence)
                    {
                        mediaRange = sourceItem->timeRange;
                        auto media = new otio::ImageSequenceReference(
                            path.getDirectory(), path.getBaseName(),
                            path.getExtension(),
                            mediaRange.start_time().value(), 1,
                            mediaRange.duration().rate(), path.getPadding());
                        media->set_available_range(mediaRange);
                        clip->set_media_reference(media);
                    }
                    else
                    {
                        auto media =
                            new otio::ExternalReference(path.get(), mediaRange);
                        clip->set_media_reference(media);
                    }
                    auto sourceRange = sourceItem->inOutRange;
                    videoDuration = sourceRange.duration();
                    if (videoDuration.rate() > videoRate)
                        videoRate = videoDuration.rate();
                    clip->set_source_range(sourceRange);
                    track->append_child(clip, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                }

                if (!audioInfo.audio.isValid() && audioTrackIndex >= 0)
                {
                    auto audioTrack = otio::dynamic_retainer_cast<Track>(
                        tracks[audioTrackIndex]);
                    if (audioTrack)
                    {
                        auto trackSampleRate =
                            audioTrack->trimmed_range().duration().rate();
                        if (trackSampleRate > sampleRate)
                            sampleRate = trackSampleRate;
                    }
                }

                auto audioDuration = RationalTime(0.0, sampleRate);
                if (audioInfo.audio.isValid())
                {
                    audioDuration = audioInfo.audioTime.duration();
                }

                if (audioDuration.rate() > sampleRate)
                    sampleRate = audioDuration.rate();
                else
                    audioDuration = audioDuration.rescaled_to(sampleRate);

                if (sampleRate > 0)
                {
                    // If no audio track, create one and fill it with a gap
                    // until new video clip.
                    // If no audio (a sequence), we also fill it with a gap.
                    if (audioTrackIndex == -1 || !audioInfo.audio.isValid())
                    {
                        auto videoTrack = otio::dynamic_retainer_cast<Track>(
                            tracks[videoTrackIndex]);
                        if (!videoTrack)
                        {
                            throw std::runtime_error(
                                _("No video track found."));
                        }
                        auto videoChildren = videoTrack->children();
                        if (videoChildren.size() <= 0)
                        {
                            throw std::runtime_error(
                                _("No video children found."));
                        }
                        auto videoComposable =
                            videoChildren[videoChildren.size() - 1];
                        auto videoClip =
                            otio::dynamic_retainer_cast<Item>(videoComposable);
                        if (!videoClip)
                        {
                            throw std::runtime_error(
                                _("Could not find video clip."));
                        }
                        TimeRange videoRange =
                            videoClip->trimmed_range_in_parent(&errorStatus)
                                .value();

                        otio::Track* audioTrack;
                        if (audioTrackIndex < 0)
                        {
                            audioTrack = new otio::Track(
                                "Audio", otio::nullopt,
                                otio::Track::Kind::audio);
                            stack->append_child(audioTrack, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error(
                                    _("Cannot append audio track."));
                            }
                            tracks = stack->children();
                            audioTrackIndex = tracks.size() - 1;
                        }
                        else
                        {
                            audioTrack = otio::dynamic_retainer_cast<Track>(
                                tracks[audioTrackIndex]);
                        }
                        audioDuration = RationalTime(
                            videoRange.duration().rescaled_to(sampleRate));
                        auto gapRange = TimeRange(
                            RationalTime(0.0, sampleRate), audioDuration);

                        auto gap = new otio::Gap(gapRange);
                        audioTrack->append_child(gap);
                    }

                    const auto inOutRange = sourceItem->inOutRange;
                    const auto start =
                        inOutRange.start_time().rescaled_to(sampleRate);
                    auto sourceDuration =
                        inOutRange.duration().rescaled_to(sampleRate);
                    auto audioTrack = otio::dynamic_retainer_cast<Track>(
                        tracks[audioTrackIndex]);
                    if (audioInfo.audio.isValid())
                    {
                        auto clip = new otio::Clip;
                        auto media = new otio::ExternalReference(
                            audioPath.get(), audioInfo.audioTime);
                        clip->set_media_reference(media);
                        if (sourceDuration > audioDuration)
                            sourceDuration = audioDuration;
                        const TimeRange sourceRange(start, sourceDuration);
                        clip->set_source_range(sourceRange);
                        audioTrack->append_child(clip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error(
                                _("Cannot append audio clip"));
                        }
                    }
                    auto gap_duration = RationalTime(0.0, sampleRate);
                    if (videoDuration > audioDuration)
                    {
                        if (audioInfo.audio.isValid())
                            gap_duration = videoDuration - audioDuration;
                        else
                            gap_duration = videoDuration;
                    }
                    // Append a gap if audio is too short.
                    if (gap_duration.value() > 0.0)
                    {
                        const auto gapRange = TimeRange(
                            RationalTime(0.0, sampleRate),
                            gap_duration.rescaled_to(sampleRate));
                        auto gap = new otio::Gap(gapRange);
                        audioTrack->append_child(gap, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error(
                                _("Cannot append audio gap"));
                        }
                    }
                }
            }

            //
            // Sanity check on video and sample rate.
            //
            videoRate = 0.F;
            sampleRate = 0.F;
            sanitizeVideoAndAudioRates(
                timeline, timeRange, videoRate, sampleRate);

            toOtioFile(timeline, ui);

            destItem->timeRange = timeRange;
            destItem->inOutRange = timeRange;
            destItem->speed = videoRate;

            clone_and_replace_cb(nullptr, ui);

            player = ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            updateTimeline(timeline, player->currentTime(), ui);
            player->setAllAnnotations(annotations);
            const auto time = sourceItem->currentTime;
            player->seek(timelineDuration.rescaled_to(time.rate()) + time);
            ui->uiTimeline->frameView();

            panel::refreshThumbnails();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        tcp->unlock();
    }

    void edit_insert_clip_annotations(
        const std::vector<mrv::MoveData>& moves, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto& timeline = player->getTimeline();
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
            if (move.fromIndex < 0 || move.fromTrack < 0 || move.toTrack < 0 ||
                move.toTrack >= tracks.size())
                continue;

            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    stack->children()[move.fromTrack]))
            {
                if (track->kind() != otio::Track::Kind::video)
                    continue;
            }

            int toIndex = move.toIndex;
            if (move.fromTrack == move.toTrack && move.fromIndex < toIndex)
            {
                --toIndex;
            }

            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.fromTrack]))
            {
                auto child = track->children()[move.fromIndex];
                auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                if (!item)
                    continue;

                auto oldRange = item->trimmed_range_in_parent().value();

                if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                        tracks[move.fromTrack]))
                {
                    auto child = track->children()[toIndex];
                    auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                    if (!item)
                        continue;

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

                    //
                    // Shift annotations
                    //
                    shiftAnnotations(oldRange, insertTime, previous, ui);
                }
            }
        }

        ui->uiTimeline->redraw();
    }

    void edit_move_clip(const std::vector<mrv::MoveData>& moves, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        std::vector<tl::timeline::MoveData> moveData;
        const auto& timeline = player->getTimeline();
        const auto& stack = timeline->tracks();
        const auto& tracks = stack->children();
        for (const auto& move : moves)
        {
            if (auto track = otio::dynamic_retainer_cast<otio::Track>(
                    tracks[move.fromTrack]))
            {
                if (auto child = track->children()[move.fromIndex])
                {
                    auto item = otio::dynamic_retainer_cast<otio::Item>(child);
                    if (!item)
                        continue;

                    timeline::MoveData data;
                    data.fromTrack = move.fromTrack;
                    data.fromIndex = move.fromIndex;
                    data.toTrack = move.toTrack;
                    data.toIndex = move.toIndex;
                    moveData.push_back(data);
                }
            }
        }

        auto otioTimeline = tl::timeline::move(timeline, moveData);
        player->player()->getTimeline()->setTimeline(otioTimeline);

        edit_move_clip_annotations(moveData, ui);
    }

    void edit_move_clip_annotations(
        const std::vector<tl::timeline::MoveData>& moves, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        edit_move_clip_annotations(moves, ui);
    }

    EditMode editMode = EditMode::kTimeline;
    int editModeH = 30;
    const int kMinEditModeH = 30;

    void save_edit_mode_state(ViewerUI* ui)
    {
        int H = ui->uiTimelineGroup->h();

        if (H == 0)
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
    {
        // Some constants, as Darby does not expose this in tlRender.
        const int kTrackTitleHeight = 24;
        const int kTrackBottomHeight = 24;
        const int kTransitionsHeight = 20;
        const int kMarkerHeight = 20;

        int H = kMinEditModeH; // timeline height
        if (editMode == EditMode::kTimeline)
            return H;
        else if (editMode == EditMode::kSaved)
            return editModeH;

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return H;

        const Fl_Tile* tile = ui->uiTileGroup;
        const int tileH = tile->h(); // Tile Height (ie. View and Edit viewport)

        // Shift the view up to see the video thumbnails and audio waveforms
        const double pixelRatio = ui->uiTimeline->pixels_per_unit();
        const int maxTileHeight = tileH - 20;
        const timelineui::ItemOptions options =
            ui->uiTimeline->getItemOptions();
        auto timeline = player->getTimeline();
        for (const auto& child : timeline->tracks()->children())
        {
            if (const auto* track = dynamic_cast<otio::Track*>(child.value))
            {
                if (otio::Track::Kind::video == track->kind())
                {
                    H += kTrackTitleHeight;
                    if (options.thumbnails)
                        H += options.thumbnailHeight / pixelRatio;
                    H += kTrackBottomHeight;
                }
                else if (otio::Track::Kind::audio == track->kind())
                {
                    H += kTrackTitleHeight;
                    if (options.thumbnails)
                        H += options.waveformHeight / pixelRatio;
                    H += kTrackBottomHeight;
                }
                // Handle Markers
                if (options.showMarkers)
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
                    H += markerSizeForTrack;
                }
                // Handle transitions
                if (options.showTransitions)
                {
                    bool found = false;
                    for (const auto& child : track->children())
                    {
                        if (const auto& transition =
                                dynamic_cast<otio::Transition*>(child.value))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        H += kTransitionsHeight;
                }
            }
        }

        if (H >= maxTileHeight)
            H = maxTileHeight;
        return H;
    }

    void set_edit_mode_cb(EditMode mode, ViewerUI* ui)
    {
        Fl_Button* b = ui->uiEdit;

        bool active = (mode == EditMode::kFull || mode == EditMode::kSaved);
        if (mode == EditMode::kSaved && editModeH == kMinEditModeH)
            active = false;

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

        Fl_Tile* tile = ui->uiTileGroup;
        Fl_Group* timeline = ui->uiTimelineGroup;
        Fl_Flex* view = ui->uiViewGroup;
        int tileY = tile->y();
        int oldY = timeline->y();
        int timelineH = timeline->h();
        int tileH = tile->h();
        int H = kMinEditModeH; // timeline height
        int viewH = H;
        auto player = ui->uiView->getTimelinePlayer();
        if (mode == EditMode::kFull && player)
        {
            timeline->show();
            if (ui->uiMain->visible())
                ui->uiTimeline->show();
            editMode = mode;
            H = calculate_edit_viewport_size(ui);
            editModeH = viewH = H;
        }
        else if (mode == EditMode::kSaved)
        {
            H = viewH = editModeH;
            timeline->show();
            if (ui->uiMain->visible())
                ui->uiTimeline->show();
        }
        else if (mode == EditMode::kNone)
        {
            viewH = 0;
            ui->uiTimeline->hide();
            timeline->hide();
        }
        else
        {
            H = kMinEditModeH; // timeline height
            viewH = editModeH = H;

            // EditMode::kTimeline
            timeline->show();
            if (ui->uiMain->visible())
                ui->uiTimeline->show();
        }

        int newY = tileY + tileH - H;

        view->resize(view->x(), view->y(), view->w(), tileH - viewH);
        if (timeline->visible())
            timeline->resize(timeline->x(), newY, timeline->w(), H);

        if (mode != EditMode::kNone)
        {
            Message msg;
            msg["command"] = "setEditMode";
            msg["value"] = mode;
            msg["height"] = H;
            tcp->pushMessage(msg);
        }

        view->layout();
        tile->init_sizes();

        if (timeline->visible())
            timeline->redraw(); // needed
    }

} // namespace mrv
