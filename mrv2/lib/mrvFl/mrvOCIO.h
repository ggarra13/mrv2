#pragma once

#include <string>

namespace mrv
{
    namespace image
    {
        std::string ocioIcs();
        void setOcioIcs(const std::string&);

        std::string ocioLook();
        void setOcioLook(const std::string&);

        std::string ocioView();
        void setOcioView(const std::string&);

    } // namespace image
} // namespace mrv
