
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <tlCore/Vector.h>

#include <FL/Fl_PDF_File_Surface.H>

#include "mrvGL/mrvGLErrors.h"

#include "mrvDraw/Annotation.h"

class ViewerUI;

namespace mrv
{
    namespace pdf
    {

        using namespace tl;

        class Creator
        {
        protected:
            const int kThumbnailWidth = 175;
            const int kThumbnailHeight = 80;

        public:
            Creator(
                const std::string& fileName,
                const Fl_PDF_File_Surface::Page_Format& pageFormat,
                const std::vector< std::shared_ptr< draw::Annotation >>&
                    annotations,
                const ViewerUI* ui);

            bool create();

        private:
            std::string file;
            const ViewerUI* ui;
            const std::vector< std::shared_ptr< draw::Annotation >>&
                annotations;

            otime::RationalTime time;

            unsigned pageNumber = 1;

            unsigned thumbnailHeight;

            Fl_PDF_File_Surface pdf;
            Fl_PDF_File_Surface::Page_Format page_format;
                    
            int width = 0, height = 0;

            int time_margin, image_margin;
            
            math::Vector2i P;
            math::Vector2i margin;

            void addPage();
            void flip_image_y(
                GLubyte* image, const int width, const int height,
                const int channels = 3);
            void create_thumbnail(size_t W, size_t H,
                                  const unsigned char* buffer);
            void print_time(Fl_Font font);
            void print_note(Fl_Font font, const std::string& text);


            void wait();
        };

    } // namespace pdf
} // namespace mrv
