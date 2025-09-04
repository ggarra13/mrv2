// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvVoice/mrvVoiceOver.h"

#include <tlCore/Time.h>

#include <memory>
#include <vector>

namespace mrv
{
    namespace voice
    {
        using namespace tl;
        
        class Annotation
        {
        public:
            Annotation() {};
            Annotation(const otime::RationalTime& frame, const bool allFrames);
            ~Annotation();

            //! Returns true if the current canvas will be empty.
            bool empty() const;

            void push_back(const std::shared_ptr< VoiceOver >&);
            std::shared_ptr< VoiceOver > lastVoiceOver() const;

            //! Remove shape without keeping it in the undo buffer.
            //! Used for laser shapes.
            void remove(const std::shared_ptr< VoiceOver >&);

            void undo();
            void redo();

        public:
            otime::RationalTime time = time::invalidTime;
            std::vector< std::shared_ptr< VoiceOver > > voices;
            std::vector< std::shared_ptr< VoiceOver > > undo_voices;
            bool allFrames = false;
        };

        void to_json(nlohmann::json& json, const Annotation& value);
        void from_json(const nlohmann::json& json, Annotation& value);

    } // namespace void

} // namespace tl
