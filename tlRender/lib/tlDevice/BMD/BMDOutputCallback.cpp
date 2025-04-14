// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputPrivate.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/AudioResample.h>

#if defined(_WINDOWS)
#    include <atlbase.h>
#endif // _WINDOWS

namespace tl
{
    namespace bmd
    {
        namespace
        {
            const size_t videoPreroll = 3;
            const size_t videoFramesMax = 3;
            //! \todo Should this be the same as
            //! timeline::PlayerOptions().audioBufferFrameCount?
            const size_t audioBufferCount = 3000;
        } // namespace

        struct DLOutputCallback::Private
        {
            IDeckLinkOutput* dlOutput = nullptr;
            math::Size2i size;
            PixelType pixelType = PixelType::kNone;
            otime::RationalTime frameRate = time::invalidTime;
            audio::Info audioInfo;

            std::atomic<size_t> refCount;

            struct VideoMutex
            {
                std::list<std::shared_ptr<DLVideoFrameWrapper> > videoFrames;
                std::mutex mutex;
            };
            VideoMutex videoMutex;

            struct VideoThread
            {
                std::shared_ptr<DLVideoFrameWrapper> videoFrame;
#if defined(_WINDOWS)
                CComPtr<IDeckLinkVideoConversion> frameConverter;
#else  // _WINDOWS
                DLFrameConversionWrapper frameConverter;
#endif // _WINDOWS
                uint64_t frameCount = 0;
                std::chrono::steady_clock::time_point t;
            };
            VideoThread videoThread;

            struct AudioMutex
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                otime::RationalTime currentTime = time::invalidTime;
                float volume = 1.F;
                bool mute = false;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct AudioThread
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                size_t samplesOffset = 0;
                std::shared_ptr<audio::AudioResample> resample;
            };
            AudioThread audioThread;
        };

        DLOutputCallback::DLOutputCallback(
            IDeckLinkOutput* dlOutput, const math::Size2i& size,
            PixelType pixelType, const otime::RationalTime& frameRate,
            const audio::Info& audioInfo) :
            _p(new Private)
        {
            TLRENDER_P();

            p.dlOutput = dlOutput;
            p.size = size;
            p.pixelType = pixelType;
            p.frameRate = frameRate;
            p.audioInfo = audioInfo;
            p.refCount = 1;

#if defined(_WINDOWS)
            HRESULT r = p.videoThread.frameConverter.CoCreateInstance(
                CLSID_CDeckLinkVideoConversion, nullptr, CLSCTX_ALL);
            if (r != S_OK)
            {
                throw std::runtime_error("Cannot create video frame converter");
            }
#else  // _WINDOWS
            p.videoThread.frameConverter.p = CreateVideoConversionInstance();
            if (!p.videoThread.frameConverter.p)
            {
                throw std::runtime_error("Cannot create video frame converter");
            }
#endif // _WINDOWS

            IDeckLinkProfileAttributes* dlProfileAttributes = nullptr;
            if (dlOutput->QueryInterface(
                    IID_IDeckLinkProfileAttributes,
                    (void**)&dlProfileAttributes) == S_OK)
            {
                LONGLONG minVideoPreroll = 0;
                if (dlProfileAttributes->GetInt(
                        BMDDeckLinkMinimumPrerollFrames, &minVideoPreroll) ==
                    S_OK)
                {
                    //! \bug Leave the default preroll, lower numbers
                    //! cause stuttering.
                    // videoPreroll = minVideoPreroll;
                }
            }

            p.dlOutput->BeginAudioPreroll();
            /*const size_t audioPrerollSamples = videoPreroll / 24.0 *
            audioInfo.sampleRate; std::vector<uint8_t> emptyAudio(
                audioPrerollSamples *
                audioInfo.channelCount *
                audio::getByteCount(audioInfo.dataType), 0);
            uint32_t audioSamplesWritten = 0;
            p.dlOutput->ScheduleAudioSamples(
                emptyAudio.data(),
                audioPrerollSamples,
                0,
                0,
                nullptr);*/
            p.dlOutput->EndAudioPreroll();

            for (size_t i = 0; i < videoPreroll; ++i)
            {
                DLVideoFrameWrapper dlVideoFrame;
                if (p.dlOutput->CreateVideoFrame(
                        p.size.w, p.size.h,
                        getRowByteCount(p.size.w, p.pixelType),
                        toBMD(p.pixelType), bmdFrameFlagDefault,
                        &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (p.dlOutput->ScheduleVideoFrame(
                        dlVideoFrame.p,
                        p.videoThread.frameCount * p.frameRate.value(),
                        p.frameRate.value(), p.frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                p.videoThread.frameCount = p.videoThread.frameCount + 1;
            }

            p.videoThread.t = std::chrono::steady_clock::now();

            p.dlOutput->StartScheduledPlayback(0, p.frameRate.rate(), 1.0);
        }

        void DLOutputCallback::setPlayback(
            timeline::Playback value, const otime::RationalTime& time)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            if (value != p.audioMutex.playback)
            {
                p.dlOutput->FlushBufferedAudioSamples();
                p.audioMutex.playback = value;
                p.audioMutex.startTime = time;
                p.audioMutex.currentTime = time;
            }
        }

        void DLOutputCallback::setVideo(
            const std::shared_ptr<DLVideoFrameWrapper>& value,
            const otime::RationalTime& time)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                p.videoMutex.videoFrames.push_back(value);
                while (p.videoMutex.videoFrames.size() > videoFramesMax)
                {
                    p.videoMutex.videoFrames.pop_front();
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (time != p.audioMutex.currentTime)
                {
                    const otime::RationalTime currentTimePlusOne(
                        p.audioMutex.currentTime.value() + 1.0,
                        p.audioMutex.currentTime.rate());
                    if (time != currentTimePlusOne)
                    {
                        p.audioMutex.startTime = time;
                    }
                    p.audioMutex.currentTime = time;
                }
            }
        }

        void DLOutputCallback::setVolume(float value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.volume = value;
        }

        void DLOutputCallback::setMute(bool value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.mute = value;
        }

        void DLOutputCallback::setAudioOffset(double value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.audioOffset = value;
        }

        void DLOutputCallback::setAudioData(
            const std::vector<timeline::AudioData>& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.audioData = value;
        }

        HRESULT DLOutputCallback::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        ULONG DLOutputCallback::AddRef()
        {
            return ++_p->refCount;
        }

        ULONG DLOutputCallback::Release()
        {
            const ULONG out = --_p->refCount;
            if (0 == out)
            {
                delete this;
                return 0;
            }
            return out;
        }

        HRESULT DLOutputCallback::ScheduledFrameCompleted(
            IDeckLinkVideoFrame* dlVideoFrame,
            BMDOutputFrameCompletionResult dlResult)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.videoFrames.empty())
                {
                    p.videoThread.videoFrame = p.videoMutex.videoFrames.front();
                    p.videoMutex.videoFrames.pop_front();
                }
            }

            if (p.videoThread.videoFrame)
            {
                if (p.videoThread.videoFrame->p->GetPixelFormat() ==
                    toBMD(p.pixelType))
                {
                    dlVideoFrame = p.videoThread.videoFrame->p;
                }
                else
                {
                    p.videoThread.frameConverter->ConvertFrame(
                        p.videoThread.videoFrame->p, dlVideoFrame);
                }
            }

            p.dlOutput->ScheduleVideoFrame(
                dlVideoFrame, p.videoThread.frameCount * p.frameRate.value(),
                p.frameRate.value(), p.frameRate.rate());
            // std::cout << "result: " <<
            // getOutputFrameCompletionResultLabel(dlResult) << std::endl;
            p.videoThread.frameCount += 1;

            const auto t = std::chrono::steady_clock::now();
            const std::chrono::duration<double> diff = t - p.videoThread.t;
            // std::cout << "diff: " << diff.count() * 1000 << std::endl;
            p.videoThread.t = t;

            return S_OK;
        }

        HRESULT DLOutputCallback::ScheduledPlaybackHasStopped()
        {
            return S_OK;
        }

        HRESULT DLOutputCallback::RenderAudioSamples(BOOL preroll)
        {
            TLRENDER_P();

            // Get values.
            otime::RationalTime currentTime = time::invalidTime;
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioDataList;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (p.audioMutex.playback != p.audioThread.playback ||
                    p.audioMutex.startTime != p.audioThread.startTime)
                {
                    p.audioThread.playback = p.audioMutex.playback;
                    p.audioThread.startTime = p.audioMutex.startTime;
                    p.audioThread.samplesOffset = 0;
                }
                currentTime = p.audioMutex.currentTime;
                volume = p.audioMutex.volume;
                mute = p.audioMutex.mute;
                audioOffset = p.audioMutex.audioOffset;
                audioDataList = p.audioMutex.audioData;
            }
            // std::cout << "audio playback: " << p.audioThread.playback <<
            // std::endl; std::cout << "audio start time: " <<
            // p.audioThread.startTime << std::endl; std::cout << "audio samples
            // offset: " << p.audioThread.samplesOffset << std::endl;

            // Flush the audio resampler and BMD buffer when the playback
            // is reset.
            if (0 == p.audioThread.samplesOffset)
            {
                if (p.audioThread.resample)
                {
                    p.audioThread.resample->flush();
                }
                p.dlOutput->FlushBufferedAudioSamples();
            }

            // Create the audio resampler.
            audio::Info inputInfo;
            if (!audioDataList.empty() && !audioDataList[0].layers.empty() &&
                audioDataList[0].layers[0].audio)
            {
                inputInfo = audioDataList[0].layers[0].audio->getInfo();
                if (!p.audioThread.resample ||
                    (p.audioThread.resample &&
                     p.audioThread.resample->getInputInfo() != inputInfo))
                {
                    p.audioThread.resample =
                        audio::AudioResample::create(inputInfo, p.audioInfo);
                }
            }

            // Copy audio data to BMD.
            if (timeline::Playback::Forward == p.audioThread.playback &&
                p.audioThread.resample)
            {
                int64_t frame =
                    p.audioThread.startTime.rescaled_to(inputInfo.sampleRate)
                        .value() -
                    otime::RationalTime(audioOffset, 1.0)
                        .rescaled_to(inputInfo.sampleRate)
                        .value() +
                    p.audioThread.samplesOffset;
                int64_t seconds = inputInfo.sampleRate > 0
                                      ? (frame / inputInfo.sampleRate)
                                      : 0;
                int64_t offset = frame - seconds * inputInfo.sampleRate;

                uint32_t bufferedSampleCount = 0;
                p.dlOutput->GetBufferedAudioSampleFrameCount(
                    &bufferedSampleCount);
                // std::cout << "bmd buffered sample count: " <<
                // bufferedSampleCount << std::endl;
                while (bufferedSampleCount < audioBufferCount)
                {
                    // std::cout << "frame: " << frame << std::endl;
                    // std::cout << "seconds: " << seconds << std::endl;
                    // std::cout << "offset: " << offset << std::endl;
                    timeline::AudioData audioData;
                    for (const auto& i : audioDataList)
                    {
                        if (seconds == static_cast<int64_t>(i.seconds))
                        {
                            audioData = i;
                            break;
                        }
                    }
                    if (audioData.layers.empty())
                    {
                        {
                            std::unique_lock<std::mutex> lock(
                                p.audioMutex.mutex);
                            p.audioMutex.startTime = currentTime;
                        }
                        p.audioThread.startTime = currentTime;
                        p.audioThread.samplesOffset = 0;
                        break;
                    }
                    std::vector<const uint8_t*> audioDataP;
                    for (const auto& layer : audioData.layers)
                    {
                        if (layer.audio && layer.audio->getInfo() == inputInfo)
                        {
                            audioDataP.push_back(
                                layer.audio->getData() +
                                offset * inputInfo.getByteCount());
                        }
                    }

                    const size_t size = std::min(
                        audioBufferCount,
                        inputInfo.sampleRate - static_cast<size_t>(offset));
                    // std::cout << "size: " << size << " " << std::endl;
                    auto tmpAudio = audio::Audio::create(inputInfo, size);
                    audio::mix(
                        audioDataP.data(), audioDataP.size(),
                        tmpAudio->getData(), mute ? 0.F : volume, size,
                        inputInfo.channelCount, inputInfo.dataType);

                    auto resampledAudio =
                        p.audioThread.resample->process(tmpAudio);
                    p.dlOutput->ScheduleAudioSamples(
                        resampledAudio->getData(),
                        resampledAudio->getSampleCount(), 0, 0, nullptr);

                    offset += size;
                    if (offset >= inputInfo.sampleRate)
                    {
                        offset -= inputInfo.sampleRate;
                        seconds += 1;
                    }

                    p.audioThread.samplesOffset += size;

                    HRESULT result =
                        p.dlOutput->GetBufferedAudioSampleFrameCount(
                            &bufferedSampleCount);
                    if (result != S_OK)
                    {
                        break;
                    }

                    // std::cout << std::endl;
                }
            }

            // BMDTimeScale dlTimeScale = audioSampleRate;
            // BMDTimeValue dlTimeValue = 0;
            // if (p.dlOutput->GetScheduledStreamTime(dlTimeScale, &dlTimeValue,
            // nullptr) == S_OK)
            //{
            //     std::cout << "stream time: " << dlTimeValue << std::endl;
            // }

            return S_OK;
        }
    } // namespace bmd
} // namespace tl
