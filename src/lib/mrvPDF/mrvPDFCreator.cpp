
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <opentime/rationalTime.h>

#include <tlCore/StringFormat.h>

#include <tlUI/ThumbnailSystem.h>

#include <FL/fl_draw.H>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvIO.h"
#include "mrvNetwork/mrvTCP.h"

#include "mrvPDF/mrvPDFCreator.h"

#include "mrViewer.h"

jmp_buf env;

namespace
{
    const char* kModule = "pdf";

    const unsigned kTitleSize = 16;
} // namespace


namespace
{
    // Helper function to perform cubic interpolation
    double cubicInterpolate(double p0, double p1, double p2,
                            double p3, double x)
    {
        return p1 + 0.5 * x * (p2 - p0 + x * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 + x * (3.0 * (p1 - p2) + p3 - p0)));
    }

    // Function to get pixel value with boundary checking for each channel
    unsigned char getPixel(const unsigned char* image,
                           size_t width, size_t height,
                           int x, int y, size_t channel)
    {
        // Ensure coordinates are within bounds
        x = std::clamp(x, 0, static_cast<int>(width - 1));
        y = std::clamp(y, 0, static_cast<int>(height - 1));
        return image[(y * width + x) * 3 + channel]; // Each pixel has 3 channels (R, G, B)
    }

    // Bicubic interpolation function for scaling a 3-channel (RGB) image
    unsigned char* scaleImageBicubic(const unsigned char* image,
                                     size_t srcWidth, size_t srcHeight,
                                     size_t dstWidth, size_t dstHeight)
    {
        double scaleX = static_cast<double>(dstWidth) / srcWidth;
        double scaleY = static_cast<double>(dstHeight) / srcHeight;
        std::cerr << "scale=" << scaleX << " " << scaleY << std::endl;
        
        // Allocate memory for the scaled image
        unsigned char* scaledImage = new unsigned char[dstWidth * dstHeight * 3]; // 3 channels per pixel (RGB)

        // Loop through the destination image
        for (size_t j = 0; j < dstHeight; ++j) {
            for (size_t i = 0; i < dstWidth; ++i) {
                // Map destination coordinates back to source
                double gx = i / scaleX;
                double gy = j / scaleY;

                int x = static_cast<int>(gx);
                int y = static_cast<int>(gy);

                // Fractional part of the coordinate
                double dx = gx - x;
                double dy = gy - y;

                // Iterate over each channel (R, G, B)
                for (size_t channel = 0; channel < 3; ++channel) {
                    // Bicubic interpolation
                    double patch[4][4];
                    for (int m = -1; m <= 2; ++m)
                    {
                        for (int n = -1; n <= 2; ++n)
                        {
                            patch[m + 1][n + 1] = static_cast<double>(getPixel(image, srcWidth, srcHeight, x + m, y + n, channel));
                        }
                    }

                    // Interpolate along x direction for each row
                    double col[4];
                    for (int k = 0; k < 4; ++k) {
                        col[k] = cubicInterpolate(patch[k][0], patch[k][1], patch[k][2], patch[k][3], dx);
                    }

                    // Interpolate along y direction using the results of x interpolation
                    double value = cubicInterpolate(col[0], col[1], col[2], col[3], dy);

                    // Clamp the result to the valid range [0, 255] and assign it to the scaled image
                    scaledImage[(j * dstWidth + i) * 3 + channel] = static_cast<unsigned char>(std::clamp(value, 0.0, 255.0));
                }
            }
        }

        return scaledImage;
    }


}



namespace mrv
{
    namespace pdf
    {
        namespace opentime = otime;

        Creator::Creator(
            const std::string& fileName,
            const std::vector< std::shared_ptr< draw::Annotation >>& ann,
            const ViewerUI* ui) :
            file(fileName),
            annotations(ann),
            ui(ui)
        {
            margin.x = 10;
            margin.y = 10;

            time_margin = 27;
            image_margin = 12;
        }

        void Creator::addPage()
        {
            /* Add a new page object. */
            int err = pdf.begin_page();
            if (err)
            {
                LOG_ERROR("Error in pdf.begin_page");
                return;
            }

            pdf.printable_rect(&width, &height);
            
            /* Print the lines of the page. */
            fl_color(FL_BLACK);
            
            fl_line_style(FL_SOLID, 1);
            fl_rect(50, 50, width - 100, height - 100);

            auto model = App::app->filesModel();
            auto item = model->observeA()->get();
            auto path = item->path;
            std::string title =
                path.getBaseName() + path.getNumber() + path.getExtension();

            char page_title[256];
            snprintf(
                page_title, 256, _("%s - Page %d"), title.c_str(), pageNumber);
            ++pageNumber;

            // Draw Page Title
            fl_font(FL_HELVETICA, 20);
            int tw, th;
            fl_measure(page_title, tw, th);
            fl_draw(page_title, (width - tw) / 2, th);

            P.x = 55;
            P.y = 55;
            
            Fl_Surface_Device::pop_current();
        }

        void Creator::flip_image_y(
            GLubyte* image, const int width, const int height,
            const int channels)
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
        Creator::create_thumbnail(size_t W, size_t H,
                                  const unsigned char* buffer)
        {
            Fl_Surface_Device::push_current(&pdf);

            size_t srcWidth = W;
            size_t srcHeight = H;
            
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

            unsigned char* scaledImage = scaleImageBicubic(buffer, srcWidth,
                                                           srcHeight, W, H);

            fl_draw_image(scaledImage, P.x + Xoffset, P.y, W, H);
            
            delete [] scaledImage;
            Fl_Surface_Device::pop_current();
        }

        void Creator::print_time(Fl_Font font, const ViewerUI* ui)
        {
            Fl_Surface_Device::push_current(&pdf);
            
            fl_color(FL_BLACK);
            fl_font(font, 11);
            
            std::string buf =
                tl::string::Format(_("Frame {0} - Timecode: {1} - {2} Secs."))
                    .arg(time.to_frames())
                    .arg(time.to_timecode())
                    .arg(time.to_seconds());

            int tw, th;
            fl_measure(buf.c_str(), tw, th);
            fl_draw(buf.c_str(), P.x + kThumbnailWidth + margin.x,
                    P.y + th / 2);

            Fl_Surface_Device::pop_current();
        }

        void Creator::print_note(Fl_Font font, const std::string& text)
        {
            Fl_Surface_Device::push_current(&pdf);


            fl_color(FL_BLACK);
            fl_font(font, 9);

            int tw, th;

            int Y = P.y + time_margin;
            
            // Split text into lines
            auto lines = string::split(text, '\n');

            // Write out each line
            for (const auto& line : lines)
            {
                fl_draw(line.c_str(), P.x + kThumbnailWidth + margin.x, Y);

                fl_measure(line.c_str(), tw, th);
                Y += th;
            }

            Fl_Surface_Device::pop_current();
        }

        bool Creator::create()
        {
            char* err_message;
            int err = pdf.begin_document(file.c_str(), Fl_Paged_Device::A4,
                                         Fl_Paged_Device::PORTRAIT, &err_message);
            if (err)
            {
                LOG_ERROR(err_message);
                return false;
            }

            addPage();

            auto view = ui->uiView;
            auto player = view->getTimelinePlayer();
            bool presentation = view->getPresentationMode();
            // Turn off hud so it does not get captured by glReadPixels.
            bool hud = view->getHudActive();

            view->setPresentationMode(true);
            view->redraw();
            // flush is needed
            Fl::flush();
            view->flush();
            Fl::check();

            auto viewportSize = view->getViewportSize();
            auto renderSize = view->getRenderSize();

            int X = 0, Y = 0;
            if (viewportSize.w < renderSize.w ||
                viewportSize.h < renderSize.h)
            {
                LOG_WARNING(_("Image too big.  "
                              "Will save the viewport size."));
            }

            view->frameView();
            renderSize.w = viewportSize.w;
            renderSize.h = viewportSize.h;
                
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

            view->setHudActive(false);
            view->setActionMode(ActionMode::kScrub);
            
            TimelineClass* c = ui->uiTimeWindow;

            const GLenum format = GL_RGB;
            const GLenum type = GL_UNSIGNED_BYTE;

            GLubyte* buffer = new GLubyte[renderSize.w * renderSize.h * 3];

            for (const auto& annotation : annotations)
            {
                time = annotation->time;
                player->seek(time);

                view->make_current();
                view->centerView();
                view->redraw();
                view->flush();
                Fl::check();

                view->make_current();
                glReadBuffer(GL_FRONT);
                glReadPixels(
                    X, Y, renderSize.w, renderSize.h, format, type, buffer);

                flip_image_y(buffer, renderSize.w, renderSize.h, 3);

                create_thumbnail(renderSize.w, renderSize.h, buffer);

                // print time
                print_time(FL_HELVETICA, ui);

                for (const auto& shape : annotation->shapes)
                {
                    auto note = dynamic_cast< draw::NoteShape* >(shape.get());
                    if (!note)
                        continue;

                    print_note(FL_HELVETICA, note->text);
                }
            
                P.y += thumbnailHeight + image_margin;
            
                if (P.y > height - 50 - thumbnailHeight)
                {
                    pdf.end_page();
                    
                    addPage();
                }
            }
            
            delete[] buffer;
            

            pdf.end_page();

            view->setPresentationMode(presentation);
            view->setHudActive(hud);
            
            tcp->unlock();

            pdf.end_job();

            return true;
        }

    } // namespace pdf
} // namespace mrv
