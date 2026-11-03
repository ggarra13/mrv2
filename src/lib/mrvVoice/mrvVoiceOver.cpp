
#include "mrViewer.h"

#include "mrvApp/mrvApp.h"

#include "mrvVoice/mrvVoiceOver.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvOS/mrvI8N.h"

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
#    include <miniaudio/miniaudio.h>
#endif // TLRENDER_AUDIO

#include <thread>
#include <atomic>
#include <algorithm>

namespace
{
    const int      kNumChannels = 2; // we store and play stereo
    const unsigned kSampleRate = 44100;
    const char*    kModule = "voice";
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
        const std::string tmpdir = uiPrefs->VoiceOverPath->value();
        auto out = tmpdir + '/' + buf;

        ++voiceOverIndex;

        return out;
    }

    // A helper function to print error messages from FFmpeg functions
    void print_ffmpeg_error(int error_code, const std::string& message)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(error_code, errbuf, sizeof(errbuf));
        LOG_ERROR(message << ": " << errbuf);
    }
}


struct AudioData
{
    size_t rtCurrentFrame = 0;
    std::vector<float> buffer;
    std::atomic<bool> playbackFinished{false};
};


#if defined(TLRENDER_AUDIO)

// miniaudio callbacks must not perform blocking operations. The recording
// callback appends samples to the buffer, matching the original RtAudio
// implementation. The playback callback signals completion through an atomic
// flag; the device is stopped later from the normal application thread.
static void maAudioPlaybackCallback(
    ma_device* device, void* outputBuffer, const void* inputBuffer,
    ma_uint32 frameCount)
{
    (void)inputBuffer;

    AudioData* audio = static_cast<AudioData*>(device->pUserData);
    float* out = static_cast<float*>(outputBuffer);
    const size_t requested = static_cast<size_t>(frameCount) * kNumChannels;

    std::memset(out, 0, requested * sizeof(float));

    const size_t pos = audio->rtCurrentFrame * kNumChannels;
    if (pos >= audio->buffer.size())
    {
        audio->playbackFinished.store(true, std::memory_order_release);
        return;
    }

    const size_t availableFrames =
        (audio->buffer.size() - pos) / kNumChannels;
    const size_t framesToCopy =
        std::min<size_t>(availableFrames, frameCount);
    const size_t samplesToCopy = framesToCopy * kNumChannels;

    if (samplesToCopy > 0)
    {
        std::memcpy(out, audio->buffer.data() + pos,
                    samplesToCopy * sizeof(float));
    }

    // Preserve the original voice-over speed behavior: advance through the
    // source by the preference multiplier after each output callback.
    int speed = 1;
    if (mrv::App::ui && mrv::App::ui->uiPrefs)
        speed = mrv::App::ui->uiPrefs->VoiceOverSpeed->value() + 1;
    audio->rtCurrentFrame += static_cast<size_t>(frameCount) *
                             static_cast<size_t>(std::max(speed, 1));

    if (framesToCopy < frameCount ||
        audio->rtCurrentFrame * kNumChannels >= audio->buffer.size())
    {
        audio->playbackFinished.store(true, std::memory_order_release);
    }
}

static void maAudioRecordCallback(
    ma_device* device, void* outputBuffer, const void* inputBuffer,
    ma_uint32 frameCount)
{
    (void)outputBuffer;

    AudioData* audio = static_cast<AudioData*>(device->pUserData);
    const size_t numSamples = static_cast<size_t>(frameCount) * kNumChannels;

    if (!inputBuffer)
    {
        audio->buffer.insert(audio->buffer.end(), numSamples, 0.0f);
        return;
    }

    const float* input = static_cast<const float*>(inputBuffer);
    audio->buffer.reserve(audio->buffer.size() + numSamples);

    // Capture is mono; store it as interleaved stereo.
    for (ma_uint32 i = 0; i < frameCount; ++i)
    {
        audio->buffer.push_back(input[i]);
        audio->buffer.push_back(input[i]);
    }
}

#endif // TLRENDER_AUDIO

namespace mrv
{
    namespace voice
    {
        using namespace tl;

        struct VoiceOver::Private
        {
            std::weak_ptr<system::Context> context;

#if defined(TLRENDER_AUDIO)
            ma_device audioDevice{};
            bool audioDeviceInitialized = false;
#endif // TLRENDER_AUDIO

            bool running = false;
            std::thread thread;

            AudioData audio;
        };



        TLRENDER_ENUM_IMPL(RecordStatus, "Stopped", "Recording", "Saved", "Playing");
        TLRENDER_ENUM_SERIALIZE_IMPL(RecordStatus);


        void VoiceOver::_init(const std::shared_ptr<system::Context>& context,
                              const math::Vector2f& inCenter)
        {
            TLRENDER_P();
            p.context = context;
            center  = inCenter;

            MouseData data;
            data.pos = inCenter;
            data.pressed = false;

            mouse.data.push_back(data);
            mouse.idx = 0;
        }

        VoiceOver::VoiceOver() :
            _p(new Private)
        {
        }

        VoiceOver::~VoiceOver()
        {
            _cleanupAudio();
        }

        std::shared_ptr<VoiceOver>
        VoiceOver::create(const std::shared_ptr<system::Context>& context,
                          const math::Vector2f& center)
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

        void VoiceOver::clear()
        {
            TLRENDER_P();

            p.audio.buffer.clear();
            mouse.data.clear();

            MouseData data;
            data.pos = center;
            data.pressed = false;
            mouse.data.push_back(data);

            status = RecordStatus::Stopped;
            p.audio.rtCurrentFrame = 0;
            mouse.idx = 0;
        }

        std::vector<float> VoiceOver::getAudio() const
        {
            return _p->audio.buffer;
        }

        void VoiceOver::startRecording()
        {
            TLRENDER_P();

            // Clear the recording
            p.audio.buffer.clear();
            p.audio.rtCurrentFrame = 0;

            _startRecording();
        }

        void VoiceOver::appendRecording()
        {
            TLRENDER_P();

            _startRecording();
        }

        void VoiceOver::_startRecording()
        {
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            _cleanupAudio();

            ma_device_config config =
                ma_device_config_init(ma_device_type_capture);
            config.capture.format = ma_format_f32;
            config.capture.channels = 1;
            config.sampleRate = kSampleRate;
            config.dataCallback = maAudioRecordCallback;
            config.pUserData = &p.audio;

            // Use miniaudio's default capture device. RtAudio device IDs
            // are backend-specific and cannot be reused as miniaudio IDs.
            ma_result result = ma_device_init(nullptr, &config,
                                              &p.audioDevice);
            if (result == MA_SUCCESS)
            {
                p.audioDeviceInitialized = true;
                result = ma_device_start(&p.audioDevice);
            }

            if (result == MA_SUCCESS)
            {
                status = RecordStatus::Recording;
            }
            else
            {
                LOG_ERROR("Cannot initialize or start miniaudio capture device: "
                          << ma_result_description(result));
                _cleanupAudio();
            }
#endif // TLRENDER_AUDIO
        }

        std::string VoiceOver::getFileName() const
        {
            return fileName;
        }

        void VoiceOver::loadAudio()
        {
            TLRENDER_P();

            AVFormatContext* fmt_ctx = NULL;
            AVCodecContext* dec_ctx = NULL;
            int audio_stream_index = -1;
            AVPacket* pkt = NULL;
            AVFrame* frame = NULL;

            try {
                // Step 1: Open the input file and allocate the format context
                if (avformat_open_input(&fmt_ctx, fileName.c_str(), NULL, NULL) < 0)
                {
                    throw std::runtime_error("Could not open input file");
                }

                // Step 2: Find stream information
                if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
                {
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Could not find stream information");
                }

                // Step 3: Find the audio stream
                audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
                if (audio_stream_index < 0)
                {
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Could not find any audio stream in the input file");
                }

                AVStream* audio_stream = fmt_ctx->streams[audio_stream_index];
                const AVCodec* decoder = avcodec_find_decoder(audio_stream->codecpar->codec_id);
                if (!decoder)
                {
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Failed to find decoder for the audio stream");
                }

                // Step 4: Allocate a new decoding context
                dec_ctx = avcodec_alloc_context3(decoder);
                if (!dec_ctx)
                {
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Failed to allocate a codec context");
                }

                // Copy codec parameters from input stream to decoder context
                if (avcodec_parameters_to_context(dec_ctx,
                                                  audio_stream->codecpar) < 0)
                {
                    avcodec_free_context(&dec_ctx);
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Failed to copy codec parameters to decoder context");
                }

                // Step 5: Open the decoder
                if (avcodec_open2(dec_ctx, decoder, NULL) < 0)
                {
                    avcodec_free_context(&dec_ctx);
                    avformat_close_input(&fmt_ctx);
                    throw std::runtime_error("Failed to open decoder");
                }

                // Allocate a packet and frame for reading and decoding
                pkt = av_packet_alloc();
                frame = av_frame_alloc();
                if (!pkt || !frame)
                {
                    throw std::runtime_error("Could not allocate packet or frame");
                }

                // Step 6: Read frames from the file
                p.audio.buffer.clear(); // Clear any existing buffer data

                int ret = 0;
                while (av_read_frame(fmt_ctx, pkt) >= 0) {
                    // Check if the packet belongs to our audio stream
                    if (pkt->stream_index == audio_stream_index)
                    {
                        // Send the packet to the decoder
                        ret = avcodec_send_packet(dec_ctx, pkt);
                        if (ret < 0)
                        {
                            print_ffmpeg_error(ret, "Error sending packet to the decoder");
                            av_packet_unref(pkt);
                            break;
                        }

                        // Receive decoded frames from the decoder
                        while (ret >= 0)
                        {
                            ret = avcodec_receive_frame(dec_ctx, frame);
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            {
                                break;
                            }
                            else if (ret < 0)
                            {
                                print_ffmpeg_error(ret, "Error receiving frame from the decoder");
                                break;
                            }

                            // Append decoded data to the buffer
                            const size_t num_samples = frame->nb_samples * kNumChannels;
                            size_t current_size = p.audio.buffer.size();
                            p.audio.buffer.resize(current_size + num_samples);
                            memcpy(p.audio.buffer.data() + current_size, frame->data[0], num_samples * sizeof(float));

                            av_frame_unref(frame);
                        }
                    }
                    av_packet_unref(pkt);
                }

                // Flush the decoder
                ret = avcodec_send_packet(dec_ctx, NULL); // Flush decoder
                while (ret >= 0) {
                    ret = avcodec_receive_frame(dec_ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        print_ffmpeg_error(ret, "Error receiving frame from the decoder on flush");
                        break;
                    }

                    // Append remaining decoded data to the buffer
                    const size_t num_samples = frame->nb_samples * kNumChannels;
                    size_t current_size = p.audio.buffer.size();
                    p.audio.buffer.resize(current_size + num_samples);
                    memcpy(p.audio.buffer.data() + current_size, frame->data[0], num_samples * sizeof(float));

                    av_frame_unref(frame);
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            // Step 7: Cleanup
            av_packet_free(&pkt);
            av_frame_free(&frame);
            avcodec_free_context(&dec_ctx);
            avformat_close_input(&fmt_ctx);

            if (!p.audio.buffer.empty())
            {
                status = RecordStatus::Saved;
            }
        }

        void VoiceOver::stopRecording()
        {
            TLRENDER_P();

            if (status != RecordStatus::Recording || p.audio.buffer.empty())
            {
                return;
            }

#if defined(TLRENDER_AUDIO)
            _cleanupAudio();
#endif

            status = RecordStatus::Saved;
            mouse.idx = 0;

            fileName = voiceOverFileName();

            AVFormatContext *fmt_ctx = NULL;
            AVStream *stream = NULL;

            // Allocate the format context for WAV output
            if (avformat_alloc_output_context2(&fmt_ctx, NULL, "wav",
                                               fileName.c_str()) < 0 ||
                !fmt_ctx)
            {
                throw std::runtime_error("Could not create output context");
            }

            // Create new audio stream
            stream = avformat_new_stream(fmt_ctx, NULL);
            if (!stream)
            {
                avformat_free_context(fmt_ctx);
                throw std::runtime_error("Could not create new stream");
            }

            // Configure codec parameters for PCM 32-bit float LE
            AVCodecParameters* codecpar = stream->codecpar;
            codecpar->codec_type     = AVMEDIA_TYPE_AUDIO;
            codecpar->codec_id       = AV_CODEC_ID_PCM_F32LE;
            codecpar->sample_rate    = kSampleRate;
            av_channel_layout_default(&codecpar->ch_layout, kNumChannels);
            codecpar->bit_rate       = codecpar->sample_rate * kNumChannels * sizeof(float) * 8;
            stream->time_base = AVRational{1, codecpar->sample_rate};

            // Open the output file
            if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
                if (avio_open(&fmt_ctx->pb, fileName.c_str(), AVIO_FLAG_WRITE) < 0) {
                    avformat_free_context(fmt_ctx);
                    throw std::runtime_error("Could not open output file");
                }
            }

            // Write the WAV header
            if (avformat_write_header(fmt_ctx, NULL) < 0) {
                if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
                    avio_closep(&fmt_ctx->pb);
                }
                avformat_free_context(fmt_ctx);
                throw std::runtime_error("Error occurred when writing header");
            }

            // Allocate packet
            AVPacket *pkt = av_packet_alloc();
            if (!pkt) {
                if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
                    avio_closep(&fmt_ctx->pb);
                }
                avformat_free_context(fmt_ctx);
                throw std::runtime_error("Could not allocate packet");
            }

            const int channels = kNumChannels;
            const size_t totalSamples = p.audio.buffer.size(); // number of floats = frames * channels
            const size_t frames = totalSamples / channels;
            const size_t bytes = totalSamples * sizeof(float);

            pkt->data = (uint8_t*)p.audio.buffer.data();
            pkt->size = bytes;
            pkt->flags |= AV_PKT_FLAG_KEY;
            pkt->stream_index = stream->index;
            pkt->pts = av_rescale_q(0,
                                    AVRational{1, codecpar->sample_rate},
                                    stream->time_base);
            pkt->dts = pkt->pts;
            pkt->duration = av_rescale_q(frames,
                                         AVRational{1,
                                             codecpar->sample_rate},
                                         stream->time_base);


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

        void VoiceOver::startPlaying()
        {
            TLRENDER_P();

            if (mouse.data.empty())
            {
                LOG_ERROR(_("Cannot play - no mouse recording"));
                return;
            }

            if (p.audio.buffer.empty())
            {
                LOG_ERROR(_("Cannot play - audio annotation is empty"));
                return;
            }

            if (status != RecordStatus::Saved)
            {
                LOG_ERROR(_("Status is not saved"));
                return;
            }

            mouse.idx = 0;
            p.audio.rtCurrentFrame = 0;

#if defined(TLRENDER_AUDIO)
            _cleanupAudio();
            p.audio.playbackFinished.store(false, std::memory_order_release);

            ma_device_config config =
                ma_device_config_init(ma_device_type_playback);
            config.playback.format = ma_format_f32;
            config.playback.channels = kNumChannels;
            config.sampleRate = kSampleRate;
            config.dataCallback = maAudioPlaybackCallback;
            config.pUserData = &p.audio;

            ma_result result = ma_device_init(nullptr, &config,
                                              &p.audioDevice);
            if (result == MA_SUCCESS)
            {
                p.audioDeviceInitialized = true;
                result = ma_device_start(&p.audioDevice);
            }

            if (result == MA_SUCCESS)
            {
                status = RecordStatus::Playing;
            }
            else
            {
                LOG_ERROR("Cannot initialize or start miniaudio playback "
                          "device: " << ma_result_description(result));
                _cleanupAudio();
            }
#endif
        }

        void VoiceOver::stopPlaying()
        {
            TLRENDER_P();

            if (status != RecordStatus::Playing || p.audio.buffer.empty())
            {
                p.audio.rtCurrentFrame = 0;
                mouse.idx = 0;
                return;
            }

            _cleanupAudio();

            status = RecordStatus::Saved;
            p.audio.rtCurrentFrame = 0;
            mouse.idx = 0;
        }

        RecordStatus VoiceOver::getStatus() const
        {
            return status;
        }

        MouseData VoiceOver::getMouseData() const
        {
            TLRENDER_P();

            size_t idx = mouse.idx;
            if (idx >= mouse.data.size())
            {
                idx = mouse.data.size() - 1;
            }

            return mouse.data[idx];
        }

        void VoiceOver::appendMouseData(const MouseData& value)
        {
            mouse.data.push_back(value);
            mouse.idx = mouse.data.size() - 1;
        }

        const math::Vector2f& VoiceOver::getCenter() const
        {
            return center;
        }

        const math::Box2f VoiceOver::getBBox(const float mult) const
        {
            TLRENDER_P();

            return math::Box2f(center.x - 10 * mult,
                               center.y - 10 * mult,
                               20 * mult, 20 * mult);
        }

        void VoiceOver::tick()
        {
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            if (status == RecordStatus::Playing &&
                p.audio.playbackFinished.load(std::memory_order_acquire))
            {
                _cleanupAudio();
            }
#endif

            mouse.idx++;

            if (mouse.idx >= mouse.data.size())
            {
                status = RecordStatus::Saved;
                p.audio.rtCurrentFrame = 0;
                mouse.idx = 0;

                _cleanupAudio();
            }
        }

        void VoiceOver::_cleanupAudio()
        {
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            if (p.audioDeviceInitialized)
            {
                ma_device_uninit(&p.audioDevice);
                p.audioDeviceInitialized = false;
            }
            p.audio.playbackFinished.store(false, std::memory_order_release);
#endif
        }

        void to_json(nlohmann::json& j, const MouseData& value)
        {
            j["pos"] = value.pos;
            j["pressed"] = value.pressed;
        }

        void from_json(const nlohmann::json& j, MouseData& value)
        {
            j.at("pos").get_to(value.pos);
            j.at("pressed").get_to(value.pressed);
        }

        void to_json(nlohmann::json& j, const MouseRecording& value)
        {
            j["data"] = value.data;
        }

        void from_json(const nlohmann::json& j, MouseRecording& value)
        {
            j.at("data").get_to(value.data);
            value.idx = 0;
        }

        void to_json(nlohmann::json& j, const VoiceOver& value)
        {
            j["center"] = value.center;
            j["fileName"] = value.fileName;
            j["mouse"] = value.mouse;
        }

        void from_json(const nlohmann::json& j, VoiceOver& value)
        {
            j.at("center").get_to(value.center);
            j.at("fileName").get_to(value.fileName);
            j.at("mouse").get_to(value.mouse);

            if (file::isReadable(value.fileName))
            {
                value.loadAudio();
            }
            else
            {
                /* xgettext:c++-format */
                std::string err = tl::string::Format(_("Audio annotation file {0} is missing or not exists!")).arg(value.fileName);
                LOG_ERROR(err);
            }
        }
    }
}
