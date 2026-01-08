// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/PlayerOptions.h>
#include <tlTimeline/Timeline.h>

#include <tlCore/ListObserver.h>

namespace tl
{
    namespace timeline
    {
        //! Timeline player cache information.
        struct PlayerCacheInfo
        {
            //! Video cache percentage used.
            float videoPercentage = 0.F;

            //! Cached video frames.
            std::vector<otime::TimeRange> videoFrames;

            //! Cached audio frames.
            std::vector<otime::TimeRange> audioFrames;

            bool operator==(const PlayerCacheInfo&) const;
            bool operator!=(const PlayerCacheInfo&) const;
        };

        //! Playback loop modes.
        enum class Loop {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        TLRENDER_ENUM(Loop);
        TLRENDER_ENUM_SERIALIZE(Loop);

        //! Time actions.
        enum class TimeAction {
            Start,
            End,
            FramePrev,
            FramePrevX10,
            FramePrevX100,
            FrameNext,
            FrameNextX10,
            FrameNextX100,
            JumpBack1s,
            JumpBack10s,
            JumpForward1s,
            JumpForward10s,

            Count,
            First = Start
        };
        TLRENDER_ENUM(TimeAction);
        TLRENDER_ENUM_SERIALIZE(TimeAction);

        //! Timeline player.
        class Player : public std::enable_shared_from_this<Player>
        {
            TLRENDER_NON_COPYABLE(Player);

        protected:
            void _init(
                const std::shared_ptr<Timeline>&,
                const std::shared_ptr<system::Context>&, const PlayerOptions&);

            Player();

            void updateVideoData();

        public:
            ~Player();

            //! Create a new timeline player.
            static std::shared_ptr<Player> create(
                const std::shared_ptr<Timeline>&,
                const std::shared_ptr<system::Context>&,
                const PlayerOptions& = PlayerOptions());

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;

            //! Get the timeline.
            const std::shared_ptr<Timeline>& getTimeline() const;

            //! Set the OpenTimelineIO timeline, resetting in/out points.
            void setTimeline(
                const otio::SerializableObject::Retainer<otio::Timeline>&);
            
            //! Get the path.
            const file::Path& getPath() const;

            //! Get the audio path.
            const file::Path& getAudioPath() const;

            //! Get the timeline player options.
            const PlayerOptions& getPlayerOptions() const;

            //! Get the timeline options.
            const Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the time range.
            const otime::TimeRange& getTimeRange() const;

            //! Get the I/O information. This information is retrieved from
            //! the first clip in the timeline.
            const io::Info& getIOInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            double getDefaultSpeed() const;

            //! Get the playback speed.
            double getSpeed() const;

            //! Observe the playback speed.
            std::shared_ptr<observer::IValue<double> > observeSpeed() const;

            //! Set the playback speed.
            void setSpeed(double);

            //! Get the playback mode.
            Playback getPlayback() const;

            //! Observe the playback mode.
            std::shared_ptr<observer::IValue<Playback> >
            observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Get the playback loop.
            Loop getLoop() const;

            //! Observe the playback loop mode.
            std::shared_ptr<observer::IValue<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            ///@}

            //! \name Time
            ///@{

            //! Get the current time.
            otime::RationalTime getCurrentTime() const;

            //! Observe the current time.
            std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeCurrentTime() const;

            //! Seek to the given time.
            void seek(const otime::RationalTime&);

            //! Time action.
            void timeAction(TimeAction);

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

            //! Get the in/out points range.
            otime::TimeRange getInOutRange() const;

            //! Observe the in/out points range.
            std::shared_ptr<observer::IValue<otime::TimeRange> >
            observeInOutRange() const;

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

            //! \name Comparison
            ///@{

            //! Get the timelines for comparison.
            const std::vector<std::shared_ptr<Timeline> >& getCompare() const;

            //! Observe the timelines for comparison.
            std::shared_ptr<observer::IList<std::shared_ptr<Timeline> > >
            observeCompare() const;

            //! Set the timelines for comparison.
            void setCompare(const std::vector<std::shared_ptr<Timeline> >&);

            //! Get the comparison time mode.
            CompareTimeMode getCompareTime() const;

            //! Observe the comparison time mode.
            std::shared_ptr<observer::IValue<CompareTimeMode> >
            observeCompareTime() const;

            //! Set the comparison time mode.
            void setCompareTime(CompareTimeMode);

            ///@}

            //! \name I/O
            ///@{

            //! Get the I/O options.
            const io::Options& getIOOptions() const;

            //! Observe the I/O options.
            std::shared_ptr<observer::IValue<io::Options> >
            observeIOOptions() const;

            //! Set the I/O options.
            void setIOOptions(const io::Options&);

            ///@}

            //! \name Video
            ///@{

            //! Get the video layer.
            int getVideoLayer() const;

            //! Observer the video layer.
            std::shared_ptr<observer::IValue<int> > observeVideoLayer() const;

            //! Set the video layer.
            void setVideoLayer(int);

            //! Get the comparison video layers.
            const std::vector<int>& getCompareVideoLayers() const;

            //! Observe the comparison video layers.
            std::shared_ptr<observer::IList<int> >
            observeCompareVideoLayers() const;

            //! Set the comparison video layers.
            void setCompareVideoLayers(const std::vector<int>&);

            //! Get the current video data.
            const std::vector<VideoData>& getCurrentVideo() const;

            //! Observe the current video data.
            std::shared_ptr<observer::IList<VideoData> >
            observeCurrentVideo() const;

            ///@}

            //! \name Audio
            ///@{

            //! Get the volume.
            float getVolume() const;

            //! Observe the audio volume.
            std::shared_ptr<observer::IValue<float> > observeVolume() const;

            //! Set the audio volume.
            void setVolume(float);

            //! Get the audio mute.
            bool isMuted() const;

            //! Observe the audio mute.
            std::shared_ptr<observer::IValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Get the audio channels mute.
            const std::vector<int>& getChannelMute() const;

            //! Observe the audio channels mute.
            std::shared_ptr<observer::IList<int> > observeChannelMute() const;

            //! Set the audio channels mute.
            void setChannelMute(const std::vector<int>&);

            //! Get the audio sync offset (in seconds).
            double getAudioOffset() const;

            //! Observe the audio sync offset (in seconds).
            std::shared_ptr<observer::IValue<double> >
            observeAudioOffset() const;

            //! Set the audio sync offset (in seconds).
            void setAudioOffset(double);

            //! Get the current audio data.
            const std::vector<AudioData>& getCurrentAudio() const;

            //! Observe the current audio data.
            std::shared_ptr<observer::IList<AudioData> >
            observeCurrentAudio() const;

            ///@}

            //! \name Cache
            ///@{

            //! Get the cache options.
            const PlayerCacheOptions& getCacheOptions() const;

            //! Observe the cache options.
            std::shared_ptr<observer::IValue<PlayerCacheOptions> >
            observeCacheOptions() const;

            //! Set the cache options.
            void setCacheOptions(const PlayerCacheOptions&);

            //! Observe the cache information.
            std::shared_ptr<observer::IValue<PlayerCacheInfo> >
            observeCacheInfo() const;

            //! Update Video Cache Time.
            void updateVideoCache(const otime::RationalTime& time);

            //! Clear the cache.
            void clearCache();

            ///@}

            //! Tick the timeline player.
            void tick();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace timeline
} // namespace tl

#include <tlTimeline/PlayerInline.h>
