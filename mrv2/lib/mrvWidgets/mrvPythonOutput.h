
#pragma once

#include "mrvWidgets/mrvLogDisplay.h"

namespace mrv
{
    class PythonOutput : public LogDisplay
    {
    public:
        PythonOutput( int X, int Y, int W, int H, const char* L = 0 );

        int handle( int e ) override;
    };
}
