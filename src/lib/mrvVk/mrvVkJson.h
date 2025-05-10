// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvBackend.h"

#include <tlDraw/Annotation.h>

#ifdef VULKAN_BACKEND

namespace mrv
{
    using namespace tl::draw;
    
    //! Translate a nlohmann::json message to a draw::Shape.
    std::shared_ptr< Shape > messageToShape(const nlohmann::json&);

    //! Translate a tl::draw::Shape to a nlohmann::json message.
    nlohmann::json shapeToMessage(const std::shared_ptr< Shape > shape);
}

#endif
