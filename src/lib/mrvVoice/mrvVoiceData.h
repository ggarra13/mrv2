
#pragma once

#include <tlCore/Vector.h>
#include <tlCore/Color.h>

#include <string>
#include <vector>

namespace mrv
{
    namespace voice
    {

        class MouseData
        {
            math::Vector2i pos;
            bool pressed = false;
        };

        class AudioData
        {
            std::string fileName;
            std::vector<float> audio; // mono
            float speed = 1.0;
        };

        class VoiceData
        {
            image::Color4f labelColor;
            AudioData audioData;
            MouseData mouseData;
        };

        class VoiceOver
        {
            int64_t   frame;
            int       currentVoiceData = -1;
            VoiceData voiceData;
        };

    }
}
