// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <ApplicationServices/ApplicationServices.h>

//! Localized name of the display with the given CGDirectDisplayID.
bool getDisplayNameForDispID(CGDirectDisplayID dispID, std::string& strOutName);
