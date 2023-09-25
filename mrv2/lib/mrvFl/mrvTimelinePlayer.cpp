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

#ifdef TLRENDER_GL
#    include "mrvGL/mrvTimelineViewport.h"
#endif

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "player";
}

namespace
{
    const double kTimeout = 0.005;
}

namespace mrv
{
    namespace
    {
    } // namespace

    struct TimelinePlayer::Private
    {
        std::shared_ptr<timeline::Player> timelinePlayer;

        std::shared_ptr<observer::ValueObserver<double> > speedObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Playback> >
            playbackObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
        std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
            currentTimeObserver;
        std::shared_ptr<observer::ValueObserver<otime::TimeRange> >
            inOutRangeObserver;
        std::shared_ptr<observer::ValueObserver<size_t> > videoLayerObserver;
        std::shared_ptr<observer::ValueObserver<timeline::VideoData> >
            currentVideoObserver;
        std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
        std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
        std::shared_ptr<observer::ValueObserver<double> > audioOffsetObserver;
        std::shared_ptr<observer::ListObserver<timeline::AudioData> >
            currentAudioObserver;
        std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheOptions> >
            cacheOptionsObserver;
        std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
            cacheInfoObserver;

        //! Measuring timer
#ifdef DEBUG_SPEED
        std::chrono::time_point<std::chrono::steady_clock> start_time;
#endif

        //! List of annotations ( drawings/text per time )
        std::vector<std::shared_ptr<draw::Annotation> > annotations;

        //! Last annotation undone
        std::shared_ptr<draw::Annotation > undoAnnotation = nullptr;
    };

    void TimelinePlayer::_init(
        const std::shared_ptr<timeline::Player>& timelinePlayer,
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        p.timelinePlayer = timelinePlayer;

        p.speedObserver = observer::ValueObserver<double>::create(
            p.timelinePlayer->observeSpeed(),
            [this](double value) { speedChanged(value); });

        p.playbackObserver =
            observer::ValueObserver<timeline::Playback>::create(
                p.timelinePlayer->observePlayback(),
                [this](timeline::Playback value) { playbackChanged(value); });

        p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
            p.timelinePlayer->observeLoop(),
            [this](timeline::Loop value) { loopChanged(value); });

        p.currentTimeObserver =
            observer::ValueObserver<otime::RationalTime>::create(
                p.timelinePlayer->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                { currentTimeChanged(value); });

        p.inOutRangeObserver =
            observer::ValueObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeInOutRange(),
                [this](const otime::TimeRange value)
                { inOutRangeChanged(value); });

        p.videoLayerObserver = observer::ValueObserver<size_t>::create(
            p.timelinePlayer->observeVideoLayer(),
            [this](size_t value) { videoLayerChanged(value); });

        p.currentVideoObserver =
            observer::ValueObserver<timeline::VideoData>::create(
                p.timelinePlayer->observeCurrentVideo(),
                [this](const timeline::VideoData& value)
                { currentVideoChanged(value); },
                observer::CallbackAction::Suppress);

        p.volumeObserver = observer::ValueObserver<float>::create(
            p.timelinePlayer->observeVolume(),
            [this](float value) { volumeChanged(value); });

        p.muteObserver = observer::ValueObserver<bool>::create(
            p.timelinePlayer->observeMute(),
            [this](bool value) { muteChanged(value); });

        p.audioOffsetObserver = observer::ValueObserver<double>::create(
            p.timelinePlayer->observeAudioOffset(),
            [this](double value) { audioOffsetChanged(value); });

        p.currentAudioObserver =
            observer::ListObserver<timeline::AudioData>::create(
                p.timelinePlayer->observeCurrentAudio(),
                [this](const std::vector<timeline::AudioData>& value)
                { currentAudioChanged(value); });

        p.cacheOptionsObserver =
            observer::ValueObserver<timeline::PlayerCacheOptions>::create(
                p.timelinePlayer->observeCacheOptions(),
                [this](const timeline::PlayerCacheOptions& value)
                { cacheOptionsChanged(value); });

        p.cacheInfoObserver =
            observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.timelinePlayer->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                { cacheInfoChanged(value); });

#ifdef DEBUG_SPEED
        p.start_time = std::chrono::steady_clock::now();
#endif

        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    TimelinePlayer::TimelinePlayer(
        const std::shared_ptr<timeline::Player>& timelinePlayer,
        const std::shared_ptr<system::Context>& context) :
        _p(new Private)
    {
        _init(timelinePlayer, context);
    }

    TimelinePlayer::~TimelinePlayer()
    {
        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
    }

    const std::weak_ptr<system::Context>& TimelinePlayer::context() const
    {
        return _p->timelinePlayer->getContext();
    }

    const std::shared_ptr<timeline::Player>& TimelinePlayer::player() const
    {
        return _p->timelinePlayer;
    }

    const std::shared_ptr<timeline::Timeline>& TimelinePlayer::timeline() const
    {
        return _p->timelinePlayer->getTimeline();
    }

    const otio::SerializableObject::Retainer<otio::Timeline>&
    TimelinePlayer::getTimeline() const
    {
        return _p->timelinePlayer->getTimeline()->getTimeline();
    }

    void TimelinePlayer::setTimeline(
        const otio::SerializableObject::Retainer<otio::Timeline>& timeline)
    {
        _p->timelinePlayer->getTimeline()->setTimeline(timeline);
    }

    const file::Path& TimelinePlayer::path() const
    {
        return _p->timelinePlayer->getPath();
    }

    const file::Path& TimelinePlayer::audioPath() const
    {
        return _p->timelinePlayer->getAudioPath();
    }

    const timeline::PlayerOptions& TimelinePlayer::getPlayerOptions() const
    {
        return _p->timelinePlayer->getPlayerOptions();
    }

    const timeline::Options& TimelinePlayer::getOptions() const
    {
        return _p->timelinePlayer->getOptions();
    }

    const otime::TimeRange& TimelinePlayer::timeRange() const
    {
        return _p->timelinePlayer->getTimeRange();
    }

    const tl::io::Info& TimelinePlayer::ioInfo() const
    {
        return _p->timelinePlayer->getIOInfo();
    }

    double TimelinePlayer::defaultSpeed() const
    {
        return _p->timelinePlayer->getDefaultSpeed();
    }

    double TimelinePlayer::speed() const
    {
        return _p->timelinePlayer->observeSpeed()->get();
    }

    timeline::Playback TimelinePlayer::playback() const
    {
        return _p->timelinePlayer->observePlayback()->get();
    }

    timeline::Loop TimelinePlayer::loop() const
    {
        return _p->timelinePlayer->observeLoop()->get();
    }

    const otime::RationalTime& TimelinePlayer::currentTime() const
    {
        return _p->timelinePlayer->observeCurrentTime()->get();
    }

    const otime::TimeRange& TimelinePlayer::inOutRange() const
    {
        return _p->timelinePlayer->observeInOutRange()->get();
    }

    int TimelinePlayer::videoLayer() const
    {
        return _p->timelinePlayer->observeVideoLayer()->get();
    }

    const timeline::VideoData& TimelinePlayer::currentVideo() const
    {
        return _p->timelinePlayer->observeCurrentVideo()->get();
    }

    float TimelinePlayer::volume() const
    {
        return _p->timelinePlayer->observeVolume()->get();
    }

    bool TimelinePlayer::isMuted() const
    {
        return _p->timelinePlayer->observeMute()->get();
    }

    double TimelinePlayer::audioOffset() const
    {
        return _p->timelinePlayer->observeAudioOffset()->get();
    }

    const std::vector<timeline::AudioData>& TimelinePlayer::currentAudio() const
    {
        return _p->timelinePlayer->observeCurrentAudio()->get();
    }

    const timeline::PlayerCacheOptions& TimelinePlayer::cacheOptions() const
    {
        return _p->timelinePlayer->observeCacheOptions()->get();
    }

    const timeline::PlayerCacheInfo& TimelinePlayer::cacheInfo() const
    {
        return _p->timelinePlayer->observeCacheInfo()->get();
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

        _p->timelinePlayer->setSpeed(value);
    }

    void TimelinePlayer::setPlayback(timeline::Playback value)
    {
        pushMessage("seek", currentTime());
        pushMessage("setPlayback", value);
        _p->timelinePlayer->setPlayback(value);

        if (value == timeline::Playback::Stop)
        {
            // Send a seek request to make sure we are at the right time
            pushMessage("seek", currentTime());
            redrawPanelThumbnails();
        }
    }

    void TimelinePlayer::stop()
    {
        setPlayback(timeline::Playback::Stop);
    }

    void TimelinePlayer::forward()
    {
        pushMessage("setPlayback", timeline::Playback::Forward);
        _p->timelinePlayer->setPlayback(timeline::Playback::Forward);
    }

    void TimelinePlayer::reverse()
    {
        pushMessage("setPlayback", timeline::Playback::Reverse);
        _p->timelinePlayer->setPlayback(timeline::Playback::Reverse);
    }

    void TimelinePlayer::togglePlayback()
    {
        setPlayback(
            timeline::Playback::Stop ==
                    _p->timelinePlayer->observePlayback()->get()
                ? timeline::Playback::Forward
                : timeline::Playback::Stop);
    }

    void TimelinePlayer::setLoop(timeline::Loop value)
    {
        Message m = value;
        pushMessage("setLoop", m);
        _p->timelinePlayer->setLoop(value);
    }

    void TimelinePlayer::seek(const otime::RationalTime& value)
    {
        pushMessage("seek", value);
        _p->timelinePlayer->seek(value);
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::timeAction(timeline::TimeAction value)
    {
        _p->timelinePlayer->timeAction(value);
    }

    void TimelinePlayer::start()
    {
        pushMessage("start", 0);
        _p->timelinePlayer->start();
        redrawPanelThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::end()
    {
        pushMessage("end", 0);
        _p->timelinePlayer->end();
        redrawPanelThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::framePrev()
    {
        pushMessage("framePrev", 0);
        _p->timelinePlayer->framePrev();
        redrawPanelThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::frameNext()
    {
        pushMessage("frameNext", 0);
        _p->timelinePlayer->frameNext();
        redrawPanelThumbnails();
        if (timelineViewport)
            timelineViewport->updateUndoRedoButtons();
    }

    void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
    {
        pushMessage("setInOutRange", value);
        _p->timelinePlayer->setInOutRange(value);
    }

    void TimelinePlayer::setInPoint()
    {
        pushMessage("setInPoint", 0);
        _p->timelinePlayer->setInPoint();
    }

    void TimelinePlayer::resetInPoint()
    {
        pushMessage("resetInPoint", 0);
        _p->timelinePlayer->resetInPoint();
    }

    void TimelinePlayer::setOutPoint()
    {
        pushMessage("setOutPoint", 0);
        _p->timelinePlayer->setOutPoint();
    }

    void TimelinePlayer::resetOutPoint()
    {
        pushMessage("resetOutPoint", 0);
        _p->timelinePlayer->resetOutPoint();
    }

    void TimelinePlayer::setVideoLayer(int value)
    {
        pushMessage("setVideoLayer", value);
        _p->timelinePlayer->setVideoLayer(math::clamp(
            value, 0, static_cast<int>(std::numeric_limits<int>::max())));
    }

    void TimelinePlayer::setVolume(float value)
    {
        _p->timelinePlayer->setVolume(value);
    }

    void TimelinePlayer::setMute(bool value)
    {
        _p->timelinePlayer->setMute(value);
    }

    void TimelinePlayer::setAudioOffset(double value)
    {
        bool send = App::ui->uiPrefs->SendAudio->value();
        if (send)
            tcp->pushMessage("setAudioOffset", value);
        _p->timelinePlayer->setAudioOffset(value);
    }

    void TimelinePlayer::setTimelineViewport(TimelineViewport* view)
    {
        timelineViewport = view;
    }

    void TimelinePlayer::setSecondaryViewport(TimelineViewport* view)
    {
        secondaryViewport = view;
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
    void TimelinePlayer::playbackChanged(tl::timeline::Playback value) {}

    //! This signal is emitted when the playback loop mode is changed.
    void TimelinePlayer::loopChanged(tl::timeline::Loop value)
    {
        TimelineClass* c = App::ui->uiTimeWindow;
        c->uiLoopMode->value(static_cast<int>(value));
        c->uiLoopMode->do_callback();
    }

    //! This signal is emitted when the current time is changed.
    void TimelinePlayer::currentTimeChanged(const otime::RationalTime& value) {}

    //! This signal is emitted when the in/out points range is changed.
    void TimelinePlayer::inOutRangeChanged(const otime::TimeRange& value) {}

    ///@}

    //! \name Video
    ///@{

    //! This signal is emitted when the current video layer is changed.
    void TimelinePlayer::videoLayerChanged(size_t) {}

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

    //! This signal is emitted when the video is changed.
    void TimelinePlayer::currentVideoChanged(const tl::timeline::VideoData& v)
    {
        if (!timelineViewport)
            return;

        timelineViewport->currentVideoCallback(v, this);
        if (secondaryViewport && secondaryViewport->visible_r())
        {
            secondaryViewport->currentVideoCallback(v, this);
        }
    }

    ///@}

    //! \name Audio
    ///@{

    //! This signal is emitted when the audio volume is changed.
    void TimelinePlayer::volumeChanged(float) {}

    //! This signal is emitted when the audio mute is changed.
    void TimelinePlayer::muteChanged(bool) {}

    //! This signal is emitted when the audio sync offset is changed.
    void TimelinePlayer::audioOffsetChanged(double) {}

    ///@}

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

        auto time = currentTime();

        otime::RationalTime previousTime(
            static_cast<double>(previous), time.rate());
        otime::RationalTime nextTime(static_cast<double>(next), time.rate());

        std::vector< std::shared_ptr< draw::Annotation > > annotations;

        auto found = p.annotations.begin();

        while (found != p.annotations.end())
        {
            found = std::find_if(
                found, p.annotations.end(),
                [time, previousTime, nextTime](const auto& a)
                {
                    if (a->allFrames)
                        return true;
                    otime::RationalTime start = a->time - previousTime;
                    otime::RationalTime end = a->time + nextTime;
                    return (time > start && time < end);
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
        _p->timelinePlayer->setCacheOptions(value);
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
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end_time - _p->start_time;
        std::cout << "timeout duration: " << diff.count() << std::endl;
        _p->start_time = std::chrono::steady_clock::now();
#endif
        _p->timelinePlayer->tick();
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void TimelinePlayer::timerEvent_cb(void* d)
    {
        TimelinePlayer* t = static_cast< TimelinePlayer* >(d);
        t->timerEvent();
    }

} // namespace mrv
