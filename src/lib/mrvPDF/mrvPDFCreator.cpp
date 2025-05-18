
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

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvImage.h"
#include "mrvCore/mrvString.h"
#include "mrvCore/mrvWait.h"

#ifdef OPENGL_BACKEND
#    include <tlGL/Init.h>
#endif

#ifdef VULKAN_BACKEND
#    include <tlTimelineUIVk/ThumbnailSystem.h>
#endif

#include <FL/fl_draw.H>


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
            const Fl_PDF_File_Surface::Page_Format& pageFormat,
            const std::vector< std::shared_ptr< draw::Annotation >>& ann,
            const ViewerUI* ui) :
            file(fileName),
            page_format(pageFormat),
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

            auto view = ui->uiView;
            auto player = view->getTimelinePlayer();

            auto cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    player->player()->observeCacheInfo(),
                    [this, &found](const timeline::PlayerCacheInfo& value)
                    {
                        for (const auto& t : value.videoFrames)
                        {
                            if (this->time >= t.start_time() &&
                                this->time <= t.end_time_exclusive())
                            {
                                found = true;
                                break;
                            }
                        }
                    });

            while (!found)
            {
                // flush is needed
                Fl::check();
            }

            wait::milliseconds(1000);
        }

        bool Creator::create()
        {
            char* err_message;
            int err = pdf.begin_document(
                file.c_str(), page_format, Fl_Paged_Device::PORTRAIT,
                &err_message);
            if (err)
            {
                LOG_ERROR(err_message);
                return false;
            }

            pdf.printable_rect(&width, &height);

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
            float pixels_unit = view->pixels_per_unit();
            auto renderSize = view->getRenderSize();

            int X = 0, Y = 0;
#ifdef __APPLE__
            viewportSize.w /= pixels_unit;
            viewportSize.h /= pixels_unit;
#endif
            if (viewportSize.w >= renderSize.w &&
                viewportSize.h >= renderSize.h)
            {
                view->setFrameView(false);
#ifdef __APPLE__
                view->setViewZoom(pixels_unit);
#else
                view->setViewZoom(1.0);
#endif
                view->centerView();
                view->redraw();
                // flush is needed
                Fl::flush();
            }
            else
            {
                LOG_WARNING(_("Image too big.  "
                              "Will save the viewport size."));

                float aspectImage =
                    static_cast<float>(renderSize.w) / renderSize.h;
                float aspectViewport =
                    static_cast<float>(viewportSize.w) / viewportSize.h;

                if (aspectImage > aspectViewport)
                {
                    // Fit to width
                    renderSize.w = viewportSize.w;
                    renderSize.h = viewportSize.w / aspectImage;
                }
                else
                {
                    // Fit to height
                    renderSize.h = viewportSize.h;
                    renderSize.w = viewportSize.h * aspectImage;
                }
                view->frameView();
            }

            X = (viewportSize.w - renderSize.w) / 2;
            Y = (viewportSize.h - renderSize.h) / 2;

            std::string msg = tl::string::Format(_("Viewport Size: {0} - "
                                                   "X={1}, Y={2}"))
                                  .arg(viewportSize)
                                  .arg(X)
                                  .arg(Y);
            LOG_INFO(msg);

#ifdef OPENGL_BACKEND
            view->make_current();
            gl::initGLAD();
#endif
            
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

#ifdef OPENGL_BACKEND
                view->make_current();
#endif
                
                view->redraw();
                view->flush();
                Fl::flush();

#ifdef __APPLE__
                Fl_RGB_Image* tmp =
                    fl_capture_window(view, X, Y, renderSize.w, renderSize.h);
                Fl_Image* rgb = tmp->copy(renderSize.w, renderSize.h);
                tmp->alloc_array = 1;
                delete tmp;

                // Access the first pointer in the data array
                const char* const* data = rgb->data();
                const size_t data_size = rgb->w() * rgb->h() * rgb->d();
                memcpy(buffer, data[0], data_size);
                delete rgb;
#else

#ifdef OPENGL_BACKEND
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

                flipImageInY(buffer, renderSize.w, renderSize.h, 3);
#endif
                
#endif

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
