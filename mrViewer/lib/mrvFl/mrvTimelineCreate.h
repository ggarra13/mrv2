
#pragma once

#include <tlTimeline/Audio.h>
#include <tlTimeline/ReadCache.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>

#include <opentimelineio/composable.h>
#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::vector< std::string>& fileNames,
            const std::shared_ptr<system::Context>& context,
            const Options& options = Options(),
            const std::shared_ptr<ReadCache>& readCache = nullptr);
    }
}
