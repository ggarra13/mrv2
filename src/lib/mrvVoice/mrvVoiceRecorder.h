
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
        
        class Recorder
        {
            TLRENDER_NON_COPYABLE(Recorder);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Recorder();
            
        public:
            virtual ~Recorder();

            //! Create a new system.
            static std::shared_ptr<Recorder>
            create(const std::shared_ptr<system::Context>&);

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;
            
            //! Get recorded data.
            std::vector<float> getData() const;
            
            //! Record data from default microphone.
            void record();

            //! Stop recording.
            void stop();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
