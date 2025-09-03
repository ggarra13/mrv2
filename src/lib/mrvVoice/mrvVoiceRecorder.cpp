
#include "mrvVoice/mrvVoiceRecorder.h"

#include <tlCore/Audio.h>
#include <tlCore/AudioSystem.h>

#if defined(TLRENDER_AUDIO)
#    include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <thread>

#if defined(TLRENDER_AUDIO)
static int rtAudioCallback(
    void* outputBuffer, void* inputBuffer, unsigned int nFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    std::cerr << "recording " << nFrames << std::endl;
    return 0;
}

static void rtAudioErrorCallback(
    RtAudioError::Type type, const std::string& errorText)
{
}
#endif // TLRENDER_AUDIO

namespace mrv
{
    namespace voice
    {

        struct Recorder::Private
        {
            std::weak_ptr<system::Context> context;
        
            std::vector<float> data;

            bool running = false;
#if defined(TLRENDER_AUDIO)
            std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
            std::thread thread;
            
            size_t rtAudioCurrentFrame = 0;
            
        };

        void Recorder::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
        }

        Recorder::Recorder() :
            _p(new Private)
        {
        }

        Recorder::~Recorder()
        {
            stop();
        }

        std::shared_ptr<Recorder>
        Recorder::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<Recorder>();
            if (!out)
            {
                out = std::shared_ptr<Recorder>(new Recorder);
                out->_init(context);
            }
            return out;
        }

        //! Get the context.
        const std::weak_ptr<system::Context>& Recorder::getContext() const
        {
            return _p->context;
        }
        
        void Recorder::record()
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
                    {
                        std::stringstream ss;
                        ss << "Cannot create RtAudio instance: " << e.what();
                        context->log(
                            "mrv::voice::Recorder", ss.str(), log::Type::Error);
                    }
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
                        rtParameters.nChannels = 1;
                        unsigned int rtBufferFrames = 256;
                        p.rtAudio->openStream(
                            nullptr, &rtParameters,
                            RTAUDIO_FLOAT32,
                            44100,
                            &rtBufferFrames, rtAudioCallback,
                            this, nullptr,
                            rtAudioErrorCallback);
                        p.rtAudio->startStream();
                    }
                    catch (const std::exception& e)
                    {
                        std::stringstream ss;
                        ss << "Cannot open audio stream: "
                           << e.what();
                        context->log(
                            "tl::timeline::Player", ss.str(),
                            log::Type::Error);
                    }
                }
            }
        }
#endif // TLRENDER_AUDIO


        void Recorder::stop()
        {
            TLRENDER_P();
            
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

    }
}
