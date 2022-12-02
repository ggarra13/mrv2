
#include <string>
#include <algorithm>

#include <tlIO/IOSystem.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Util.h>

#include <tlGlad/gl.h>

#include "mrvCore/mrvUtil.h"

#include "mrViewer.h"

namespace mrv
{
    namespace
    {
        void _print( const std::string& text )
        {
            std::cout << text << std::endl;
        }
    }

    void save_movie( const std::string& file, ViewerUI* ui )
    {
        Viewport* view = ui->uiView;
        
        auto player = view->getTimelinePlayer();
        if (! player ) return;  // should never happen
        
        // Time range.
        auto timeRange = player->inOutRange();
        _print(string::Format("In/out range: {0}-{1}").
               arg(timeRange.start_time().value()).
               arg(timeRange.end_time_inclusive().value()));
        auto startTime = timeRange.start_time();
        auto   endTime = timeRange.end_time_inclusive();
        auto currentTime = startTime;
        _print(string::Format("In/out range: {0}-{1}").
               arg(startTime.value()).arg(endTime.value()));

        
        // Render information.
        const auto& info = player->ioInfo();
        if (info.video.empty())
        {
            throw std::runtime_error("No video information");
        }
        auto renderSize = info.video[0].size;
        _print(string::Format("Render size: {0}").arg(renderSize));

        // Create the writer.
        auto writerPlugin = ui->app->getContext()->getSystem<io::System>()->getPlugin(file::Path(file) );

        if (!writerPlugin )    
        {
            throw std::runtime_error(string::Format("{0}: Cannot open").arg(file));
        }

        
        io::Info ioInfo;
        imaging::Info outputInfo;
        outputInfo.size = renderSize;
        outputInfo.pixelType = info.video[0].pixelType;
        
        outputInfo = writerPlugin->getWriteInfo(outputInfo);
        if (imaging::PixelType::None == outputInfo.pixelType)
        {
            outputInfo.pixelType = imaging::PixelType::RGB_U8;
        }
        _print(string::Format("Output info: {0} {1}").
               arg(outputInfo.size).
               arg(outputInfo.pixelType));
        
        auto outputImage = imaging::Image::create(outputInfo);
        ioInfo.video.push_back(outputInfo);
        ioInfo.videoTime = timeRange;
        
        auto writer = writerPlugin->write(file::Path(file), ioInfo);
        if (!writer)
        {
            throw std::runtime_error(string::Format("{0}: Cannot open").arg(file));
        }



        
        bool running = true;
        while ( running )
        {
            player->seek( currentTime );
            Fl::check();
            
            const auto& buffer = view->getBuffer();
            gl::OffscreenBufferBinding binding(buffer);
            
            glPixelStorei(GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
            glPixelStorei(GL_PACK_SWAP_BYTES, outputInfo.layout.endian != memory::getEndian());
            const GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(string::Format("{0}: Cannot open").arg(file));
            }
            glReadPixels(
                0,
                0,
                outputInfo.size.w,
                outputInfo.size.h,
                format,
                type,
                outputImage->getData());

            std::cerr << "Processing " << currentTime << std::endl;
            
            writer->writeVideo(currentTime, outputImage);
            
            currentTime += otime::RationalTime(1, currentTime.rate());
            if (currentTime > endTime)
            {
                running = false;
            }
        }
    }

}
