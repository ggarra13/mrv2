
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <mrvWidgets/mrvFPSInput.h>

namespace mrv
{
    namespace decimal
    {
        double floorToOnePlace(double number)
        {
            // Multiply by 10, floor to the nearest integer,
            // and then divide by 10
            return std::floor(number * 10.0) / 10.0;
        }
    } // namespace decimal

    int FPSInput::format(char* buffer)
    {
        std::stringstream ss;
        double number = value();
        double rounded = decimal::floorToOnePlace(number);
        if (w() < 52)
            ss << rounded;
        else
            ss << number;
        return snprintf(buffer, 128, "%s", ss.str().c_str());
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
