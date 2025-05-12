#pragma once


#include <tlCore/Color.h>
#include <tlCore/Image.h>

namespace mrv
{
    namespace color
    {
        using namespace tl;
        image::Color4f fromVoidPtr(const void* ptr, const image::PixelType pixelType);
    }
}
