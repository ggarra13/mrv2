// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/Audio.h>
#include <tlTimeline/ReadCache.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>



#include <opentimelineio/composable.h>
#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>



#include "mrvApp/mrvFilesModel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wc++17-compat"

namespace tl
{
    namespace timeline
    {
		using namespace tl;
		using namespace tl::timeline;

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::vector<std::shared_ptr<mrv::FilesModelItem> >& fileItems,
            const std::shared_ptr<system::Context>& context,
            const Options& options = Options(),
            const std::shared_ptr<ReadCache>& readCache = nullptr);

#pragma GCC diagnostic pop
		
    }
} // namespace tl
