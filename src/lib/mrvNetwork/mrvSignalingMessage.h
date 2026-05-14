
#pragma once

#include <string>

namespace mrv
{
    struct SignalingMessage
    {
        std::string peerId;
        std::string type;

        std::string sdp;
        
        std::string candidate;
        std::string mid;
    };
}
