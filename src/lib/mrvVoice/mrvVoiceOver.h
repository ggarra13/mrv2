
#pragma once

#include <tlCore/Context.h>
#include <tlCore/ISystem.h>
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
            Playing
        };
        
        class VoiceOver
        {
            TLRENDER_NON_COPYABLE(VoiceOver);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            VoiceOver();
            
        public:
            virtual ~VoiceOver();

            //! Create a new system.
            static std::shared_ptr<VoiceOver>
            create(const std::shared_ptr<system::Context>&);

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;
            
            //! Get recorded data.
            std::vector<float> getData() const;

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

        private:
            TLRENDER_PRIVATE();
        };
    }
}
