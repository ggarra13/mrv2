#pragma once

#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <string>

namespace tl
{
    namespace timeline
    {
        enum class Debanding {
            kNone,
            Low,
            Medium,
            High,
            
            Count,
            First = kNone
        };
        
        TLRENDER_ENUM(Debanding);
        TLRENDER_ENUM_SERIALIZE(Debanding);

        struct ShaderOptions
        {
            Debanding debanding = Debanding::kNone;
            
            bool operator==(const ShaderOptions&) const;
            bool operator!=(const ShaderOptions&) const;
        };
    }
}
#include <tlTimeline/ShaderOptionsInline.h>
