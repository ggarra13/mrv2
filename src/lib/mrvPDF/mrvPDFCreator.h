
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <hpdf.h>

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

            HPDF_Doc pdf;
            HPDF_Page page;
            HPDF_Font time_font;
            HPDF_Font note_font;
            HPDF_REAL tw;
            HPDF_REAL height;
            HPDF_REAL width;
            HPDF_Point P;
            HPDF_Point margin;
            HPDF_Image image;

            void addPage();
            void flip_image_y(
                GLubyte* image, const int width, const int height,
                const int channels = 3);
            void create_thumbnail(size_t W, size_t H, const HPDF_BYTE* buffer);
            void print_time(HPDF_Font font, const ViewerUI* ui);
            void print_note(HPDF_Font font, const std::string& text);
        };

    } // namespace pdf
} // namespace mrv
