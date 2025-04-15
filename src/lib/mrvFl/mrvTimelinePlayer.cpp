// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvTimelinePlayer.h"

#include <tlCore/Math.h>
#include <tlCore/Time.h>

#include <FL/Fl.H>

#include "mrvCore/mrvMath.h"

#include "mrvDraw/Annotation.h"

#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvIO.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "player";
}

namespace
{
    const double kTimeout = 0.008;
}

namespace mrv
{
    namespace
    {
        struct StopData
        {
            TimelinePlayer* player;
            double speed;
            otio::RationalTime time;
        };

        void stop_playback_cb(StopData* data)
        {
            auto player = data->player;
            player->player()->setPlayback(timeline::Playback::Stop);
            player->seek(data->time);
            player->setSpeed(data->speed);
            panel::redrawThumbnails();
            delete data;
        }

    } // namespace

    struct TimelinePlayer::Private
    {
        std::shared_ptr<timeline::Player> player;

        std::shared_ptr<observer::ValueObserver<double> > speedObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Playback> >
            playbackObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
        std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
            currentTimeObserver;
        std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheOptions> >
            cacheOptionsObserver;
        std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
            cacheInfoObserver;

        bool isStepping = false;

        //! Measuring timer
#ifdef DEBUG_SPEED
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
#endif

        //! List of annotations ( drawings/text per time )
        std::vector<std::shared_ptr<draw::Annotation> > annotations;

        //! Last annotation undone
        std::shared_ptr<draw::Annotation > undoAnnotation = nullptr;
    };

    void TimelinePlayer::_init(
        const std::shared_ptr<timeline::Player>& player,
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        p.player = player;

        timeline::Loop loop = static_cast<timeline::Loop>(
            App::ui->uiPrefs->uiPrefsLoopMode->value());
        p.player->setLoop(loop);

        p.speedObserver = observer::ValueObserver<double>::create(
            p.player->observeSpeed(),
            [this](double value) { speedChanged(value); });

        p.playbackObserver =
            observer::ValueObserver<timeline::Playback>::create(
                p.player->observePlayback(),
                [this](timeline::Playback value) { playbackChanged(value); });

        p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
            p.player->observeLoop(),
            [this](timeline::Loop value) { loopChanged(value); });

        p.currentTimeObserver =
            observer::ValueObserver<otime::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                { currentTimeChanged(value); });

        p.cacheOptionsObserver =
            observer::ValueObserver<timeline::PlayerCacheOptions>::create(
                p.player->observeCacheOptions(),
                [this](const timeline::PlayerCacheOptions& value)
                { cacheOptionsChanged(value); });

        p.cacheInfoObserver =
            observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.player->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                { cacheInfoChanged(value); });

#ifdef DEBUG_SPEED
        p.start_time = std::chrono::high_resolution_clock::now();
#endif

        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    TimelinePlayer::TimelinePlayer(
        const std::shared_ptr<timeline::Player>& player,
        const std::shared_ptr<system::Context>& context) :
        _p(new Private)
    {
        _init(player, context);
    }

    TimelinePlayer::~TimelinePlayer()
    {
        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
    }

    const std::weak_ptr<system::Context>& TimelinePlayer::context() const
    {
        return _p->player->getContext();
    }

    const std::shared_ptr<timeline::Player>& TimelinePlayer::player() const
    {
        return _p->player;
    }

    const std::shared_ptr<timeline::Timeline>& TimelinePlayer::timeline() const
    {
        return _p->player->getTimeline();
    }

    const otio::SerializableObject::Retainer<otio::Timeline>&
    TimelinePlayer::getTimeline() const
    {
        return _p->player->getTimeline()->getTimeline();
    }

    void TimelinePlayer::setTimeline(
        const otio::SerializableObject::Retainer<otio::Timeline>& timeline)
    {
        _p->player->getTimeline()->setTimeline(timeline);
    }

    const file::Path& TimelinePlayer::path() const
    {
        return _p->player->getPath();
    }

    const file::Path& TimelinePlayer::audioPath() const
    {
        return _p->player->getAudioPath();
    }

    const timeline::PlayerOptions& TimelinePlayer::getPlayerOptions() const
    {
        return _p->player->getPlayerOptions();
    }

    const timeline::Options& TimelinePlayer::getOptions() const
    {
        return _p->player->getOptions();
    }

    const otime::TimeRange& TimelinePlayer::timeRange() const
    {
        return _p->player->getTimeRange();
    }

    const tl::io::Info& TimelinePlayer::ioInfo() const
    {
        return _p->player->getIOInfo();
    }

    double TimelinePlayer::defaultSpeed() const
    {
        return _p->player->getDefaultSpeed();
    }

    double TimelinePlayer::speed() const
    {
        return _p->player->observeSpeed()->get();
    }

    timeline::Playback TimelinePlayer::playback() const
    {
        return _p->player->observePlayback()->get();
    }

    timeline::Loop TimelinePlayer::loop() const
    {
        return _p->player->observeLoop()->get();
    }

    const otime::RationalTime& TimelinePlayer::currentTime() const
    {
        return _p->player->observeCurrentTime()->get();
    }

    const otime::TimeRange& TimelinePlayer::inOutRange() const
    {
        return _p->player->observeInOutRange()->get();
    }

    const std::vector<timeline::VideoData>& TimelinePlayer::currentVideo() const
    {
        return _p->player->getCurrentVideo();
    }

    float TimelinePlayer::volume() const
    {
        return _p->player->observeVolume()->get();
    }

    bool TimelinePlayer::isMuted() const
    {
        return _p->player->observeMute()->get();
    }

    double TimelinePlayer::audioOffset() const
    {
        return _p->player->observeAudioOffset()->get();
    }

    const std::vector<timeline::AudioData>& TimelinePlayer::currentAudio() const
    {
        return _p->player->observeCurrentAudio()->get();
    }

    const timeline::PlayerCacheOptions& TimelinePlayer::cacheOptions() const
    {
        return _p->player->observeCacheOptions()->get();
    }

    const timeline::PlayerCacheInfo& TimelinePlayer::cacheInfo() const
    {
        return _p->player->observeCacheInfo()->get();
    }

    void TimelinePlayer::updateVideoCache(const otime::RationalTime& time)
    {
        pushMessage("updateVideoCache", time);
        _p->player->updateVideoCache(time);
        panel::redrawThumbnails(true);
    }

    void TimelinePlayer::clearCache()
    {
        pushMessage("clearCache", 0);
        _p->player->clearCache();
    }

    template < typename T >
    void TimelinePlayer::pushMessage(const std::string& command, const T& value)
    {
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message = {{"command", command}, {"value", value}};
            tcp->pushMessage(message);
        }
    }

    void TimelinePlayer::setSpeed(double value)
    {
        pushMessage("setSpeed", value);

        _p->player->setSpeed(value);

        if (panel::imageInfoPanel)
            panel::imageInfoPanel->refresh();
    }

    void TimelinePlayer::setPlayback(timeline::Playback value)
    {
        pushMessage("seek", currentTime());
        pushMessage("setPlayback", value);
        _p->player->setPlayback(value);

        if (value == timeline::Playback::Stop)
        {
            _p->isStepping = false;
            // Send a seek request to make sure we are at the right time
            pushMessage("seek", currentTime());
            panel::redrawThumbnails();
        }
    }

    void TimelinePlayer::stop()
    {
        setPlayback(timeline::Playback::Stop);
    }

    void TimelinePlayer::forward()
    {
        pushMessage("setPlayback", timeline::Playback::Forward);
        _p->player->setPlayback(timeline::Playback::Forward);
    }

    void TimelinePlayer::reverse()
    {
        pushMessage("setPlayback", timeline::Playback::Reverse);
        _p->player->setPlayback(timeline::Playback::Reverse);
    }

    void TimelinePlayer::togglePlayback()
    {
        setPlayback(
            timeline::Playback::Stop == _p->player->observePlayback()->get()
                ? timeline::Playback::Forward
                : timeline::Playback::Stop);
    }

    void TimelinePlayer::setLoop(timeline::Loop value)
    {
        Message m = value;
        pushMessage("setLoop", m);
        _p->player->setLoop(value);
    }

    void TimelinePlayer::seek(const otime::RationalTime& value)
    {
        pushMessage("seek", value);
        _p->player->seek(value);
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::timeAction(timeline::TimeAction value)
    {
        _p->player->timeAction(value);
    }

    void TimelinePlayer::start()
    {
        pushMessage("start", 0);
        _p->player->start();
        panel::redrawThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::end()
    {
        pushMessage("end", 0);
        _p->player->end();
        panel::redrawThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    bool TimelinePlayer::isStepping() const
    {
        return _p->isStepping;
    }

    void TimelinePlayer::framePrev()
    {
        TLRENDER_P();

        pushMessage("framePrev", 0);

        if (isMuted())
            return p.player->framePrev();

        p.isStepping = true;
        const auto oneFrame =
            otime::RationalTime(1.0, timeRange().duration().rate());
        auto time = currentTime() - oneFrame;
        if (time <= inOutRange().start_time())
        {
            switch (loop())
            {
            case timeline::Loop::Once:
                time = currentTime();
                break;
            case timeline::Loop::PingPong:
                time = inOutRange().start_time() + oneFrame;
                break;
            case timeline::Loop::Loop:
            default:
                time = inOutRange().end_time_exclusive() - oneFrame;
                break;
            }
        }

        p.player->setPlayback(timeline::Playback::Reverse);
        StopData* data = new StopData;
        data->player = this;
        data->time = time;
        data->speed = speed();

        Fl::add_timeout(
            1.0 / speed(), (Fl_Timeout_Handler)stop_playback_cb, data);
    }

    void TimelinePlayer::frameNext()
    {
        TLRENDER_P();

        pushMessage("frameNext", 0);

        if (isMuted())
            return p.player->frameNext();

        p.isStepping = true;
        const auto oneFrame =
            otime::RationalTime(1.0, timeRange().duration().rate());
        auto time = currentTime() + oneFrame;

        if (time >= inOutRange().end_time_exclusive())
        {
            switch (loop())
            {
            case timeline::Loop::Once:
                time = currentTime();
                break;
            case timeline::Loop::PingPong:
                time = inOutRange().end_time_exclusive() - oneFrame;
                break;
            case timeline::Loop::Loop:
            default:
                time = inOutRange().start_time();
                break;
            }
        }

        p.player->setPlayback(timeline::Playback::Forward);
        StopData* data = new StopData;
        data->player = this;
        data->time = time;
        data->speed = speed();
        Fl::add_timeout(
            1.75 / speed(), (Fl_Timeout_Handler)stop_playback_cb, data);
    }

    void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
    {
        pushMessage("setInOutRange", value);
        _p->player->setInOutRange(value);
    }

    void TimelinePlayer::setInPoint()
    {
        pushMessage("setInPoint", 0);
        _p->player->setInPoint();
    }

    void TimelinePlayer::resetInPoint()
    {
        pushMessage("resetInPoint", 0);
        _p->player->resetInPoint();
    }

    void TimelinePlayer::setOutPoint()
    {
        pushMessage("setOutPoint", 0);
        _p->player->setOutPoint();
    }

    void TimelinePlayer::resetOutPoint()
    {
        pushMessage("resetOutPoint", 0);
        _p->player->resetOutPoint();
    }

    int TimelinePlayer::videoLayer() const
    {
        auto model = App::app->filesModel();
        auto Aitem = model->observeA()->get();
        if (!Aitem)
            return 0;
        return Aitem->videoLayer;
    }

    void TimelinePlayer::setIOOptions(const io::Options& value)
    {
        _p->player->setIOOptions(value);
    }

    void TimelinePlayer::setCompare(
        const std::vector<std::shared_ptr<timeline::Timeline> >& value)
    {
        _p->player->setCompare(value);
    }

    void TimelinePlayer::setCompareTime(timeline::CompareTimeMode value)
    {
        _p->player->setCompareTime(value);
    }

    void TimelinePlayer::setVideoLayer(int value)
    {
        pushMessage("setVideoLayer", value);
        _p->player->setVideoLayer(value);
    }

    void TimelinePlayer::setCompareVideoLayers(const std::vector<int>& value)
    {
        _p->player->setCompareVideoLayers(value);
    }

    void TimelinePlayer::setVolume(float value)
    {
        _p->player->setVolume(value);
    }

    void TimelinePlayer::setMute(bool value)
    {
        _p->player->setMute(value);
    }

    void TimelinePlayer::setAudioOffset(double value)
    {
        bool send = App::ui->uiPrefs->SendAudio->value();
        if (send)
            tcp->pushMessage("setAudioOffset", value);
        _p->player->setAudioOffset(value);
    }

    void TimelinePlayer::setTimelineViewport(MyViewport* view)
    {
        timelineViewport = view;
    }

    //! \name Playback
    ///@{

    //! This signal is emitted when the playback speed is changed.
    void TimelinePlayer::speedChanged(double fps)
    {
        TimelineClass* c = App::ui->uiTimeWindow;
        c->uiFPS->value(fps);
    }

    //! This signal is emitted when the playback mode is changed.
    void TimelinePlayer::playbackChanged(tl::timeline::Playback value)
    {
        if (timelineViewport)
            timelineViewport->updatePlaybackButtons();
    }

    //! This signal is emitted when the playback loop mode is changed.
    void TimelinePlayer::loopChanged(tl::timeline::Loop value)
    {
        TimelineClass* c = App::ui->uiTimeWindow;
        c->uiLoopMode->value(static_cast<int>(value));
        c->uiLoopMode->do_callback();
    }

    //! This signal is emitted when the current time is changed.
    void TimelinePlayer::currentTimeChanged(const otime::RationalTime& value)
    {
        auto timeline = App::ui->uiTimeline;
        timeline->redraw();
        TimelineClass* c = App::ui->uiTimeWindow;
        c->uiFrame->setTime(value);
    }

    ///@}

    //! \name Video
    ///@{

    //! This signal is emitted when the cache options have changed.
    void
    TimelinePlayer::cacheOptionsChanged(const tl::timeline::PlayerCacheOptions&)
    {
        if (!timelineViewport)
            return;
        timelineViewport->cacheChangedCallback();
    }

    //! This signal is emitted when the cache information has changed.
    void TimelinePlayer::cacheInfoChanged(const tl::timeline::PlayerCacheInfo&)
    {
        if (!timelineViewport)
            return;
        timelineViewport->cacheChangedCallback();
    }

    bool TimelinePlayer::hasAnnotations() const
    {
        return !_p->annotations.empty();
    }

    const std::vector< otime::RationalTime >
    TimelinePlayer::getAnnotationTimes() const
    {
        TLRENDER_P();

        std::vector< otime::RationalTime > times;
        for (auto annotation : p.annotations)
        {
            times.push_back(annotation->time);
        }
        return times;
    }

    std::vector< std::shared_ptr< draw::Annotation > >
    TimelinePlayer::getAnnotations(const int previous, const int next) const
    {
        TLRENDER_P();

        const auto& time = currentTime();
        const int64_t frame = time.value();

        std::vector< std::shared_ptr< draw::Annotation > > annotations;

        auto found = p.annotations.begin();

        while (found != p.annotations.end())
        {
            found = std::find_if(
                found, p.annotations.end(),
                [frame, previous, next](const auto& a)
                {
                    if (a->allFrames)
                        return true;
                    const int64_t start = frame - previous;
                    const int64_t end = frame + next;
                    return (frame > start && frame < end);
                });

            if (found != p.annotations.end())
            {
                annotations.push_back(*found);
                ++found;
            }
        }
        return annotations;
    }

    std::shared_ptr< draw::Annotation > TimelinePlayer::getAnnotation() const
    {
        TLRENDER_P();

        //! Don't allow getting annotations while playing
        if (playback() != timeline::Playback::Stop)
            return nullptr;

        const auto& time = currentTime();

        const auto found = std::find_if(
            p.annotations.begin(), p.annotations.end(),
            [time](const auto& a) { return a->time == time; });

        if (found == p.annotations.end())
        {
            return nullptr;
        }
        else
        {
            return *found;
        }
    }

    std::shared_ptr< draw::Annotation >
    TimelinePlayer::createAnnotation(const bool all_frames)
    {
        TLRENDER_P();

        // Don't allow creating annotations while playing.  Stop playback first.
        if (playback() != timeline::Playback::Stop)
        {
            stop();
        }

        auto time = currentTime();

        auto found = std::find_if(
            p.annotations.begin(), p.annotations.end(),
            [time](const auto& a) { return a->time == time; });

        if (found == p.annotations.end())
        {
            auto annotation =
                std::make_shared< draw::Annotation >(time, all_frames);
            p.annotations.push_back(annotation);
            bool send = App::ui->uiPrefs->SendAnnotations->value();
            if (send)
                tcp->pushMessage("Create Annotation", all_frames);
            return annotation;
        }
        else
        {
            auto annotation = *found;
            if (!annotation->allFrames && !all_frames)
            {
                throw std::runtime_error(
                    _("Annotation already existed at this time"));
            }
            return annotation;
        }
    }

    //! Get all annotations in timeline player
    std::vector< std::shared_ptr< draw::Annotation >>
    TimelinePlayer::getAllAnnotations() const
    {
        return _p->annotations;
    }

    void TimelinePlayer::setAllAnnotations(
        const std::vector< std::shared_ptr< draw::Annotation >>& value)
    {
        _p->annotations = value;
    }

    void TimelinePlayer::clearFrameAnnotation()
    {
        TLRENDER_P();

        const auto& time = currentTime();

        auto found = std::find_if(
            p.annotations.begin(), p.annotations.end(),
            [time](const auto& a) { return a->time == time; });

        if (found != p.annotations.end())
        {
            p.annotations.erase(found);
        }
    }

    void TimelinePlayer::clearAllAnnotations()
    {
        _p->annotations.clear();
    }

    void TimelinePlayer::removeAnnotation(
        const std::shared_ptr< draw::Annotation >& annotation)
    {
        TLRENDER_P();

        p.annotations.erase(
            std::remove(p.annotations.begin(), p.annotations.end(), annotation),
            p.annotations.end());
    }

    void TimelinePlayer::undoAnnotation()
    {
        TLRENDER_P();

        auto annotation = getAnnotation();
        if (!annotation)
            return;

        annotation->undo();
        if (annotation->empty())
        {
            p.undoAnnotation = annotation;
            // If no shapes we remove the annotation too
            removeAnnotation(annotation);
        }
    }

    void TimelinePlayer::redoAnnotation()
    {
        TLRENDER_P();

        auto annotation = getAnnotation();
        if (!annotation)
        {
            if (p.undoAnnotation)
            {
                annotation = p.undoAnnotation;
                p.annotations.push_back(annotation);
                p.undoAnnotation.reset();
            }
        }
        if (!annotation)
            return;
        annotation->redo();
    }

    void
    TimelinePlayer::setCacheOptions(const timeline::PlayerCacheOptions& value)
    {
        _p->player->setCacheOptions(value);
    }

    bool TimelinePlayer::hasUndo() const
    {
        TLRENDER_P();
        auto annotation = getAnnotation();
        if (!annotation)
            return false;
        return true;
    }

    bool TimelinePlayer::hasRedo() const
    {
        TLRENDER_P();

        auto annotation = getAnnotation();
        if (!annotation)
        {
            if (p.undoAnnotation)
            {
                annotation = p.undoAnnotation;
            }
        }
        if (!annotation)
            return false;

        return !annotation->undo_shapes.empty();
    }

    void TimelinePlayer::timerEvent()
    {
#ifdef DEBUG_SPEED
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end_time - _p->start_time;
        std::cout << "timeout duration: " << diff.count() << std::endl;
        _p->start_time = std::chrono::high_resolution_clock::now();
#endif
        _p->player->tick();
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void TimelinePlayer::timerEvent_cb(void* d)
    {
        TimelinePlayer* t = static_cast< TimelinePlayer* >(d);
        t->timerEvent();
    }

} // namespace mrv
