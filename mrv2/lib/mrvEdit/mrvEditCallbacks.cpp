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

        std::vector<Composition*> getTracks(TimelinePlayer* player)
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
                        auto item = otio::dynamic_retainer_cast<Item>(child);
                        if (!item)
                            continue;
                        if (videoRate > 0)
                        {
                            auto range = item->trimmed_range();
                            auto start = time::round(
                                range.start_time().rescaled_to(videoRate));
                            auto duration = time::round(
                                range.duration().rescaled_to(videoRate));
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
                            auto start = time::round(
                                range.start_time().rescaled_to(sampleRate));
                            auto duration = time::round(
                                range.duration().rescaled_to(sampleRate));
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
        const auto& tracks = getTracks(player);

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
        const auto& tracks = getTracks(player);

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

        edit_clear_redo(ui);

        panel::redrawThumbnails();

        tcp->pushMessage("Edit/Remove", time);
    }

    void edit_insert_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();

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
                "Audio", otio::nullopt, otio::Track::Kind::audio);
            stack->append_child(track);

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
                clipRange.start_time().rescaled_to(sampleRate),
                clipRange.duration().rescaled_to(sampleRate));
            otio::Gap* gap = new Gap(audioClipRange);
            if (audioIndex < 0 || audioIndex >= track->children().size())
                track->append_child(gap);
            else
                track->insert_child(audioIndex, gap);
        }

        updateTimeline(timeline, time, ui);

        toOtioFile(timeline, ui);

        if (modified)
            edit_clear_redo(ui);

        panel::redrawThumbnails();

        tcp->pushMessage("Edit/Audio Gap/Insert", time);
    }

    void edit_remove_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto compositions = getTracks(player);

        auto timeline = player->getTimeline();
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

        tcp->pushMessage("Edit/Audio Gap/Remove", time);
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
    createEmptyTimeline(ViewerUI* ui)
    {
        otio::SerializableObject::Retainer<otio::Timeline> otioTimeline =
            new otio::Timeline("EDL");

        auto videoTrack =
            new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);

        auto stack = new otio::Stack;
        stack->append_child(videoTrack);

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
        if (!Aitem)
            return;

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

    void add_clip_to_new_timeline_cb(const int index, ViewerUI* ui)
    {
        create_new_timeline_cb(ui);
        add_clip_to_timeline_cb(index, ui);
    }

    void addTimelineToEDL(
        otio::Timeline* destTimeline, const otio::Timeline* sourceTimeline,
        const TimeRange& inOutRange, const TimeRange& timeRange)
    {
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

        if (time::compareExact(destStartTime, time::invalidTime))
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
                    "Video", otio::nullopt, otio::Track::Kind::video);
                destStack->append_child(track);
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
                track->append_child(gap);
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
                        track->append_child(item);
                    }
                }
                else
                {
                    auto transition = dynamic_cast<Transition*>(clone);
                    if (transition)
                    {
                        track->append_child(transition);
                    }
                    else
                    {
                        LOG_ERROR("Unknown child " << child->name());
                    }
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
                    track->append_child(gap);
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
                    "Audio", otio::nullopt, otio::Track::Kind::audio);
                destStack->append_child(track);
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
                track->append_child(gap);
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
                        track->append_child(item);
                    }
                }
                else
                {
                    auto transition = dynamic_cast<Transition*>(clone);
                    if (transition)
                    {
                        track->append_child(transition);
                    }
                    else
                    {
                        LOG_ERROR("Unknown child " << child->name());
                    }
                }
            }
        }
    }

    void addClipToTimeline(
        const int sourceIndex, otio::Timeline* destTimeline, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        auto destIndex = model->observeAIndex()->get();
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

        // Make a copy of the timeline, so we don't modify the original in
        // place.
        const std::string& s = timeline->to_json_string();
        otio::SerializableObject::Retainer<otio::Timeline> sourceTimeline(
            dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));
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

            addClipToTimeline(index, destTimeline, ui);

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

            clone_and_replace_cb(nullptr, ui);

            player = ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            updateTimeline(destTimeline, player->currentTime(), ui);
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

    void edit_move_clip_annotations(
        const std::vector<tl::timeline::MoveData>& moves, ViewerUI* ui)
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

                    // std::cerr << "previous =" << previous << std::endl;
                    // std::cerr << "fromIndex=" << move.fromIndex << std::endl;
                    // std::cerr << "  toIndex=" << toIndex << std::endl;
                    // std::cerr << " oldRange=" << oldRange << std::endl;
                    // std::cerr << "   insert=" << insertTime << std::endl;

                    //
                    // Shift annotations
                    //
                    shiftAnnotations(oldRange, insertTime, previous, ui);
                }
            }
        }

        ui->uiTimeline->redraw();
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
        const int kTrackTitleHeight = 20;
        const int kTrackBottomHeight = 20;
        const int kTransitionsHeight = 20;
        const int kAudioGapOnlyHeight = 20;
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

        int videoHeight = 0;
        int audioHeight = 0;
        int markersHeight = 0;
        int transitionsHeight = 0;

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
                    videoHeight += kTrackTitleHeight;
                    if (options.thumbnails)
                        videoHeight += options.thumbnailHeight / pixelRatio;
                    videoHeight += kTrackBottomHeight;
                }
                else if (otio::Track::Kind::audio == track->kind())
                {
                    if (track->children().size() > 0)
                    {
                        audioHeight += kTrackTitleHeight;
                        bool hasWaveform = false;
                        if (options.thumbnails)
                        {
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
                        }
                        if (hasWaveform)
                            audioHeight += options.waveformHeight / pixelRatio;
                        else
                            audioHeight += kAudioGapOnlyHeight / pixelRatio;

                        audioHeight += kTrackBottomHeight;
                    }
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
                    markersHeight += markerSizeForTrack;
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
                        transitionsHeight += kTransitionsHeight;
                }
            }
        }

        H += videoHeight + audioHeight + markersHeight + transitionsHeight;

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
