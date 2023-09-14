
// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

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
            RationalTime time;
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
            std::vector<std::shared_ptr<tl::draw::Annotation>> annotations;
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

        file::Path
        getRelativePath(const file::Path& path, fs::path otioFilename)
        {
            fs::path filePath = path.get();
            otioFilename = otioFilename.parent_path(); // Remove .otio file.
            fs::path relative = fs::relative(filePath, otioFilename);
            std::string file = relative.generic_string();
            return file::Path(file);
        }

        void updateTimeline(
            const otio::Timeline* timeline, const double rate, ViewerUI* ui)
        {
            // Set the end frame in the
            auto one_frame = RationalTime(1.0, rate);
            auto startTime = RationalTime(0.0, rate);
            if (timeline->global_start_time().has_value())
                startTime = timeline->global_start_time().value();
            auto endTime = startTime + timeline->duration() - one_frame;
            endTime = endTime.rescaled_to(rate);
            TimelineClass* c = ui->uiTimeWindow;
            c->uiEndFrame->setTime(endTime);
            auto player = ui->uiView->getTimelinePlayer();
            player->setTimeline(timeline);
            player->setSpeed(rate);
            player->setInOutRange(player->timeRange());
            ui->uiTimeline->frameView();
            ui->uiTimeline->redraw();
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
            RationalTime timeInTrack(time.rescaled_to(rate));
            frame.time = timeInTrack;

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
            Clip* clip = dynamic_cast<Clip*>(clonedItem);
            if (clip)
            {
                clip->set_source_range(range);
            }
            else
            {
                Gap* gap = dynamic_cast<Gap*>(clonedItem);
                if (!gap)
                    return;
                gap->set_source_range(range);
            }
            frame.item = clonedItem;
            copiedFrames.push_back(frame);
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
                        file::Path urlPath(ref->target_url_base());
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
                refreshPanelThumbnails();
            }
        }

        //! Change clips' source range to use the highest video and audio
        //! sample rate.  Also returns the largest time range for the timeline.
        void sanitizeVideoAndAudioRates(
            otio::Timeline* timeline, TimeRange& timeRange, double& videoRate,
            double& sampleRate)
        {
            videoRate = 0.0;
            sampleRate = 0.0;
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
                            // Make sure audio is at least one video frame long
                            auto videoDuration =
                                duration.rescaled_to(videoRate);
                            if (videoDuration.value() < 1.0)
                            {
                                duration = RationalTime(1.0, videoRate);
                                duration = duration.rescaled_to(sampleRate);
                            }
                            range = TimeRange(start, duration);
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
            const RationalTime& duration, const double videoRate,
            const std::vector<std::shared_ptr<draw::Annotation>>& annotations,
            const std::vector<std::shared_ptr<draw::Annotation>>&
                clipAnnotations)
        {
            std::vector<std::shared_ptr<draw::Annotation>> out;
            for (auto a : annotations)
            {
                out.push_back(a);
            }
            for (auto a : clipAnnotations)
            {
                out.push_back(a);
                auto time = out.back()->time;
                time += duration;
                time = time::round(time.rescaled_to(videoRate));
                out.back()->time = time;
            }
            return out;
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

    } // namespace

    //! Dump undo queue to tmpdir.
    void dump_undo_queue_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        // const std::string path = tmppath() + "/UndoQueue";
        // int mode = 0777;
        // fl_mkdir(path.c_str(), mode);
        // unsigned int idx = 1;
        // char buf[4096];
        // for (auto& undo : undoBuffer)
        // {
        //     snprintf(buf, 4096, "%s/undo.%d.otio", path.c_str(), idx);
        //     std::ofstream f(buf);
        //     f << undo.json << std::endl;
        //     f.close();
        //     ++idx;
        // }
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

        UndoRedo buffer;
        buffer.json = state;
        buffer.fileName = getEDLName(ui);
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

        UndoRedo buffer;
        buffer.json = state;
        buffer.fileName = getEDLName(ui);
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

        updateTimeline(timeline, time.rate(), ui);
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

        const TimeRange range(time, RationalTime(1.0, time.rate()));

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

        // TimeRange timeRange;
        // double videoRate, sampleRate;
        // sanitizeVideoAndAudioRates(timeline, timeRange, videoRate,
        // sampleRate);
        updateTimeline(timeline, time.rate(), ui);

        toOtioFile(timeline, ui);

        redrawPanelThumbnails();

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

            otio::algo::insert(item, track, time);
            frame.item = item;
        }

        const RationalTime one_frame(1.0, time.rate());
        auto annotations =
            offsetAnnotations(time, one_frame, player->getAllAnnotations());
        player->setAllAnnotations(annotations);

        edit_clear_redo(ui);

        TimeRange timeRange;
        double videoRate, sampleRate;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        updateTimeline(timeline, videoRate, ui);
        toOtioFile(timeline, ui);

        redrawPanelThumbnails();

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

        TimeRange timeRange;
        double videoRate, sampleRate;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        auto annotations =
            removeAnnotations(timeRange, player->getAllAnnotations());

        player->setAllAnnotations(annotations);
        updateTimeline(timeline, videoRate, ui);

        ui->uiTimeline->setTimelinePlayer(player);

        toOtioFile(timeline, ui);

        tcp->pushMessage("Edit/Remove", time);
    }

    void edit_trim_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_slip_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_slide_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_ripple_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_roll_cb(Fl_Menu_* m, ViewerUI* ui) {}

    void edit_remove_clip_with_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player, time);

        auto timeline = player->getTimeline();
        edit_store_undo(player, ui);

        for (auto track : tracks)
        {
            otio::algo::remove(track, time);
        }
        player->setTimeline(timeline);
        updateTimeline(timeline, time.rate(), ui);

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

        if (!switchToEDL(buffer.fileName, ui))
            return;

        // We must get player again, as we might have changed clips.
        player = ui->uiView->getTimelinePlayer();
        edit_store_redo(player, ui);

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(buffer.json)));

        auto stack = timeline->tracks();
        TimeRange timeRange;
        double videoRate, sampleRate;
        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        player->setAllAnnotations(buffer.annotations);
        player->setTimeline(timeline);
        updateTimeline(timeline, videoRate, ui);

        toOtioFile(timeline, ui);
        if (playlistPanel)
            playlistPanel->redraw();
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
        double videoRate, sampleRate;
        stack = timeline->tracks();

        sanitizeVideoAndAudioRates(timeline, timeRange, videoRate, sampleRate);

        player->setAllAnnotations(buffer.annotations);
        player->setTimeline(timeline);
        updateTimeline(timeline, videoRate, ui);

        toOtioFile(timeline, ui);

        if (playlistPanel)
            playlistPanel->redraw();

        if (refreshCache)
            refresh_file_cache_cb(nullptr, ui);
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

        auto Aitem = model->observeA()->get();
        std::string Afile = Aitem->path.get();

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
        add_clip_to_timeline(Afile, Aindex, ui);
        tcp->unlock();
    }

    void
    save_timeline_to_disk(otio::Timeline* timeline, const std::string& otioFile)
    {
        const std::string s = timeline->to_json_string();
        otio::SerializableObject::Retainer<otio::Timeline> out(
            dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(s)));
        auto stack = out->tracks();
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

    void save_timeline_to_disk_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto model = ui->app->filesModel();
        auto Aitem = model->observeA()->get();
        file::Path path = Aitem->path;
        if (!isTemporaryEDL(path))
        {
            LOG_ERROR(_("Not an EDL file to save."));
            return;
        }

        auto timeline = player->getTimeline();
        if (timeline->duration().value() <= 0.0)
        {
            LOG_ERROR(_("Empty EDL file.  Not saving."));
            return;
        }

        auto otioFile = save_otio(nullptr, ui);
        if (otioFile.empty())
            return;

        save_timeline_to_disk(timeline, otioFile);
    }

    void
    add_clip_to_timeline(const std::string& file, const int index, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();

        auto time = getTime(player);

        if (!verifySourceItem(index, ui))
            return;

        auto model = ui->app->filesModel();

        const auto& sourceItems = model->observeFiles()->get();
        auto sourceItem = sourceItems[index];

        auto destItem = model->observeA()->get();

        if (!isTemporaryEDL(destItem->path))
        {
            LOG_ERROR(_("You can only add clips to an .otio EDL playlist."));
            return;
        }

        Message msg;
        msg["command"] = "Add Clip to Timeline";
        msg["fileName"] = file;
        msg["sourceIndex"] = index;
        tcp->pushMessage(msg);
        tcp->lock();

        try
        {
            auto timelineDuration = timeline->duration();
            auto annotations = addAnnotations(
                timelineDuration, player->defaultSpeed(),
                player->getAllAnnotations(), sourceItem->annotations);

            edit_store_undo(player, ui);

            auto Aindex = model->observeAIndex()->get();
            const file::Path path(file);
            auto stack = timeline->tracks();
            const bool refreshCache = hasEmptyTracks(stack);
            auto tracks = stack->children();
            otio::ErrorStatus errorStatus;
            int videoTrackIndex = -1;
            int audioTrackIndex = -1;
            for (int i = 0; i < tracks.size(); ++i)
            {
                auto track = otio::dynamic_retainer_cast<Track>(tracks[i]);
                if (!track)
                    continue;
                if (track->kind() == otio::Track::Kind::video &&
                    videoTrackIndex == -1)
                    videoTrackIndex = i;

                if (track->kind() == otio::Track::Kind::audio &&
                    audioTrackIndex == -1)
                    audioTrackIndex = i;
            }
            if (videoTrackIndex == -1 && audioTrackIndex == -1)
            {
                throw std::runtime_error(
                    _("Neither video nor audio tracks found."));
            }

            const auto& context = ui->app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();
            if (auto read = ioSystem->read(path))
            {
                const auto info = read->getInfo().get();

                bool isSequence =
                    io::FileType::Sequence ==
                        ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();

                if (!info.video.empty() && videoTrackIndex != -1)
                {
                    auto track = otio::dynamic_retainer_cast<Track>(
                        tracks[videoTrackIndex]);
                    auto clip = new otio::Clip;
                    const TimeRange mediaRange(info.videoTime);
                    if (isSequence)
                    {
                        auto media = new otio::ImageSequenceReference(
                            path.getDirectory(), path.getBaseName(),
                            path.getExtension(),
                            mediaRange.start_time().value(), 1,
                            mediaRange.duration().rate(), path.getPadding());
                        clip->set_media_reference(media);
                    }
                    else
                    {
                        auto media =
                            new otio::ExternalReference(path.get(), mediaRange);
                        clip->set_media_reference(media);
                    }
                    auto sourceRange = sourceItem->inOutRange;
                    const double rate = sourceRange.duration().rate();
                    clip->set_source_range(sourceRange);
                    track->append_child(clip, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }

                    // If audio is longer than video, append a video gap.
                    if (info.audio.isValid())
                    {
                        const auto audio_duration =
                            info.audioTime.duration().rescaled_to(rate);
                        const auto clip_duration = mediaRange.duration();
                        const auto gap_duration =
                            audio_duration - clip_duration;
                        if (time::round(gap_duration).value() > 0.0)
                        {
                            const auto gapRange = TimeRange(
                                RationalTime(0.0, rate), gap_duration);
                            auto gap = new otio::Gap(gapRange);
                            track->append_child(gap, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error(
                                    _("Cannot append video gap"));
                            }
                        }
                    }
                }

                if (info.audio.isValid())
                {
                    const auto rate = info.audioTime.duration().rate();
                    // If no audio track, create one and fill it with a gap
                    // until new clip.
                    if (audioTrackIndex == -1)
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

                        auto gapRange = TimeRange(
                            RationalTime(0.0, rate),
                            RationalTime(
                                videoRange.start_time().rescaled_to(rate)));
                        auto gap = new otio::Gap(gapRange);
                        auto audioTrack = new otio::Track(
                            "Audio", otio::nullopt, otio::Track::Kind::audio);
                        audioTrack->append_child(gap);
                        stack->append_child(audioTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error(
                                _("Cannot append audio track."));
                        }
                        tracks = stack->children();
                        audioTrackIndex = tracks.size() - 1;
                    }

                    auto track = otio::dynamic_retainer_cast<Track>(
                        tracks[audioTrackIndex]);
                    auto clip = new otio::Clip;
                    auto media =
                        new otio::ExternalReference(path.get(), info.audioTime);
                    clip->set_media_reference(media);
                    const auto inOutRange = sourceItem->inOutRange;
                    const auto start =
                        inOutRange.start_time().rescaled_to(rate);
                    auto duration = inOutRange.duration().rescaled_to(rate);
                    auto gap_duration = RationalTime(0.0, rate);
                    if (duration > info.audioTime.duration())
                    {
                        gap_duration = duration - info.audioTime.duration();
                        duration = info.audioTime.duration();
                    }
                    const TimeRange sourceRange(start, duration);
                    clip->set_source_range(sourceRange);
                    track->append_child(clip, &errorStatus);
                    if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error(_("Cannot append audio clip"));
                    }
                    // Append a gap if audio is too short.
                    if (gap_duration.value() > 0.0)
                    {
                        const auto gapRange =
                            TimeRange(RationalTime(0.0, rate), gap_duration);
                        auto gap = new otio::Gap(gapRange);
                        track->append_child(gap, &errorStatus);
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
            TimeRange timeRange;
            double videoRate;
            double sampleRate;
            sanitizeVideoAndAudioRates(
                timeline, timeRange, videoRate, sampleRate);

            toOtioFile(timeline, ui);

            destItem->timeRange = timeRange;
            destItem->inOutRange = timeRange;
            destItem->speed = videoRate;

            if (refreshCache)
                refresh_file_cache_cb(nullptr, ui);

            player = ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            player->setAllAnnotations(annotations);
            updateTimeline(timeline, videoRate, ui);

            refreshPanelThumbnails();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        tcp->unlock();
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

        const Fl_Tile* tile = ui->uiTileGroup;
        const int tileH = tile->h(); // Tile Height (ie. View and Edit viewport)

        int H = kMinEditModeH; // timeline height

        // Shift the view up to see the video thumbnails and audio waveforms
        const double pixelRatio = ui->uiTimeline->pixels_per_unit();
        const int maxTileHeight = tileH - 20;
        const timelineui::ItemOptions options =
            ui->uiTimeline->getItemOptions();
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return H;

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
        if (!(mode == EditMode::kSaved))
            ui->uiView->resizeWindow();
        if (b->value())
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
        if (mode == EditMode::kFull)
        {
            timeline->show();
            if (ui->uiMain->visible())
                ui->uiTimeline->show();
            H = calculate_edit_viewport_size(ui);
            editMode = EditMode::kSaved;
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
            Message msg = {{"command", "setEditMode"}, {"value", mode}};
            tcp->pushMessage(msg);
        }

        view->layout();
        tile->init_sizes();

        if (timeline->visible())
            timeline->redraw(); // needed
    }

} // namespace mrv
