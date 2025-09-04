// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvVoice/mrvAnnotation.h"

#include <algorithm>

namespace mrv
{
    namespace voice
    {
        Annotation::Annotation(
            const otime::RationalTime& inTime, const bool inAllFrames)
        {
            time = inTime;
            allFrames = inAllFrames;
        }

        Annotation::~Annotation() {}

        bool Annotation::empty() const
        {
            return voices.empty();
        }

        void Annotation::push_back(const std::shared_ptr< VoiceOver >& shape)
        {
            voices.push_back(shape);
            undo_voices.clear();
        }

        void Annotation::remove(const std::shared_ptr< VoiceOver >& shape)
        {
            auto it = std::find(voices.begin(), voices.end(), shape);
            if (it != voices.end())
            {
                voices.erase(it);
            }
        }

        std::shared_ptr< VoiceOver > Annotation::lastVoiceOver() const
        {
            if (voices.empty())
                return nullptr;
            return voices.back();
        }

        void Annotation::undo()
        {
            if (voices.empty())
                return;

            const auto& shape = voices.back();
            undo_voices.push_back(shape);
            voices.pop_back();
        }

        void Annotation::redo()
        {
            if (undo_voices.empty())
                return;

            const auto& shape = undo_voices.back();
            voices.push_back(shape);
            undo_voices.pop_back();
        }

    } // namespace draw
} // namespace tl
