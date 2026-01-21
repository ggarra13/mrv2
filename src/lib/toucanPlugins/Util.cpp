// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "Util.h"

#include <OpenFX/ofxImageEffect.h>

#include <cstring>

OIIO::ImageBuf propSetToBuf(OfxPropertySuiteV1* suite, OfxPropertySetHandle handle)
{
    OfxRectI bounds;
    suite->propGetIntN(handle, kOfxImagePropBounds, 4, &bounds.x1);

    int components = 0;
    char* s = nullptr;
    suite->propGetString(handle, kOfxImageEffectPropComponents, 0, &s);
    if (strcmp(s, kOfxImageComponentAlpha) == 0)
    {
        components = 1;
    }
    else if (strcmp(s, kOfxImageComponentRGB) == 0)
    {
        components = 3;
    }
    else if (strcmp(s, kOfxImageComponentRGBA) == 0)
    {
        components = 4;
    }

    int pixelDepth = 0;
    suite->propGetString(handle, kOfxImageEffectPropPixelDepth, 0, &s);
    if (strcmp(s, kOfxBitDepthByte) == 0)
    {
        pixelDepth = 1;
    }
    else if (strcmp(s, kOfxBitDepthShort) == 0)
    {
        pixelDepth = 2;
    }
    else if (strcmp(s, kOfxBitDepthFloat) == 0)
    {
        pixelDepth = 4;
    }

    int rowBytes = 0;
    suite->propGetInt(handle, kOfxImagePropRowBytes, 0, &rowBytes);

    void* p = nullptr;
    suite->propGetPointer(handle, kOfxImagePropData, 0, &p);

    OIIO::TypeDesc format = OIIO::TypeDesc::UNKNOWN;
    switch (pixelDepth)
    {
    case 1: format = OIIO::TypeDesc::UINT8; break;
    case 2: format = OIIO::TypeDesc::UINT16; break;
    case 4: format = OIIO::TypeDesc::FLOAT; break;
    default: break;
    }
    OIIO::ImageSpec spec(
        bounds.x2 - bounds.x1,
        bounds.y2 - bounds.y1,
        components,
        format);

    return OIIO::ImageBuf(
        spec,
        p,
        components * pixelDepth,
        rowBytes,
        0);
}
