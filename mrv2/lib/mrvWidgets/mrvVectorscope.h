// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Group.H>

#include <tlCore/Util.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class Vectorscope : public Fl_Group
    {
    public:
        Vectorscope(int X, int Y, int W, int H, const char* L = 0);
        ~Vectorscope();

        virtual void draw() override;

        void update(const area::Info& info);

        void main(ViewerUI* m);
        ViewerUI* main();

    protected:
        void draw_grid() noexcept;
        void draw_pixel(image::Color4f& rgb) const noexcept;
        void draw_pixels() const noexcept;

        TLRENDER_PRIVATE();
    };

} // namespace mrv
