// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <OpenFX/ofxProperty.h>

#include <OpenImageIO/imagebuf.h>

//! Convert from a property set.
OIIO::ImageBuf propSetToBuf(OfxPropertySuiteV1*, OfxPropertySetHandle);
