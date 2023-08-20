

#include <opentimelineio/clip.h>
#include <opentimelineio/editAlgorithm.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>

#include "mrvNetwork/mrvTCP.h"

#include "mrvFl/mrvEditCallbacks.h"
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

    struct FrameInfo
    {
        Composition* composition;
        std::string kind;
        Item* item;
    };

    static std::vector<FrameInfo> copiedFrames;

    void add_copy_frame(
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
        frame.composition = composition;
        frame.kind = track->kind();
        auto clonedItem = dynamic_cast<Item*>(item->clone());
        auto clip_range = item->trimmed_range();
        auto track_range = item->trimmed_range_in_parent(&errorStatus);
        if (is_error(errorStatus))
        {
            LOG_ERROR(item->name() << " is not attached to a track.");
            return;
        }
        auto range = TimeRange(
            time - track_range.value().start_time() + clip_range.start_time(),
            RationalTime(1.0, time.rate()));
        Clip* clip = dynamic_cast<Clip*>(clonedItem);
        if (clip)
        {
            clip->set_source_range(range);
            frame.item = clip;
        }
        else
        {
            Gap* gap = dynamic_cast<Gap*>(clonedItem);
            if (!gap)
                return;
            gap->set_source_range(range);
            frame.item = gap;
        }
        copiedFrames.push_back(frame);
    }

    void edit_copy_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto timeline = player->getTimeline();
        const auto timeline_range = player->timeRange();
        const auto startTime = timeline_range.start_time();
        const auto time = player->currentTime() - startTime;

        otio::ErrorStatus errorStatus;
        RationalTime one_frame = RationalTime(1.0, time.rate());
        TimeRange range(time, one_frame);

        copiedFrames.clear();

#if 0
        // @bug:  This OpentimelineIO routine is buggy.  It does not return
        //        all items when there's two tracks.
        auto items = timeline->find_children<Item>(&errorStatus, range);
        for (const auto& item : items)
        {
            add_copy_frame(item->parent(), item, time);
        }
#else
        auto stack = timeline->tracks();
        for (auto child : stack->children())
        {
            auto composition = otio::dynamic_retainer_cast<Composition>(child);
            if (!composition)
                continue;
            auto items = composition->find_children<Item>(&errorStatus, range);
            for (const auto& item : items)
            {
                add_copy_frame(composition, item, time);
            }
        }
#endif
        if (copiedFrames.empty())
        {
            LOG_ERROR(_("No frames were copied."));
        }
    }

    void edit_cut_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto players = ui->uiView->getTimelinePlayers();
        if (players.empty())
            return;

        auto player = players[0];
        auto timeline = player->getTimeline();

        edit_copy_frame_cb(m, ui);

        const auto startTime = player->timeRange().start_time();
        const auto time = player->currentTime() - startTime;

        player->setTimeline(nullptr);
        otio::ErrorStatus errorStatus;
        for (const auto& frame : copiedFrames)
        {
            Composition* composition = frame.composition;
            const auto one_frame = RationalTime(1.0, time.rate());
            const TimeRange cut_range(time, one_frame);
            auto items =
                composition->find_children<Item>(&errorStatus, cut_range);
            if (items.empty())
            {
                LOG_ERROR(
                    "No items found at "
                    << cut_range << " "
                    << otio::ErrorStatus::outcome_to_string(
                           errorStatus.outcome));
                continue;
            }
            auto cut_item = items.front();
            auto item_range = cut_item->trimmed_range();
            auto track_range =
                cut_item->trimmed_range_in_parent(&errorStatus).value();
            if (item_range.duration() != one_frame)
            {
                if (track_range.start_time() != time)
                {
                    // Cut at time frame
                    otio::algo::slice(composition, time);
                }
                const RationalTime out_time = time + one_frame;
                if (track_range.end_time_exclusive() != out_time)
                {
                    // Cut at time + 1 frame
                    otio::algo::slice(composition, out_time);
                }
            }

            // Get the cut frame
            items = composition->find_children<Item>(&errorStatus, cut_range);
            if (items.empty())
            {
                LOG_ERROR(
                    "No cut frame found at "
                    << cut_range << " "
                    << otio::ErrorStatus::outcome_to_string(
                           errorStatus.outcome));
                continue;
            }
            for (const auto& cut_item : items)
            {
                item_range = cut_item->trimmed_range();

                // Only remove the one frame clip
                if (item_range.duration() > one_frame)
                    continue;

                int index = composition->index_of_child(cut_item);
                auto children_size = composition->children().size();
                if (index < 0 || static_cast<size_t>(index) >= children_size)
                    continue;
                composition->remove_child(index);
            }
        }
        player->setTimeline(timeline);

        // Set the end frame in the
        auto startFrame = RationalTime(0.0, time.rate());
        if (timeline->global_start_time())
            startFrame = timeline->global_start_time().value();
        auto endFrame = startFrame + timeline->duration();
        TimelineClass* c = ui->uiTimeWindow;
        c->uiEndFrame->setTime(endFrame);
    }

    void edit_paste_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        auto timeline = player->getTimeline();
        const auto time =
            player->currentTime() - player->timeRange().start_time();

        player->setTimeline(nullptr);
        for (auto& frame : copiedFrames)
        {
            TimeRange range(time, RationalTime(1.0, time.rate()));
            auto item = dynamic_cast<Item*>(frame.item->clone());
            if (!item)
                continue;
            const auto composition = frame.composition;
            otio::algo::overwrite(item, composition, range);
            frame.item = item;
        }

        player->setTimeline(timeline);
    }

    void edit_insert_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player || copiedFrames.empty())
            return;

        auto timeline = player->getTimeline();
        const auto time =
            player->currentTime() - player->timeRange().start_time();

        player->setTimeline(nullptr);
        for (auto& frame : copiedFrames)
        {
            auto item = dynamic_cast<Item*>(frame.item->clone());
            if (!item)
                continue;
            const auto composition = frame.composition;
            otio::algo::insert(item, composition, time);
            frame.item = item;
        }

        player->setTimeline(timeline);

        // Set the end frame in the
        auto startFrame = RationalTime(0.0, time.rate());
        if (timeline->global_start_time())
            startFrame = timeline->global_start_time().value();
        auto endFrame = startFrame + timeline->duration();
        TimelineClass* c = ui->uiTimeWindow;
        c->uiEndFrame->setTime(endFrame);
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
            H = calculate_edit_viewport_size(ui);
            editMode = EditMode::kSaved;
            editModeH = viewH = H;
            timeline->show();
            if (ui->uiMain->visible())
                ui->uiTimeline->show();
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
