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

        void to_json(nlohmann::json& j, const Annotation& value)
        {
            nlohmann::json voices;
            for (auto& voice_shared : value.voices)
            {
                const auto voice = voice_shared.get();
                voices.push_back(*voice);
            }
            
            j = nlohmann::json{
                {"voices", voices},
                {"allFrames", value.allFrames}
            };
        }
        
        nlohmann::json voiceOverToMessage(
            const std::shared_ptr< voice::VoiceOver > voice)
        {
            nlohmann::json msg;
            msg = *voice.get();
            return msg;
        }
        
        std::shared_ptr< voice::VoiceOver >
        messageToVoiceOver(const nlohmann::json& json)
        {
            auto voice = std::make_shared< voice::VoiceOver >();
            json.get_to(*voice.get());
            return voice;
        }
        
        void from_json(const nlohmann::json& j, Annotation& value)
        {
            const nlohmann::json& voices = j["voices"];

            value.voices.clear();
            for (const auto& voice : voices)
            {
                value.voices.push_back(messageToVoiceOver(voice));
            }
            j.at("allFrames").get_to(value.allFrames);
        }
        
        std::shared_ptr< Annotation >
        messageToAnnotation(const nlohmann::json& json)
        {
            auto annotation = std::make_shared< Annotation >();
            json.get_to(*annotation.get());
            return annotation;
        }
        
    } // namespace voice
} // namespace tl
