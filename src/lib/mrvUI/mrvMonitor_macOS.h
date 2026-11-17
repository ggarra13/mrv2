// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <string>

bool getDisplayNameForDispID(CGDirectDisplayID dispID, std::string& strOutName);
