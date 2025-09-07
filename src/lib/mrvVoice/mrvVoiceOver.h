
#pragma once

#include <tlCore/Box.h>
#include <tlCore/Context.h>
#include <tlCore/ISystem.h>
#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>
#include <tlCore/Util.h>

#include <memory>
#include <vector>

namespace mrv
{
    namespace voice
    {
        using namespace tl;

        enum class RecordStatus
        {
            Stopped,
            Recording,
            Saved,
            Playing,

            Count,
            First = Stopped
        };
        
        TLRENDER_ENUM(RecordStatus);
        TLRENDER_ENUM_SERIALIZE(RecordStatus);
        
        struct MouseData
        {
            math::Vector2f pos;
            bool pressed = false;
        };
        
        class VoiceOver
        {
            TLRENDER_NON_COPYABLE(VoiceOver);

        protected:
            void _init(const std::shared_ptr<system::Context>&,
                       const math::Vector2f&);

            VoiceOver();
            
        public:
            virtual ~VoiceOver();

            //! Create a new system.
            static std::shared_ptr<VoiceOver>
            create(const std::shared_ptr<system::Context>&,
                   const math::Vector2f& center);

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;

            //! Clear the audio and mouse data.
            void clear();
            
            //! Get recorded data.
            std::vector<float> getAudio() const;

            //! Return status
            RecordStatus getStatus() const;
            
            //! Record Voice data.
            void startRecording();

            //! Append Voice data.
            void appendRecording();

            //! Stop recording.
            void stopRecording();
            
            //! Record Voice data.
            void startPlaying();

            //! Stop playing.
            void stopPlaying();

            //! Get the bounding box.
            const math::Box2f getBBox(const float mult = 1.F) const;
            
            //! Get center of current voice over.
            const math::Vector2f& getCenter() const;
            
            //! Get current mouse data for current audio frame.
            MouseData getMouseData() const;

            //! Append mouse data.
            void appendMouseData(const MouseData&);
            
            //! Tick the mouse position playback.
            void tick();
            
        private:
            void _startRecording();
            void _cleanupAudio();
            
            TLRENDER_PRIVATE();
        };
    }
}
