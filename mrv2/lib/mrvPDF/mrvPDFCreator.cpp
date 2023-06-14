
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <opentime/rationalTime.h>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvPDF/mrvPDFCreator.h"

#include "mrViewer.h"

jmp_buf env;

namespace
{
    const char* kModule = "pdf";
    const char* kFont = "FreeSans.ttf";

    const unsigned kTitleSize = 16;
} // namespace

#ifdef HPDF_DLL
void __stdcall pdf_error_handler(
    HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
#else
void pdf_error_handler(
    HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
#endif
{
    LOG_ERROR(
        "error_no=" << (HPDF_UINT)error_no
                    << " detail_no=" << (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

namespace mrv
{
    namespace opentime = otime;

    PDFCreator::PDFCreator(
        const std::string& fileName,
        const std::vector< std::shared_ptr< draw::Annotation >>& ann,
        const ViewerUI* ui) :
        file(fileName),
        annotations(ann),
        ui(ui)
    {
        margin.x = 10;
        margin.y = 10;
    }

    void PDFCreator::addPage()
    {
        /* Add a new page object. */
        page = HPDF_AddPage(pdf);

        height = HPDF_Page_GetHeight(page);
        width = HPDF_Page_GetWidth(page);

        /* Print the lines of the page. */
        HPDF_Page_SetLineWidth(page, 1);
        HPDF_Page_Rectangle(page, 50, 50, width - 100, height - 110);
        HPDF_Page_Stroke(page);

        // Get True-Type font_name
        const std::string fontPath = rootpath() + "/fonts/" + kFont;
        if (fs::exists(fontPath))
        {
            const char* font_name =
                HPDF_LoadTTFontFromFile(pdf, fontPath.c_str(), HPDF_TRUE);

            /* Print the title of the page (with positioning center). */
            note_font = HPDF_GetFont(pdf, font_name, "UTF-8");
            HPDF_Page_SetFontAndSize(page, note_font, kTitleSize);

            time_font = HPDF_GetFont(pdf, font_name, "UTF-8");
            HPDF_Page_SetFontAndSize(page, time_font, kTitleSize);
        }
        else
        {
            LOG_WARNING(_("Font not found.  Will not use UTF-8."));

            /* Print the title of the page (with positioning center). */
            note_font = HPDF_GetFont(pdf, "Helvetica", NULL);
            HPDF_Page_SetFontAndSize(page, note_font, kTitleSize);

            time_font = HPDF_GetFont(pdf, "Helvetica", NULL);
            HPDF_Page_SetFontAndSize(page, time_font, kTitleSize);
        }

        auto model = ui->app->filesModel();
        auto item = model->observeA()->get();
        auto path = item->path;
        std::string title =
            path.getBaseName() + path.getNumber() + path.getExtension();

        char page_title[256];
        snprintf(page_title, 256, _("%s - Page %d"), title.c_str(), pageNumber);
        ++pageNumber;

        tw = HPDF_Page_TextWidth(page, page_title);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, (width - tw) / 2, height - 50, page_title);
        HPDF_Page_EndText(page);

        P.x = 55;
        P.y = height - 160;
    }

    void PDFCreator::flip_image_y(
        GLubyte* image, const int width, const int height, const int channels)
    {
        int rowSize = width * channels;
        GLubyte* rowBuffer = new GLubyte[rowSize];

        for (int y = 0; y < height / 2; y++)
        {
            int rowIndex1 = y * rowSize;
            int rowIndex2 = (height - y - 1) * rowSize;

            // Swap rows
            memcpy(rowBuffer, &image[rowIndex1], rowSize);
            memcpy(&image[rowIndex1], &image[rowIndex2], rowSize);
            memcpy(&image[rowIndex2], rowBuffer, rowSize);
        }

        delete[] rowBuffer;
    }

    void
    PDFCreator::create_thumbnail(size_t W, size_t H, const HPDF_BYTE* buffer)
    {
        image =
            HPDF_LoadRawImageFromMem(pdf, buffer, W, H, HPDF_CS_DEVICE_RGB, 8);

        int Xoffset = 0;
        if (W >= H)
        {
            size_t W2 = kThumbnailWidth;

            H = thumbnailHeight = W2 * H / W;
            W = W2;
        }
        else
        {
            size_t H2 = kThumbnailHeight;

            W = H2 * W / H;
            if (W > kThumbnailWidth)
                W = kThumbnailWidth;

            int centerX = (kThumbnailWidth - W) / 2;
            Xoffset = centerX - W / 2;

            H = thumbnailHeight = H2;
        }

        HPDF_Page_DrawImage(page, image, P.x + Xoffset, P.y, W, H);
    }

    void PDFCreator::print_time(HPDF_Font font, const ViewerUI* ui)
    {
        HPDF_Page_BeginText(page);

        HPDF_Page_SetFontAndSize(page, font, 12);
        HPDF_Page_MoveTextPos(
            page, P.x + kThumbnailWidth + margin.x, P.y + thumbnailHeight - 12);

        std::string buf =
            tl::string::Format(_("Frame {0} - Timecode: {1} - {2} Secs."))
                .arg(time.to_frames())
                .arg(time.to_timecode())
                .arg(time.to_seconds());

        // Write out each line
        HPDF_Page_ShowText(page, buf.c_str());

        HPDF_Page_EndText(page);
    }

    void PDFCreator::print_note(HPDF_Font font, const std::string& text)
    {
        HPDF_Page_BeginText(page);

        HPDF_Page_SetFontAndSize(page, font, 9);
        HPDF_Page_MoveTextPos(
            page, P.x + kThumbnailWidth + margin.x, P.y + thumbnailHeight - 27);

        // Split text into lines
        stringArray lines;
        split_string(lines, text, "\n");

        // Write out each line
        for (const auto& line : lines)
        {
            HPDF_Page_ShowText(page, line.c_str());
            HPDF_Page_MoveTextPos(page, 0, -12);
        }

        HPDF_Page_EndText(page);
    }

    bool PDFCreator::create()
    {
        pdf = HPDF_New(pdf_error_handler, NULL);
        if (!pdf)
        {
            LOG_ERROR("error: cannot create PdfDoc object");
            return false;
        }

        if (setjmp(env))
        {
            HPDF_Free(pdf);
            return false;
        }

        HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);

        HPDF_UseUTFEncodings(pdf);
        HPDF_SetCurrentEncoder(pdf, "UTF-8");

        addPage();

        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        bool presentation = view->getPresentationMode();

        view->setPresentationMode(true);
        view->redraw();
        // flush is needed
        Fl::flush();
        view->flush();
        Fl::check();

        auto viewportSize = view->getViewportSize();
        auto renderSize = view->getRenderSize();

        int X = 0, Y = 0;
        if (viewportSize.w >= renderSize.w && viewportSize.h >= renderSize.h)
        {
            view->setViewZoom(1.0);
            view->centerView();
            view->redraw();
            // flush is needed
            Fl::flush();

            X = (viewportSize.w - renderSize.w) / 2;
            Y = (viewportSize.h - renderSize.h) / 2;
        }
        else
        {
            view->frameView();
            renderSize.w = viewportSize.w;
            renderSize.h = viewportSize.h;
            LOG_WARNING(_("Image too big.  "
                          "Will save the viewport size."));
        }

        std::string msg =
            tl::string::Format(_("Viewport Size: {0}  Render Size: {1}"))
                .arg(viewportSize)
                .arg(renderSize);
        LOG_INFO(msg);

        msg = tl::string::Format("viewZoom: {2} X: {3} Y: {4}")
                  .arg(view->viewZoom())
                  .arg(X)
                  .arg(Y);
        LOG_INFO(msg);

        // Don't send any tcp updates
        tcp->lock();

        // Turn off hud so it does not get captured by glReadPixels.
        bool hud = view->getHudActive();
        view->setHudActive(false);
        view->setActionMode(ActionMode::kScrub);

        TimelineClass* c = ui->uiTimeWindow;

        const GLenum format = GL_RGB;
        const GLenum type = GL_UNSIGNED_BYTE;

        GLubyte* buffer = new GLubyte[renderSize.w * renderSize.h * 3];

        for (const auto& annotation : annotations)
        {
            int64_t frame = annotation->frame;
            time = otio::RationalTime(frame, c->uiFPS->value());
            player->seek(time);

            view->centerView();
            view->redraw();
            view->flush();
            Fl::check();

            glReadBuffer(GL_FRONT);
            glReadPixels(
                X, Y, renderSize.w, renderSize.h, format, type, buffer);

            flip_image_y(buffer, renderSize.w, renderSize.h, 3);

            create_thumbnail(renderSize.w, renderSize.h, buffer);

            // print time
            print_time(time_font, ui);

            for (const auto& shape : annotation->shapes)
            {
                auto note = dynamic_cast< draw::NoteShape* >(shape.get());
                if (!note)
                    continue;

                print_note(note_font, note->text);
            }

            P.y -= (thumbnailHeight + margin.y);

            if (P.y <= 50)
            {
                addPage();
            }
        }

        delete[] buffer;

        view->setPresentationMode(presentation);
        view->setHudActive(hud);
        tcp->unlock();

        try
        {
            // do page description processes
            HPDF_SaveToFile(pdf, file.c_str());
        }
        catch (...)
        {
            HPDF_Free(pdf);
            return false;
        }

        HPDF_Free(pdf);
        return true;
    }

} // namespace mrv
