
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#include "mrvCore/mrvFonts.h"

namespace mrv
{
    namespace fonts
    {

        std::vector<std::string> list()
        {
            std::vector<std::string> out;
            auto numFonts = Fl::set_fonts("-*");
            for (unsigned i = 0; i < numFonts; ++i)
            {
                int attrs = 0;
                const char* fontName = Fl::get_font_name((Fl_Font)i, &attrs);
                if (!fontName)
                    continue;
                out.push_back(fontName);
            }
            return out;
        }

        int compare(const std::string& fontName)
        {
            const std::vector<std::string>& fontList = list();
            int out = FL_HELVETICA;
            for (int i = 0; i < fontList.size(); ++i)
            {
                if (fontName == fontList[i])
                {
                    out = i;
                    break;
                }
            }
            return out;
        }
    } // namespace fonts
} // namespace mrv
