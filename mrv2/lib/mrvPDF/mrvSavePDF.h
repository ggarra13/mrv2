
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <hpdf.h>

#include <tlGlad/gl.h>

#include "mrvDraw/Annotation.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class PDFCreator
    {
    protected:
        const int kThumbnailWidth = 175;

    public:
        PDFCreator(
            const std::string& fileName,
            const std::vector< std::shared_ptr< draw::Annotation >>&
                annotations,
            const ViewerUI* ui);

        bool create();

    private:
        std::string file;
        const ViewerUI* ui;
        const std::vector< std::shared_ptr< draw::Annotation >>& annotations;

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

        const char* font_name = nullptr;

        void addPage();
        void flip_image_y(
            GLubyte* image, const int width, const int height,
            const int channels = 3);
        void create_thumbnail(size_t W, size_t H, const HPDF_BYTE* buffer);
        void print_time(HPDF_Font font, const ViewerUI* ui);
        void print_note(HPDF_Font font, const std::string& text);
    };

    bool save_pdf(const std::string& file, const ViewerUI* ui);
} // namespace mrv
