// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Audio.h>

#include <tlTimeline/Timeline.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace draw
    {
        class Annotation;
    }
} // namespace tl

namespace mrv
{

    using namespace tl;

    class TimelineViewport;

    //! FLTK based timeline player.
    class TimelinePlayer
    {

        void _init(
            const std::shared_ptr<timeline::Player>&,
            const std::shared_ptr<system::Context>&);

    public:
        TimelinePlayer(
            const std::shared_ptr<timeline::Player>&,
            const std::shared_ptr<system::Context>&);

        ~TimelinePlayer();

        //! Get the context.
        const std::weak_ptr<system::Context>& context() const;

        //! Get the timeline player.
        const std::shared_ptr<timeline::Player>& player() const;

        //! Get the timeline.
        const std::shared_ptr<timeline::Timeline>& timeline() const;

        //! Get the path.
        const file::Path& path() const;

        //! Get the audio path.
        const file::Path& audioPath() const;

        //! Get the timeline player options.
        const timeline::PlayerOptions& getPlayerOptions() const;

        //! Get the timeline options.
        const timeline::Options& getOptions() const;

        //! \name Information
        ///@{

        //! Get the time range.
        const otime::TimeRange& timeRange() const;

        //! Get the I/O information. This information is retreived from
        //! the first clip in the timeline.
        const tl::io::Info& ioInfo() const;

        ///@}

        //! \name Playback
        ///@{

        //! Get the default playback speed.
        double defaultSpeed() const;

        //! Get the playback speed.
        double speed() const;

        //! Get the playback mode.
        timeline::Playback playback() const;

        //! Get the playback loop mode.
        timeline::Loop loop() const;

        ///@}

        //! \name Time
        ///@{

        //! Get the current time.
        const otime::RationalTime& currentTime() const;

        ///@}

        //! \name In/Out Points
        ///@{

        //! Get the in/out points range.
        const otime::TimeRange& inOutRange() const;

        ///@}

        //! \name Video
        ///@{

        //! Get the current video layer.
        int videoLayer() const;

        //! Get the video.
        const timeline::VideoData& currentVideo() const;

        ///@}

        //! \name Cache
        ///@{

        //! Get the cache options.
        const timeline::PlayerCacheOptions& cacheOptions() const;

        //! Get the cache information.
        const timeline::PlayerCacheInfo& cacheInfo() const;

        ///@}

        //! \name Audio
        ///@{

        //! Get the audio volume.
        float volume() const;

        //! Get the audio mute.
        bool isMuted() const;

        //! Get the audio sync offset (in seconds).
        double audioOffset() const;

        const std::vector<timeline::AudioData>& currentAudio() const;

        ///@}

        //! \name Playback
        ///@{

        //! Set the playback speed.
        void setSpeed(double);

        //! Set the playback mode.
        void setPlayback(tl::timeline::Playback);

        //! Stop playback.
        void stop();

        //! Forward playback.
        void forward();

        //! Reverse playback.
        void reverse();

        //! Toggle playback.
        void togglePlayback();

        //! Set the playback loop mode.
        void setLoop(tl::timeline::Loop);

        ///@}

        //! \name Time
        ///@{

        //! Seek to the given time.
        void seek(const otime::RationalTime&);

        //! Time action.
        void timeAction(tl::timeline::TimeAction);

        //! Go to the start time.
        void start();

        //! Go to the end time.
        void end();

        //! Go to the previous frame.
        void framePrev();

        //! Go to the next frame.
        void frameNext();

        ///@}

        //! \name In/Out Points
        ///@{

        //! Set the in/out points range.
        void setInOutRange(const otime::TimeRange&);

        //! Set the in point to the current time.
        void setInPoint();

        //! Reset the in point
        void resetInPoint();

        //! Set the out point to the current time.
        void setOutPoint();

        //! Reset the out point
        void resetOutPoint();

        ///@}

        //! \name Video
        ///@{

        //! Set the current video layer.
        void setVideoLayer(int);

        ///@}

        //! \name Audio
        ///@{

        //! Set the audio volume.
        void setVolume(float);

        //! Set the audio mute.
        void setMute(bool);

        //! Set the audio sync offset (in seconds).
        void setAudioOffset(double);

        ///@}

        //! \name Cache
        ///@{

        //! Set the cache options.
        void setCacheOptions(const tl::timeline::PlayerCacheOptions&);

        ///@}

        //! \name Playback
        ///@{

        //! This signal is emitted when the playback speed is changed.
        void speedChanged(double);

        //! This signal is emitted when the playback mode is changed.
        void playbackChanged(tl::timeline::Playback);

        //! This signal is emitted when the playback loop mode is changed.
        void loopChanged(tl::timeline::Loop);

        //! This signal is emitted when the current time is changed.
        void currentTimeChanged(const otime::RationalTime&);

        //! This signal is emitted when the in/out points range is changed.
        void inOutRangeChanged(const otime::TimeRange&);

        ///@}

        //! \name Video
        ///@{

        //! This signal is emitted when the current video layer is changed.
        void videoLayerChanged(size_t);

        //! This signal is emitted when the video is changed.
        void currentVideoChanged(const tl::timeline::VideoData&);

        //! This signal is emitted when the audio is changed.
        void
        currentAudioChanged(const std::vector<timeline::AudioData>& value){};

        ///@}

        //! \name Audio
        ///@{

        //! This signal is emitted when the audio volume is changed.
        void volumeChanged(float);

        //! This signal is emitted when the audio mute is changed.
        void muteChanged(bool);

        //! This signal is emitted when the audio sync offset is changed.
        void audioOffsetChanged(double);

        ///@}

        //! \name Cache
        ///@{

        //! This signal is emitted when the cache options have changed.
        void cacheOptionsChanged(const tl::timeline::PlayerCacheOptions&);

        //! This signal is emitted when the cache information has changed.
        void cacheInfoChanged(const tl::timeline::PlayerCacheInfo&);

        ///@}

        const otio::SerializableObject::Retainer<otio::Timeline>&
        getTimeline() const;

        //! Set the timeline.
        void
        setTimeline(const otio::SerializableObject::Retainer<otio::Timeline>&);

        //! \name Viewports
        ///@{

        void setTimelineViewport(TimelineViewport*);

        void setSecondaryViewport(TimelineViewport*);

        ///@}

        //! Return a list of annotation frames
        const std::vector< int64_t > getAnnotationFrames() const;

        //! Get annotation for current time
        std::shared_ptr< draw::Annotation > getAnnotation();

        //! Create annotation for current time
        std::shared_ptr< draw::Annotation >
        createAnnotation(const bool all_frames = false);

        //! Get list of annotations for between previous ghosting and
        //! next ghosting from current time
        std::vector< std::shared_ptr< draw::Annotation >>
        getAnnotations(const int, const int) const;

        //! Get all annotations in timeline player
        std::vector< std::shared_ptr< draw::Annotation >>
        getAllAnnotations() const;

        //! Set frame annotations in timeline player
        void setFrameAnnotation(const std::shared_ptr< draw::Annotation >&);

        //! Set all annotations in timeline player
        void setAllAnnotations(
            const std::vector< std::shared_ptr< draw::Annotation >>&);

        //! Clear all annotations in timeline player for current frame (time)
        void clearFrameAnnotation();

        //! Clear all annotations in timeline player
        void clearAllAnnotations();

        //! Undo the last annotation
        void undoAnnotation();

        //! Redo the last annotation
        void redoAnnotation();

    protected:
        template < typename T >
        void pushMessage(const std::string& command, T value);

        void timerEvent();

        static void timerEvent_cb(void* d);

    private:
        TimelineViewport* timelineViewport = nullptr;
        TimelineViewport* secondaryViewport = nullptr;

        TLRENDER_PRIVATE();
    };
} // namespace mrv
