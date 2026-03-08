#pragma once

#include <tlCore/Vector.h>

namespace tl
{
    namespace monitor
    {
        struct Capabilities
        {
            bool hdr_supported = false;
            bool hdr_enabled = false;
            
            //! Limits.
            float min_nits = 0.001F;
            float max_nits = 100.F;

            //! Chromaticities.
            math::Vector2f red;
            math::Vector2f green;
            math::Vector2f blue;
            math::Vector2f white;
            
            bool operator==(const Capabilities&) const;
            bool operator!=(const Capabilities&) const;
        };
    }
}

#include <tlCore/MonitorInline.h>
