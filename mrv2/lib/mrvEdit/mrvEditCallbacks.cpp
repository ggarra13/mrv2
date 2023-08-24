
#include <fstream>

#include <opentimelineio/clip.h>
#include <opentimelineio/editAlgorithm.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>

#include <tlCore/Path.h>
#include <tlIO/IOSystem.h>

#include "mrvNetwork/mrvTCP.h"

#include "mrvEdit/mrvEditCallbacks.h"

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
            std::string kind;
            otio::SerializableObject::Retainer<Item> item;
        };

        static std::vector<FrameInfo> copiedFrames;

        static std::vector<std::string> undoBuffer;
        static std::vector<std::string> redoBuffer;

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

        std::vector<Item*>
        getItems(TimelinePlayer* player, const RationalTime& time)
        {
            std::vector<Item*> out;
            auto timeline = player->getTimeline();

            otio::ErrorStatus errorStatus;
            auto tracks = timeline->tracks()->children();
            for (auto child : tracks)
            {
                auto composition =
                    otio::dynamic_retainer_cast<Composition>(child);
                if (!composition)
                    continue;
                auto item = otio::dynamic_retainer_cast<Item>(
                    composition->child_at_time(time, &errorStatus));
                if (!item || otio::is_error(errorStatus))
                    continue;
                out.push_back(item);
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

        void setEndTime(
            const otio::Timeline* timeline, const double rate, ViewerUI* ui)
        {
            // Set the end frame in the
            auto one_frame = RationalTime(1.0, rate);
            auto startTime = RationalTime(0.0, rate);
            if (timeline->global_start_time())
                startTime = timeline->global_start_time().value();
            auto endTime = startTime + timeline->duration() - one_frame;
            endTime = endTime.rescaled_to(rate);
            TimelineClass* c = ui->uiTimeWindow;
            c->uiEndFrame->setTime(endTime);
            auto new_range =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    startTime, endTime);
            auto player = ui->uiView->getTimelinePlayer();
            player->setInOutRange(new_range);
            ui->uiTimeline->redraw();
        }

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
            frame.trackIndex = getIndex(composition);
            frame.kind = track->kind();

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

    } // namespace

    void edit_store_undo(TimelinePlayer* player)
    {
        auto timeline = player->getTimeline();
        std::string state = timeline->to_json_string();
        if (!undoBuffer.empty())
        {
            // Don't store anything if no change.
            if (undoBuffer.back() == state)
            {
                return;
            }
        }
        undoBuffer.push_back(state);
    }

    void edit_clear_redo()
    {
        redoBuffer.clear();
    }

    void edit_store_redo(TimelinePlayer* player)
    {
        auto timeline = player->getTimeline();
        std::string state = timeline->to_json_string();
        if (!redoBuffer.empty())
        {
            // Don't store anything if no change.
            if (redoBuffer.back() == state)
            {
                return;
            }
        }
        redoBuffer.push_back(state);
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

        auto timeline = player->getTimeline();
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
    }

    void edit_cut_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();
        auto tracks = timeline->tracks()->children();

        edit_copy_frame_cb(m, ui);

        const auto startTime = player->timeRange().start_time();
        const auto time = player->currentTime() - startTime;
        const auto one_frame = RationalTime(1.0, time.rate());

        edit_store_undo(player);

        otio::ErrorStatus errorStatus;
        for (const auto& frame : copiedFrames)
        {
            const int trackIndex = frame.trackIndex;
            if (trackIndex < 0 ||
                static_cast<size_t>(trackIndex) >= tracks.size())
                continue;

            auto track = otio::dynamic_retainer_cast<Track>(tracks[trackIndex]);
            auto cut_item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            if (!cut_item)
            {
                LOG_ERROR(
                    "No item found at " << time << " "
                                        << otio::ErrorStatus::outcome_to_string(
                                               errorStatus.outcome));
                continue;
            }
            auto item_range = cut_item->trimmed_range();
            auto track_range =
                cut_item->trimmed_range_in_parent(&errorStatus).value();

            otio::algo::slice(track, time);
            const RationalTime out_time = time + one_frame;
            otio::algo::slice(track, out_time);

            // Get the cut item
            cut_item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            if (!cut_item)
            {
                LOG_ERROR(
                    "No cut frame found at "
                    << time << " "
                    << otio::ErrorStatus::outcome_to_string(
                           errorStatus.outcome));
                continue;
            }

            item_range = cut_item->trimmed_range();
            int index = track->index_of_child(cut_item);
            auto children_size = track->children().size();
            if (index < 0 || static_cast<size_t>(index) >= children_size)
                continue;
            track->remove_child(index);
        }
        player->setTimeline(timeline);
        edit_clear_redo();

        setEndTime(timeline, time.rate(), ui);
    }

    void edit_paste_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        const auto time = getTime(player);

        auto timeline = player->getTimeline();
        auto tracks = timeline->tracks()->children();

        edit_store_undo(player);

        for (auto& frame : copiedFrames)
        {
            TimeRange range(time, RationalTime(1.0, time.rate()));
            auto item = dynamic_cast<Item*>(frame.item->clone());
            if (!item)
                continue;
            const int trackIndex = frame.trackIndex;
            if (trackIndex < 0 ||
                static_cast<size_t>(trackIndex) >= tracks.size())
                continue;

            auto track = otio::dynamic_retainer_cast<Track>(tracks[trackIndex]);
            otio::algo::overwrite(item, track, range);
            frame.item = item;
        }

        player->setTimeline(timeline);
        edit_clear_redo();
    }

    void edit_insert_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        auto timeline = player->getTimeline();
        const auto time = getTime(player);
        auto tracks = timeline->tracks()->children();

        edit_store_undo(player);

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
            otio::algo::insert(item, track, time);
            frame.item = item;
        }

        player->setTimeline(timeline);

        edit_clear_redo();

        setEndTime(timeline, time.rate(), ui);
    }

    void edit_slice_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player, time);

        auto timeline = player->getTimeline();
        edit_store_undo(player);

        bool remove_undo = true;
        otio::ErrorStatus errorStatus;
        for (auto track : tracks)
        {
            auto cut_item = otio::dynamic_retainer_cast<Item>(
                track->child_at_time(time, &errorStatus));
            auto cut_range =
                cut_item->trimmed_range_in_parent(&errorStatus).value();
            if (cut_range.start_time() == time ||
                cut_range.end_time_exclusive() == time)
                continue;
            remove_undo = false;
            otio::algo::slice(track, time);
        }

        // For slicing, if we slice at the same point of another slice,
        // we may get two different timelines.  We avoid it first.
        if (remove_undo)
            undoBuffer.pop_back();

        edit_clear_redo();

        player->setTimeline(timeline);
    }

    void edit_remove_clip_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        auto tracks = getTracks(player, time);

        auto timeline = player->getTimeline();
        edit_store_undo(player);

        for (auto track : tracks)
        {
            otio::algo::remove(track, time, false);
        }
        player->setTimeline(timeline);
        ui->uiTimeline->setTimelinePlayer(player);

        setEndTime(timeline, time.rate(), ui);
    }

    void edit_remove_clip_with_gap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& time = getTime(player);
        const auto& tracks = getTracks(player, time);

        auto timeline = player->getTimeline();
        edit_store_undo(player);

        for (auto track : tracks)
        {
            otio::algo::remove(track, time);
        }
        player->setTimeline(timeline);
        setEndTime(timeline, time.rate(), ui);
    }

    void edit_undo_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (undoBuffer.empty())
            return;

        const auto& time = getTime(player);

        auto json = undoBuffer.back();
        undoBuffer.pop_back();
        edit_store_redo(player);

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(json)));
        player->setTimeline(timeline);
        setEndTime(timeline, time.rate(), ui);
    }

    void edit_redo_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (redoBuffer.empty())
            return;

        auto time = getTime(player);

        auto json = redoBuffer.back();
        redoBuffer.pop_back();
        edit_store_undo(player);

        otio::SerializableObject::Retainer<otio::Timeline> timeline(
            dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_string(json)));
        player->setTimeline(timeline);
        setEndTime(timeline, time.rate(), ui);
    }

    void add_clip_to_timeline(const std::string& file, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();

        edit_store_undo(player);

        auto time = getTime(player);
        auto model = ui->app->filesModel();
        auto fileItem = model->observeA()->get();

        file::Path path(file);
        auto tracks = timeline->tracks()->children();
        otio::ErrorStatus errorStatus;
        const auto& context = ui->app->getContext();
        auto ioSystem = context->getSystem<tl::io::System>();
        for (auto composition : tracks)
        {
            auto track = otio::dynamic_retainer_cast<Track>(composition);
            if (!track)
                continue;
            if (track->kind() == otio::Track::Kind::video ||
                track->kind() == otio::Track::Kind::audio)
            {
                bool isSequence =
                    io::FileType::Sequence ==
                        ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();

                auto clip = new otio::Clip;
                if (!clip)
                    continue;

                TimeRange mediaRange(fileItem->timeRange);
                if (isSequence)
                {
                    auto media = new otio::ImageSequenceReference(
                        path.getDirectory(), path.getBaseName(),
                        path.getExtension(), mediaRange.start_time().value(), 1,
                        mediaRange.duration().rate(), path.getPadding());
                    clip->set_media_reference(media);
                }
                else
                {
                    auto media = new otio::ExternalReference(file, mediaRange);
                    clip->set_media_reference(media);
                }
                clip->set_source_range(fileItem->inOutRange);

                track->append_child(clip, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
            }
        }
        player->setTimeline(timeline);
        setEndTime(timeline, time.rate(), ui);
        ui->uiTimeline->frameView();
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

        auto otioTimeline = player->getTimeline();
        for (const auto& child : otioTimeline->tracks()->children())
        {
            if (const auto* track = dynamic_cast<otio::Track*>(child.value))
            {
                if (otio::Track::Kind::video == track->kind())
                {
                    H += 24; // title bar
                    if (options.thumbnails)
                        H += options.thumbnailHeight / pixelRatio;
                    H += 24; // bottom bar
                }
                else if (otio::Track::Kind::audio == track->kind())
                {
                    H += 24; // title bar
                    if (options.thumbnails)
                        H += options.waveformHeight / pixelRatio;
                    H += 24; // bottom bar
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
                        H += 20;
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
            assert(view->h() + timeline->h() == tile->h());
            assert(timeline->y() == view->y() + view->h());
            tcp->pushMessage("setEditMode", (int)mode);
        }

        view->layout();
        tile->init_sizes();

        if (timeline->visible())
            timeline->redraw(); // needed

        // std::cerr << "editModeH=" << editModeH << std::endl;
        // std::cerr << "tileY=" << tileY << std::endl;
        // std::cerr << "tileH=" << tileH << " tileMY=" << tileY + tileH
        //           << std::endl
        //           << std::endl;
        // std::cerr << "viewgroupX=" << view->x() << std::endl;
        // std::cerr << "viewgroupW=" << view->w()
        //           << " viewgroupMX=" << view->x() + view->w() << std::endl;
        // std::cerr << "uiToolGroupX=" << ui->uiToolsGroup->x() << std::endl;
        // std::cerr << "uiToolGroupW=" << ui->uiToolsGroup->w()
        //           << " uiToolsGroupMX="
        //           << (ui->uiToolsGroup->x() + ui->uiToolsGroup->w())
        //           << std::endl;
        // std::cerr << "uiViewX=" << ui->uiView->x() << std::endl;
        // std::cerr << "uiViewW=" << ui->uiView->w()
        //           << " uiViewMX=" << (ui->uiView->x() + ui->uiView->w())
        //           << std::endl;
        // std::cerr << "uiDockGroupX=" << ui->uiDockGroup->x() << std::endl;
        // std::cerr << "uiDockGroupW=" << ui->uiDockGroup->w()
        //           << " uiDockGroupMX="
        //           << (ui->uiDockGroup->x() + ui->uiDockGroup->w())
        //           << std::endl
        //           << std::endl;
        // std::cerr << "viewgroupY=" << view->y() << std::endl;
        // std::cerr << "viewgroupH=" << view->h()
        //           << " viewgroupMY=" << view->y() + view->h() << std::endl;
        // std::cerr << "uiViewY=" << ui->uiView->y() << std::endl;
        // std::cerr << "uiViewH=" << ui->uiView->h()
        //           << " uiViewMY=" << (ui->uiView->y() + ui->uiView->h())
        //           << std::endl;
        // std::cerr << "timelineGroupY=" << timeline->y() << std::endl;
        // std::cerr << "timelineGroupH=" << timeline->h()
        //           << " timelineGroupMY=" << (timeline->y() + timeline->h())
        //           << std::endl;
        // std::cerr << "uiTimelineX=" << ui->uiTimeline->x() << std::endl;
        // std::cerr << "uiTimelineY=" << ui->uiTimeline->y() << std::endl;
        // std::cerr << "uiTimelineH=" << ui->uiTimeline->h()
        //           << " uiTimelineMY="
        //           << (ui->uiTimeline->y() + ui->uiTimeline->h())
        //           << std::endl;
        // std::cerr << std::endl;
    }

} // namespace mrv
