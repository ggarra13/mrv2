// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cmath>
#include <iomanip>
#include <sstream>

#include <mrvWidgets/mrvFPSInput.h>

namespace
{
    double floorToOnePlace(double number)
    {
        // Multiply by 10, floor to the nearest integer,
        // and then divide by 10
        return std::floor(number * 10.0) / 10.0;
    }
} // namespace

namespace mrv
{
    int FPSInput::format(char* buffer)
    {
        char temp[64];
        int length;

        double number = value();

        if (w() < 52.0)
        {
            length = snprintf(temp, 64, "%.1f", floorToOnePlace(number));
        }
        else
        {
            length = snprintf(temp, 64, "%.3f", number);
        }

        strcpy(buffer, temp);
        return length;
    }

    void FPSInput::resize(int X, int Y, int W, int H)
    {
        Fl_Value_Input::resize(X, Y, W, H);
        char buf[128];
        format(buf);
        input.value(buf);
        input.mark(input.insert_position()); // turn off selection highlight
        redraw();
    }
} // namespace mrv
