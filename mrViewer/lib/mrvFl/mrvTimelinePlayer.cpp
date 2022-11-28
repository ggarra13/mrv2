// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include <mrvFl/mrvTimelinePlayer.h>

#include <tlCore/Math.h>
#include <tlCore/Time.h>

#include <mrvFl/mrvToolsCallbacks.h>
#include <mrvDraw/Annotation.h>
#include <mrvGL/mrvTimelineViewport.h>

#include <FL/Fl.H>

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "timelineplayer";
}

namespace
{
    const double kTimeout = 0.005;
}



namespace mrv
{
    struct TimelinePlayer::Private
    {
        std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;

        std::shared_ptr<observer::ValueObserver<double> > speedObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
        std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
        std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
        std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
        std::shared_ptr<observer::ValueObserver<uint16_t> > videoLayerObserver;
        std::shared_ptr<observer::ValueObserver<timeline::VideoData> > videoObserver;
        std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
        std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
        std::shared_ptr<observer::ValueObserver<double> > audioOffsetObserver;
        std::shared_ptr<observer::ValueObserver<otime::RationalTime> > cacheReadAheadObserver;
        std::shared_ptr<observer::ValueObserver<otime::RationalTime> > cacheReadBehindObserver;
        std::shared_ptr<observer::ValueObserver<float> > cachePercentageObserver;
        std::shared_ptr<observer::ListObserver<otime::TimeRange> > cachedVideoFramesObserver;
        std::shared_ptr<observer::ListObserver<otime::TimeRange> > cachedAudioFramesObserver;
        
        //! List of annotations ( drawings/text per frame )
        std::vector<std::shared_ptr<draw::Annotation> > annotations;
        
        //! Last annotation undone
        std::shared_ptr<draw::Annotation > undoAnnotation = nullptr;
    };

    void TimelinePlayer::_init(
        const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();


        p.timelinePlayer = timelinePlayer;

        p.speedObserver = observer::ValueObserver<double>::create(
            p.timelinePlayer->observeSpeed(),
            [this](double value)
            {
                speedChanged(value);
            });

        p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
            p.timelinePlayer->observePlayback(),
            [this](timeline::Playback value)
            {
                playbackChanged(value);
            });

        p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
            p.timelinePlayer->observeLoop(),
            [this](timeline::Loop value)
            {
                loopChanged(value);
            });

        p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
            p.timelinePlayer->observeCurrentTime(),
            [this](const otime::RationalTime& value)
            {
                currentTimeChanged(value);
            });

        p.inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
            p.timelinePlayer->observeInOutRange(),
            [this](const otime::TimeRange value)
            {
                inOutRangeChanged(value);
            });

        p.videoLayerObserver = observer::ValueObserver<uint16_t>::create(
            p.timelinePlayer->observeVideoLayer(),
            [this](uint16_t value)
            {
                videoLayerChanged(value);
            });

        p.videoObserver = observer::ValueObserver<timeline::VideoData>::create(
            p.timelinePlayer->observeVideo(),
            [this](const timeline::VideoData& value)
            {
                videoChanged(value);
            });

        p.volumeObserver = observer::ValueObserver<float>::create(
            p.timelinePlayer->observeVolume(),
            [this](float value)
            {
                volumeChanged(value);
            });

        p.muteObserver = observer::ValueObserver<bool>::create(
            p.timelinePlayer->observeMute(),
            [this](bool value)
            {
                muteChanged(value);
            });

        p.audioOffsetObserver = observer::ValueObserver<double>::create(
            p.timelinePlayer->observeAudioOffset(),
            [this](double value)
            {
                audioOffsetChanged(value);
            });

        p.cacheReadAheadObserver = observer::ValueObserver<otime::RationalTime>::create(
            p.timelinePlayer->observeCacheReadAhead(),
            [this](const otime::RationalTime& value)
            {
                cacheReadAheadChanged(value);
            });

        p.cacheReadBehindObserver = observer::ValueObserver<otime::RationalTime>::create(
            p.timelinePlayer->observeCacheReadBehind(),
            [this](const otime::RationalTime& value)
            {
                cacheReadBehindChanged(value);
            });

        p.cachePercentageObserver = observer::ValueObserver<float>::create(
            p.timelinePlayer->observeCachePercentage(),
            [this](float value)
            {
                cachePercentageChanged(value);
            });

        p.cachedVideoFramesObserver = observer::ListObserver<otime::TimeRange>::create(
            p.timelinePlayer->observeCachedVideoFrames(),
            [this](const std::vector<otime::TimeRange>& value)
            {
                cachedVideoFramesChanged(value);
            });

        p.cachedAudioFramesObserver = observer::ListObserver<otime::TimeRange>::create(
            p.timelinePlayer->observeCachedAudioFrames(),
            [this](const std::vector<otime::TimeRange>& value)
            {

                cachedAudioFramesChanged(value);

            });

        Fl::add_timeout( kTimeout, (Fl_Timeout_Handler) timerEvent_cb,
                         this );
    }

    TimelinePlayer::TimelinePlayer(
        const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
        const std::shared_ptr<system::Context>& context) :
        _p(new Private)
    {
        _init(timelinePlayer, context);
    }

    TimelinePlayer::~TimelinePlayer()
    {
        Fl::remove_timeout( (Fl_Timeout_Handler) timerEvent_cb, this );
    }

    const std::weak_ptr<system::Context>& TimelinePlayer::context() const
    {
        return _p->timelinePlayer->getContext();
    }

    const std::shared_ptr<timeline::TimelinePlayer>& TimelinePlayer::timelinePlayer() const
    {
        return _p->timelinePlayer;
    }

    const std::shared_ptr<timeline::Timeline>& TimelinePlayer::timeline() const
    {
        return _p->timelinePlayer->getTimeline();
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

    const timeline::VideoData& TimelinePlayer::video() const
    {
        return _p->timelinePlayer->observeVideo()->get();
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

    otime::RationalTime TimelinePlayer::cacheReadAhead() const
    {
        return _p->timelinePlayer->observeCacheReadAhead()->get();
    }

    otime::RationalTime TimelinePlayer::cacheReadBehind() const
    {
        return _p->timelinePlayer->observeCacheReadBehind()->get();
    }

    float TimelinePlayer::cachePercentage() const
    {
        return _p->timelinePlayer->observeCachePercentage()->get();
    }

    const std::vector<otime::TimeRange>& TimelinePlayer::cachedVideoFrames() const
    {
        return _p->timelinePlayer->observeCachedVideoFrames()->get();
    }

    const std::vector<otime::TimeRange>& TimelinePlayer::cachedAudioFrames() const
    {
        return _p->timelinePlayer->observeCachedAudioFrames()->get();
    }

    void TimelinePlayer::setSpeed(double value)
    {
        _p->timelinePlayer->setSpeed(value);
    }

    void TimelinePlayer::setPlayback(timeline::Playback value)
    {
        _p->timelinePlayer->setPlayback(value);

        // We must not remove the timeout on stop as then changing frames
        // would not work.
        int ok = Fl::has_timeout( (Fl_Timeout_Handler) timerEvent_cb, this );
        DBGM1( this << " view= " << timelineViewport << " " << path().get() );
        if ( value == timeline::Playback::Stop )
        {
            if ( filesTool )       filesTool->redraw();
            if ( compareTool ) compareTool->redraw();
        }
    }

    void TimelinePlayer::stop()
    {
        setPlayback(timeline::Playback::Stop);
    }

    void TimelinePlayer::forward()
    {
        _p->timelinePlayer->setPlayback(timeline::Playback::Forward);
    }

    void TimelinePlayer::reverse()
    {
        _p->timelinePlayer->setPlayback(timeline::Playback::Reverse);
    }

    void TimelinePlayer::togglePlayback()
    {
        setPlayback(
            timeline::Playback::Stop == _p->timelinePlayer->observePlayback()->get() ?
            timeline::Playback::Forward :
            timeline::Playback::Stop);
    }

    void TimelinePlayer::setLoop(timeline::Loop value)
    {
        _p->timelinePlayer->setLoop(value);
    }

    void TimelinePlayer::seek(const otime::RationalTime& value)
    {
        _p->timelinePlayer->seek(value);
        if ( filesTool )   filesTool->redraw();
        if ( compareTool ) compareTool->redraw();
    }

    void TimelinePlayer::timeAction(timeline::TimeAction value)
    {
        _p->timelinePlayer->timeAction(value);
    }

    void TimelinePlayer::start()
    {
        _p->timelinePlayer->start();
        if ( filesTool )   filesTool->redraw();
        if ( compareTool ) compareTool->redraw();
    }

    void TimelinePlayer::end()
    {
        _p->timelinePlayer->end();
        if ( filesTool )   filesTool->redraw();
        if ( compareTool ) compareTool->redraw();
    }

    void TimelinePlayer::framePrev()
    {
        _p->timelinePlayer->framePrev();
        if ( filesTool )   filesTool->redraw();
        if ( compareTool ) compareTool->redraw();
    }

    void TimelinePlayer::frameNext()
    {
        _p->timelinePlayer->frameNext();
        if ( filesTool )   filesTool->redraw();
        if ( compareTool ) compareTool->redraw();
    }

    void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
    {
        _p->timelinePlayer->setInOutRange(value);
    }

    void TimelinePlayer::setInPoint()
    {
        _p->timelinePlayer->setInPoint();
    }

    void TimelinePlayer::resetInPoint()
    {
        _p->timelinePlayer->resetInPoint();
    }

    void TimelinePlayer::setOutPoint()
    {
        _p->timelinePlayer->setOutPoint();
    }

    void TimelinePlayer::resetOutPoint()
    {
        _p->timelinePlayer->resetOutPoint();
    }

    void TimelinePlayer::setVideoLayer(int value)
    {
        _p->timelinePlayer->setVideoLayer(math::clamp(value, 0, static_cast<int>(std::numeric_limits<uint16_t>::max())));
    }

    void TimelinePlayer::setVolume(float value)
    {
        _p->timelinePlayer->setVolume(value);
    }

    void TimelinePlayer::increaseVolume()
    {
        _p->timelinePlayer->increaseVolume();
    }

    void TimelinePlayer::decreaseVolume()
    {
        _p->timelinePlayer->decreaseVolume();
    }

    void TimelinePlayer::setMute(bool value)
    {
        _p->timelinePlayer->setMute(value);
    }

    void TimelinePlayer::setAudioOffset(double value)
    {
        _p->timelinePlayer->setAudioOffset(value);
    }

    void TimelinePlayer::setCacheReadAhead(const otime::RationalTime& value)
    {
        _p->timelinePlayer->setCacheReadAhead(value);
    }

    void TimelinePlayer::setCacheReadBehind(const otime::RationalTime& value)
    {
        _p->timelinePlayer->setCacheReadBehind(value);
    }

    void TimelinePlayer::setTimelineViewport( TimelineViewport* view )
    {
        timelineViewport = view;
        DBGM1( this << " CHANGED VIEWPORT TO " << timelineViewport
               << " " << path().get() );
    }

    void TimelinePlayer::setSecondaryViewport( TimelineViewport* view )
    {
        secondaryViewport = view;
    }

    //! \name Playback
    ///@{

    //! This signal is emitted when the playback speed is changed.
    void TimelinePlayer::speedChanged(double) { }

    //! This signal is emitted when the playback mode is changed.
    void TimelinePlayer::playbackChanged(tl::timeline::Playback) { }

    //! This signal is emitted when the playback loop mode is changed.
    void TimelinePlayer::loopChanged(tl::timeline::Loop) { }

    //! This signal is emitted when the current time is changed.
    void TimelinePlayer::currentTimeChanged(const otime::RationalTime&) { }

    //! This signal is emitted when the in/out points range is changed.
    void TimelinePlayer::inOutRangeChanged(const otime::TimeRange&) { }

    ///@}

    //! \name Video
    ///@{

    //! This signal is emitted when the current video layer is changed.
    void TimelinePlayer::videoLayerChanged(int) { }

    //! This signal is emitted when the video is changed.
    void TimelinePlayer::videoChanged(const tl::timeline::VideoData& v)
    {
        if ( ! timelineViewport ) return;

        timelineViewport->videoCallback( v, this );
        if ( secondaryViewport ) secondaryViewport->videoCallback( v, this );
    }

    ///@}

    //! \name Audio
    ///@{

    //! This signal is emitted when the audio volume is changed.
    void TimelinePlayer::volumeChanged(float) { }

    //! This signal is emitted when the audio mute is changed.
    void TimelinePlayer::muteChanged(bool) { }

    //! This signal is emitted when the audio sync offset is changed.
    void TimelinePlayer::audioOffsetChanged(double) { }

    ///@}

    //! \name Cache
    ///@{

    //! This signal is emitted when the cache read ahead has changed.
    void TimelinePlayer::cacheReadAheadChanged(const otime::RationalTime&) { }

    //! This signal is emitted when the cache read behind has changed.
    void TimelinePlayer::cacheReadBehindChanged(const otime::RationalTime&) { }

    //! This signal is emitted when the cache percentage has changed.
    void TimelinePlayer::cachePercentageChanged(float) { }

    //! This signal is emitted when the cached video frames are changed.
    void TimelinePlayer::cachedVideoFramesChanged(const std::vector<otime::TimeRange>&) { }

    //! This signal is emitted when the cached audio frames are changed.
    void TimelinePlayer::cachedAudioFramesChanged(const std::vector<otime::TimeRange>&) { }

    const std::vector< int64_t >
    TimelinePlayer::getAnnotationFrames() const
    {
        TLRENDER_P();

        std::vector< int64_t > frames;
        for ( auto annotation : p.annotations )
        {
            frames.push_back( annotation->frame() );
        }
        return frames;
    }

    std::vector< std::shared_ptr< tl::draw::Annotation > >
    TimelinePlayer::getAnnotations(
        const int previous,
        const int next ) const
    {
        TLRENDER_P();

        auto    time = currentTime();
        int64_t frame = time.value();
        
        std::vector< std::shared_ptr< tl::draw::Annotation > > annotations;
        
        auto found = p.annotations.begin();

        while ( found != p.annotations.end() )
        {
            found =
                std::find_if( found, p.annotations.end(),
                              [frame, previous, next]( const auto& a ) {
                                  if ( a->allFrames() ) return true;
                                  int start = a->frame() - previous;
                                  int end   = a->frame() + next;
                                  return ( frame > start && frame < end );
                              } );
            
            if ( found != p.annotations.end() )
            {
                annotations.push_back( *found );
                ++found;
            }
        }
        return annotations;
    }
    
    std::shared_ptr< tl::draw::Annotation >
    TimelinePlayer::getAnnotation()
    {
        TLRENDER_P();
        
        //! Don't allow getting annotations while playing
        if ( playback() != timeline::Playback::Stop )
            return nullptr;

        auto time = currentTime();
        int64_t frame = time.value();
        
        auto found =  std::find_if( p.annotations.begin(),
                                    p.annotations.end(),
                                    [frame]( const auto& a ) {
                                        return a->frame() == frame;
                                    } );
        
        if ( found == p.annotations.end() )
        {
            return nullptr;
        }
        else
        {
            return *found;
        }
                    
    }

    
    std::shared_ptr< tl::draw::Annotation >
    TimelinePlayer::createAnnotation( const bool all_frames )
    {
        TLRENDER_P();

        //! Don't allow creating annotations while playing
        if ( playback() != timeline::Playback::Stop )
            return nullptr;

        auto time = currentTime();
        int64_t frame = time.value();
        
        auto found = std::find_if( p.annotations.begin(),
                                   p.annotations.end(),
                                   [frame]( const auto& a ) {
                                       return a->frame() == frame;
                                   } );
        
        if ( found == p.annotations.end() )
        {
            auto annotation =
                std::make_shared< draw::Annotation >(frame, all_frames);
            p.annotations.push_back( annotation );
            return annotation;
        }
        else
        {
            auto annotation = *found;
            if ( ! annotation->allFrames() && ! all_frames )
                throw std::runtime_error( _("Annotation already existed at this time" ) );
            return annotation;
        }
                    
    }

    void
    TimelinePlayer::undoAnnotation()
    {
        TLRENDER_P();
        
        auto annotation = getAnnotation();
        if ( !annotation ) return;
        
        annotation->undo();
        if ( annotation->empty() )
        {
            p.undoAnnotation = annotation;
            // If no shapes we remote the annotation too
            p.annotations.erase(std::remove(p.annotations.begin(),
                                            p.annotations.end(), annotation),
                                p.annotations.end());
        }
    }

    void
    TimelinePlayer::redoAnnotation()
    {
        TLRENDER_P();
        
        auto annotation = getAnnotation();
        if ( !annotation )
        {
            if ( p.undoAnnotation )
            {
                annotation = p.undoAnnotation;
                p.annotations.push_back( annotation );
                p.undoAnnotation.reset();
            }
        }
        if ( !annotation ) return;
        annotation->redo();
    }
    
    void TimelinePlayer::timerEvent()
    {
        _p->timelinePlayer->tick();
        Fl::repeat_timeout( kTimeout, (Fl_Timeout_Handler) timerEvent_cb,
                            this );
    }

    void TimelinePlayer::timerEvent_cb( void* d )
    {
        TimelinePlayer* t = static_cast< TimelinePlayer* >( d );
        t->timerEvent();
    }

}
