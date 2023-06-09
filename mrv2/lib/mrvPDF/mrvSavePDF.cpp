
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <hpdf.h>

#include <opentime/rationalTime.h>

#include <tlCore/StringFormat.h>

#include <tlGlad/gl.h>

#include <FL/Fl_RGB_Image.H>

#include "mrvCore/mrvString.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvPDF/mrvSavePDF.h"

#include "mrViewer.h"

jmp_buf env;

#ifdef HPDF_DLL
void __stdcall
#else
void
#endif
    error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
{
    printf(
        "ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
        (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

namespace
{
    const char* kModule = "pdf";
}

namespace mrv
{
    namespace opentime = otime;

    class PDFCreator
    {
    protected:
        const int kThumbnailWidth = 150;

    public:
        PDFCreator(
            const std::string& fileName,
            const std::vector< std::shared_ptr< draw::Annotation >>&
                annotations,
            const ViewerUI* ui) :
            file(fileName),
            ui(ui)
        {
            pdf = HPDF_New(error_handler, NULL);
            if (!pdf)
            {
                LOG_ERROR("error: cannot create PdfDoc object");
                return;
            }

            if (setjmp(env))
            {
                HPDF_Free(pdf);
                return;
            }

            HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);

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
            auto renderSize = view->getRenderSize();
            const auto& viewportSize = view->getViewportSize();
            int X = 0, Y = 0;
            if (viewportSize.w >= renderSize.w &&
                viewportSize.h >= renderSize.h)
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
                tl::string::Format(_("Viewport Size: {0}  Render Size: {1}  "
                                     "viewZoom: {2} X: {3} Y: {4}"))
                    .arg(viewportSize)
                    .arg(renderSize)
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

            P.x = 55;
            P.y = height - 150;

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

                FlipImageOnY(buffer, renderSize.w, renderSize.h, 3);

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

                P.y -= 100;

                if (P.y <= 50)
                {
                    addPage();
                    P.y = height - 150;
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
                return;
            }

            HPDF_Free(pdf);
        }

    private:
        std::string file;
        const ViewerUI* ui;

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
        HPDF_Image image;

        void addPage()
        {
            /* Add a new page object. */
            page = HPDF_AddPage(pdf);

            height = HPDF_Page_GetHeight(page);
            width = HPDF_Page_GetWidth(page);

            /* Print the lines of the page. */
            HPDF_Page_SetLineWidth(page, 1);
            HPDF_Page_Rectangle(page, 50, 50, width - 100, height - 110);
            HPDF_Page_Stroke(page);

            /* Print the title of the page (with positioning center). */
            note_font = HPDF_GetFont(pdf, "Helvetica", NULL);
            HPDF_Page_SetFontAndSize(page, note_font, 24);

            time_font = HPDF_GetFont(pdf, "Helvetica", NULL);
            HPDF_Page_SetFontAndSize(page, time_font, 24);

            auto model = ui->app->filesModel();
            auto item = model->observeA()->get();
            auto path = item->path;
            std::string title =
                path.getBaseName() + path.getNumber() + path.getExtension();

            char page_title[256];
            snprintf(
                page_title, 256, _("%s - Page %d"), title.c_str(), pageNumber);
            ++pageNumber;

            tw = HPDF_Page_TextWidth(page, page_title);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, (width - tw) / 2, height - 50, page_title);
            HPDF_Page_EndText(page);
        }

        void FlipImageOnY(
            GLubyte* image, const int width, const int height,
            const int channels = 3)
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

        void create_thumbnail(size_t W, size_t H, const HPDF_BYTE* buffer)
        {
            image = HPDF_LoadRawImageFromMem(
                pdf, buffer, W, H, HPDF_CS_DEVICE_RGB, 8);

            size_t W2 = kThumbnailWidth;

            H = thumbnailHeight = W2 * H / W;
            W = W2;

            HPDF_Page_DrawImage(page, image, P.x, P.y, W, H);
        }

        void print_time(HPDF_Font font, const ViewerUI* ui)
        {
            HPDF_Page_BeginText(page);

            HPDF_Page_SetFontAndSize(page, font, 12);
            HPDF_Page_MoveTextPos(
                page, P.x + kThumbnailWidth + 22, P.y + thumbnailHeight - 12);

            std::string buf =
                tl::string::Format(_("Frame {0} - Timecode: {1} - {2} Seconds"))
                    .arg(time.to_frames())
                    .arg(time.to_timecode())
                    .arg(time.to_seconds());

            // Write out each line
            HPDF_Page_ShowText(page, buf.c_str());

            HPDF_Page_EndText(page);
        }

        void print_note(HPDF_Font font, const std::string& text)
        {
            HPDF_Page_BeginText(page);

            HPDF_Page_SetFontAndSize(page, font, 9);
            HPDF_Page_MoveTextPos(
                page, P.x + kThumbnailWidth + 22, P.y + thumbnailHeight - 27);

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
    };

    void save_pdf(
        const std::string& file,
        const std::vector< std::shared_ptr< draw::Annotation >>& annotations,
        const ViewerUI* ui)
    {
        PDFCreator pdf(file, annotations, ui);
    }
} // namespace mrv
