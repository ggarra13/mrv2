
#include "mrViewer.h"

#include "mrvApp/mrvApp.h"

#include "mrvVoice/mrvVoiceOver.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvI8N.h"

#include <tlIO/System.h>

#include <tlCore/Audio.h>
#include <tlCore/AudioSystem.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl.H>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
}

#if defined(TLRENDER_AUDIO)
#    include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <thread>

namespace
{
    const int kNumChannels = 1;
    const unsigned kSampleRate = 44100;
    unsigned int rtBufferFrames = 256;
    const char* kModule = "voice";
}

namespace
{
    size_t voiceOverIndex = 1;
    
    std::string voiceOverFileName()
    {
        char buf[256];
        ViewerUI* ui = mrv::App::ui;
#ifdef _WIN32
        snprintf(buf, 256, "MRV2_VOICEOVER0x%p.%zu.wav", ui, voiceOverIndex);
#else
        snprintf(buf, 256, "MRV2_VOICEOVER%p.%zu.wav", ui, voiceOverIndex);
#endif
        PreferencesUI* uiPrefs = ui->uiPrefs;
        const std::string tmpdir = uiPrefs->uiPrefsVoiceOverPath->value();
        auto out = tmpdir + '/' + buf;

        ++voiceOverIndex;
        
        return out;
    }
}

        
struct AudioData
{
    size_t rtCurrentFrame = 0;
    std::vector<float> buffer;
};


#if defined(TLRENDER_AUDIO)
static int rtAudioPlaybackCallback(
    void* outputBuffer, void* inputBuffer, unsigned int nFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    // Unused output buffer
    (void)inputBuffer;
        
    // Cast the user data back to our struct
    AudioData* audio = static_cast<AudioData*>(userData);
    
    // Cast the input buffer to our sample type (float)
    float *out = static_cast<float*>(outputBuffer);

    size_t pos = audio->rtCurrentFrame * kNumChannels;
    if (pos > audio->buffer.size())
        return 1; // Stop streaming

    // Copy the audio data
    size_t dataByteCount = nFrames * kNumChannels * sizeof(float);
    float* stored = &audio->buffer[pos];
    memcpy(out, stored, dataByteCount);

    // Increment the counter based on playback speed set in preferences.
    PreferencesUI* uiPrefs = mrv::App::ui->uiPrefs;
    audio->rtCurrentFrame += nFrames *
                             (uiPrefs->uiPrefsVoiceOverSpeed->value() + 1);
    
    return 0; // Return 0 to continue streaming
}

static int rtAudioRecordCallback(
    void* outputBuffer, void* inputBuffer, unsigned int nFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    // Unused output buffer
    (void)outputBuffer;
    
    // Cast the user data back to our struct
    AudioData* audio = static_cast<AudioData*>(userData);
    
    // Cast the input buffer to our sample type (float)
    float* input = static_cast<float*>(inputBuffer);
    
    unsigned int numSamples = nFrames * kNumChannels;
    audio->buffer.insert(audio->buffer.end(), input, input + numSamples);

    return 0; // Return 0 to continue streaming
}

static void rtAudioErrorCallback(
    RtAudioError::Type type, const std::string& errorText)
{
    LOG_ERROR("rtAudio ERROR: " << type << " " << errorText);
}
#endif // TLRENDER_AUDIO

namespace mrv
{
    namespace voice
    {
        using namespace tl;

        struct MouseRecording
        {
            size_t idx = 0;
            std::vector<MouseData> data;
        };
        
        struct VoiceOver::Private
        {
            std::weak_ptr<system::Context> context;
                
#if defined(TLRENDER_AUDIO)
            std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
            
            bool running = false;
            std::thread thread;

            math::Vector2i center;
            
            AudioData audio;
            MouseRecording mouse;

            std::string  fileName;
            RecordStatus status = RecordStatus::Stopped;
        };



        TLRENDER_ENUM_IMPL(RecordStatus, "Stopped", "Recording", "Saved", "Playing");
        TLRENDER_ENUM_SERIALIZE_IMPL(RecordStatus);
        
        
        void VoiceOver::_init(const std::shared_ptr<system::Context>& context,
                              const math::Vector2i& center)
        {
            TLRENDER_P();
            p.context = context;
            p.center  = center;
            
            MouseData data;
            data.pos = center;
            data.pressed = false;

            p.mouse.data.push_back(data);
            p.mouse.idx = 0;
        }

        VoiceOver::VoiceOver() :
            _p(new Private)
        {
        }

        VoiceOver::~VoiceOver()
        {
            TLRENDER_P();
            
            if (p.rtAudio && p.rtAudio->isStreamRunning())
                p.rtAudio->abortStream();
                
            if (p.rtAudio && p.rtAudio->isStreamOpen())
            {
                p.rtAudio->closeStream();
            }
        }

        std::shared_ptr<VoiceOver>
        VoiceOver::create(const std::shared_ptr<system::Context>& context,
                          const math::Vector2i& center)
        {
            auto out = context->getSystem<VoiceOver>();
            if (!out)
            {
                out = std::shared_ptr<VoiceOver>(new VoiceOver);
                out->_init(context, center);
            }
            return out;
        }

        //! Get the context.
        const std::weak_ptr<system::Context>& VoiceOver::getContext() const
        {
            return _p->context;
        }

        std::vector<float> VoiceOver::getAudio() const
        {
            return _p->audio.buffer;
        }
        
        void VoiceOver::startRecording()
        {
            TLRENDER_P();
            
            if (p.status != RecordStatus::Stopped && p.audio.buffer.empty())
            {
                LOG_ERROR("Not stopped or audio buffer not empty.");
                return;
            }

            // Clear the recording
            p.audio.buffer.clear();
            p.audio.rtCurrentFrame = 0;

            appendRecording();            
        }
        
        void VoiceOver::appendRecording()
        {
            TLRENDER_P();
            
#if defined(TLRENDER_AUDIO)
            if (auto context = getContext().lock())
            {
                try
                {
                    auto audioSystem = context->getSystem<audio::System>();
                    auto api = static_cast<RtAudio::Api>(audioSystem->getCurrentAPI());
                    p.rtAudio.reset(new RtAudio(api));
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Cannot create RtAudio instance: " << e.what());
                }
            }
#endif
#if defined(TLRENDER_AUDIO)
            if (auto context = getContext().lock())
            {
                // Initialize audio.
                auto audioSystem = context->getSystem<audio::System>();
                if (p.rtAudio &&
                    !audioSystem->getDevices().empty())
                {
                    try
                    {
                        RtAudio::StreamParameters rtParameters;
                        auto audioSystem =
                            context->getSystem<audio::System>();
                        rtParameters.deviceId =
                            audioSystem->getInputDevice();
                        rtParameters.nChannels = kNumChannels;
                        p.rtAudio->openStream(
                            nullptr, &rtParameters,
                            RTAUDIO_FLOAT32,
                            kSampleRate,
                            &rtBufferFrames, rtAudioRecordCallback,
                            &p.audio, nullptr,
                            rtAudioErrorCallback);
                        p.rtAudio->startStream();
                        p.status = RecordStatus::Recording;
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("Cannot open audio stream for recording: " << e.what());
                    }
                }
            }
#endif // TLRENDER_AUDIO   
        }

        void VoiceOver::stopRecording()
        {
            TLRENDER_P();
            
            if (p.status != RecordStatus::Recording || p.audio.buffer.empty())
            {
                LOG_ERROR("No audio recorded.");
                return;
            }
            
            try
            {
                if (p.rtAudio && p.rtAudio->isStreamRunning())
                    p.rtAudio->stopStream();
            }
            catch (const RtAudioError& e)
            {
                e.printMessage();
            }

            p.status = RecordStatus::Saved;
            p.mouse.idx = 0;
                        
            if (auto context = p.context.lock())
            {
                auto ioSystem = context->getSystem<io::System>();

                const std::string fileName("/tmp/saved.wav");
            
                size_t totalSamples = p.audio.buffer.size() / sizeof(float);
                if (totalSamples == 0)
                {
                    return;
                }

                p.fileName = voiceOverFileName();
                AVFormatContext *fmt_ctx = NULL;
                AVStream *stream = NULL;

                // Allocate the format context for WAV output
                if (avformat_alloc_output_context2(&fmt_ctx, NULL, "wav",
                                                   fileName.c_str()) < 0 ||
                    !fmt_ctx)
                {
                    throw std::runtime_error("Could not create output context");
                    return;
                }

                // Create new audio stream
                stream = avformat_new_stream(fmt_ctx, NULL);
                if (!stream)
                {
                    avformat_free_context(fmt_ctx);
                    throw std::runtime_error("Could not free context");
                    return;
                }

                // Configure codec parameters for PCM 32-bit float LE
                AVCodecParameters *codecpar = stream->codecpar;
                codecpar->codec_type     = AVMEDIA_TYPE_AUDIO;
                codecpar->codec_id       = AV_CODEC_ID_PCM_F32LE;
                codecpar->sample_rate    = kSampleRate;
                av_channel_layout_default(&codecpar->ch_layout, kNumChannels);
                codecpar->bit_rate       = codecpar->sample_rate * kNumChannels * sizeof(float) * 8;

                // Open the output file
                if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
                    if (avio_open(&fmt_ctx->pb, p.fileName.c_str(), AVIO_FLAG_WRITE) < 0) {
                        avformat_free_context(fmt_ctx);
                        throw std::runtime_error("Could not open output file");
                        return;
                    }
                }

                // Write the WAV header
                if (avformat_write_header(fmt_ctx, NULL) < 0) {
                    avio_closep(&fmt_ctx->pb);
                    avformat_free_context(fmt_ctx);
                    throw std::runtime_error("Error occurred when writing header");
                    return;
                }

                // Allocate packet
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    avio_closep(&fmt_ctx->pb);
                    avformat_free_context(fmt_ctx);
                    return;
                }

                pkt->data = (uint8_t*)&p.audio.buffer[0];
                pkt->size = p.audio.buffer.size();
                pkt->stream_index = stream->index;
                pkt->duration = totalSamples;
                pkt->pts = pkt->dts = 0;

                // Write raw samples
                if (av_interleaved_write_frame(fmt_ctx, pkt) < 0) {
                    throw std::runtime_error("Error writing frame");
                }

                // Write trailer (finalize WAV header)
                av_write_trailer(fmt_ctx);

                // Cleanup
                av_packet_free(&pkt);
                if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE))
                    avio_closep(&fmt_ctx->pb);
                avformat_free_context(fmt_ctx);
            }
        }

        void VoiceOver::startPlaying()
        {
            TLRENDER_P();

            if (p.status != RecordStatus::Saved || p.audio.buffer.empty())
            {
                LOG_ERROR("No audio for playing.");
                return;
            }
            
#if defined(TLRENDER_AUDIO)
            if (auto context = getContext().lock())
            {
                try
                {
                    auto audioSystem = context->getSystem<audio::System>();
                    auto api = static_cast<RtAudio::Api>(audioSystem->getCurrentAPI());
                    p.rtAudio.reset(new RtAudio(api));
                }
                catch (const std::exception& e)
                {
                    std::stringstream ss;
                    ss << "Cannot create RtAudio instance: " << e.what();
                    context->log(
                        "mrv::voice::VoiceOver", ss.str(), log::Type::Error);
                }
            }
#endif
            if (auto context = getContext().lock())
            {
                // Initialize audio.
                auto audioSystem = context->getSystem<audio::System>();
                if (p.rtAudio &&
                    !audioSystem->getDevices().empty())
                {
                    try
                    {
                        RtAudio::StreamParameters rtParameters;
                        auto audioSystem = context->getSystem<audio::System>();
                        rtParameters.deviceId = audioSystem->getOutputDevice();
                        rtParameters.nChannels = kNumChannels;
                        p.rtAudio->openStream(
                            &rtParameters, nullptr,
                            RTAUDIO_FLOAT32,
                            kSampleRate,
                            &rtBufferFrames, rtAudioPlaybackCallback,
                            &p.audio, nullptr,
                            rtAudioErrorCallback);
                        p.rtAudio->startStream();
                        p.status = RecordStatus::Playing;
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("Cannot open audio stream for playback: " << e.what());
                    }
                }
            }
        }
        
        void VoiceOver::stopPlaying()
        {
            TLRENDER_P();
            
            if (p.status != RecordStatus::Playing || p.audio.buffer.empty())
            {
                LOG_ERROR("No audio playing.");
                return;
            }
            
            try
            {
                if (p.rtAudio && p.rtAudio->isStreamRunning())
                    p.rtAudio->stopStream();
            }
            catch (const RtAudioError& e)
            {
                e.printMessage();
            }
            
            p.status = RecordStatus::Saved;
            p.audio.rtCurrentFrame = 0;
            p.mouse.idx = 0;
        }

        RecordStatus VoiceOver::getStatus() const
        {
            return _p->status;
        }
        
        MouseData VoiceOver::getMouseData() const
        {
            TLRENDER_P();

            size_t idx = p.mouse.idx;
            if (idx >= p.mouse.data.size())
                idx = p.mouse.data.size() - 1;

            return p.mouse.data[idx];
        }

        void VoiceOver::appendMouseData(const MouseData& mouse)
        {
            _p->mouse.data.push_back(mouse);
            _p->mouse.idx = _p->mouse.data.size() - 1;
        }

        const math::Vector2i& VoiceOver::getCenter() const
        {
            return _p->center;
        }
        
        const math::Box2i VoiceOver::getBBox() const
        {
            return math::Box2i(_p->center.x - 10, _p->center.y - 10, 20, 20);
        }

        void VoiceOver::tick()
        {
            TLRENDER_P();
            
            p.mouse.idx++;

            if (p.mouse.idx >= p.mouse.data.size())
            {
                p.status = RecordStatus::Saved;
                p.audio.rtCurrentFrame = 0;
                p.mouse.idx = 0;
            }
        }
        
    }
}
