
#pragma once

#include <memory>

#include <tlCore/Util.h>

class Fl_Image;

namespace mrv
{
    class MainWindow;

    class FileDragger
    {
    protected:
        FileDragger();

    public:
        ~FileDragger();

        static FileDragger* create();

        MainWindow* window() const;

        void image(Fl_Image*);

    private:
        TLRENDER_PRIVATE();
    };

} // namespace mrv
