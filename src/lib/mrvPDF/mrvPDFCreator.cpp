
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <opentime/rationalTime.h>

#include <tlCore/StringFormat.h>

#include <tlIO/System.h>

#include <tlGL/Init.h>

#include <tlUI/ThumbnailSystem.h>

#include <FL/fl_draw.H>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvFl/mrvIO.h"
#include "mrvNetwork/mrvTCP.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvPDF/mrvPDFCreator.h"

#include "mrViewer.h"

jmp_buf env;

namespace
{
    const char* kModule = "pdf";

    const unsigned kTitleSize = 16;
} // namespace

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
            margin.x = 5;
            margin.y = 5;

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
            fl_font(FL_HELVETICA, 18);
            int tw, th;
            fl_measure(page_title, tw, th);
            fl_draw(page_title, (width - tw) / 2, th);

            fl_line_style(FL_SOLID, 1);
            fl_rect(20, th + 10, width - 40, height - 40);

            P.x = 25;
            P.y = th + 15;

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

        void Creator::create_thumbnail(
            size_t W, size_t H, const unsigned char* buffer)
        {
            Fl_Surface_Device::push_current(&pdf);

            Fl_RGB_Image image(buffer, W, H);

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

            image.scale(W, H);
            image.draw(P.x + Xoffset, P.y);

            Fl_Surface_Device::pop_current();
        }

        void Creator::print_time(Fl_Font font)
        {
            Fl_Surface_Device::push_current(&pdf);

            fl_color(FL_BLACK);
            fl_font(font, 11);

            std::string buf =
                tl::string::Format(_("Frame {0} - TC: {1} - S: {2}"))
                    .arg(time.to_frames())
                    .arg(time.to_timecode())
                    .arg(time.to_seconds());

            int tw, th;
            fl_measure(buf.c_str(), tw, th);
            fl_draw(buf.c_str(), P.x + kThumbnailWidth + margin.x, P.y + 11);

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

        void Creator::wait()
        {
            bool found = false;

            auto uiView = ui->uiView;
            auto player = uiView->getTimelinePlayer();

            auto cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    player->player()->observeCacheInfo(),
                    [this, &found](const timeline::PlayerCacheInfo& value)
                    {
                        for (const auto& t : value.videoFrames)
                        {
                            if (time >= t.start_time() &&
                                time <= t.end_time_exclusive())
                            {
                                found = true;
                                break;
                            }
                        }
                    });

            while (!found)
            {
                Fl::check();
            }
        }

        bool Creator::create()
        {
            char* err_message;
            int err = pdf.begin_document(
                file.c_str(), Fl_Paged_Device::A4, Fl_Paged_Device::PORTRAIT,
                &err_message);
            if (err)
            {
                LOG_ERROR(err_message);
                return false;
            }

            addPage();

            auto view = ui->uiView;
            auto player = view->getTimelinePlayer();

            // Store presentation mode
            bool presentation = view->getPresentationMode();

            // Turn off hud so it does not get captured by glReadPixels.
            bool hud = view->getHudActive();

            // Turn off audio, so we don't play sound.
            auto mute = player->isMuted();
            player->setMute(true);
            player->start();

            // Set presentation mode
            view->setPresentationMode(true);
            view->redraw();

            // flush is needed
            Fl::flush();
            view->flush();
            Fl::check();

            auto viewportSize = view->getViewportSize();
            auto renderSize = view->getRenderSize();

            int X = 0, Y = 0;
            if (viewportSize.w >= renderSize.w &&
                viewportSize.h >= renderSize.h)
            {
                view->setFrameView(false);
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
                LOG_WARNING(_("Image too big.  "
                              "Will save the viewport size."));
                renderSize.w = viewportSize.w;
                renderSize.h = viewportSize.h;
                view->frameView();
            }

            view->make_current();
            gl::initGLAD();

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

            // Set some defaults to avoid drawing HUD, icon and sound.
            view->setHudActive(false);
            view->setActionMode(ActionMode::kScrub);
            player->stop();

            TimelineClass* c = ui->uiTimeWindow;

            const GLenum format = GL_RGB;
            const GLenum type = GL_UNSIGNED_BYTE;

            GLubyte* buffer = new GLubyte[renderSize.w * renderSize.h * 3];

            bool exit = false;
            for (const auto& annotation : annotations)
            {
                if (P.y > height - 20 - thumbnailHeight)
                {
                    pdf.end_page();

                    addPage();
                }

                time = annotation->time;
                player->seek(time);

                // Wait a while until so viewport updates.
                wait();

                view->make_current();
                view->redraw();
                view->flush();
                Fl::flush();

                GLenum imageBuffer = GL_FRONT;

                // @note: Wayland does not work like Windows, macOS or
                //        X11.  The compositor does not immediately
                //        swap buffers when calling view->flush().
                if (desktop::Wayland())
                {
                    imageBuffer = GL_BACK;
                }

                glReadBuffer(imageBuffer);

                glReadPixels(
                    X, Y, renderSize.w, renderSize.h, format, type, buffer);

                flip_image_y(buffer, renderSize.w, renderSize.h, 3);

                create_thumbnail(renderSize.w, renderSize.h, buffer);

                // print time
                print_time(FL_HELVETICA);

                for (const auto& shape : annotation->shapes)
                {
                    auto note = dynamic_cast< draw::NoteShape* >(shape.get());
                    if (!note)
                        continue;

                    print_note(FL_HELVETICA, note->text);
                }

                P.y += thumbnailHeight + image_margin;
            }

            delete[] buffer;

            pdf.end_page();

            pdf.end_job();

            view->setPresentationMode(presentation);
            wait();

            view->setHudActive(hud);

            view->frameView();

            player->setMute(mute);

            tcp->unlock();

            return true;
        }

    } // namespace pdf
} // namespace mrv
