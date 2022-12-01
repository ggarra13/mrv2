
#include <string>
#include <algorithm>

#include <tlIO/IOSystem.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include "mrvCore/mrvUtil.h"

#include "mrViewer.h"

namespace mrv
{

    void save_movie( const std::string& file, ViewerUI* ui )
    {
        Viewport* view = ui->uiView;
        
        auto player = view->getTimelinePlayer();
        if (! player ) return;  // should never happen
        
        auto timeRange = player->inOutRange();

        auto writerPlugin = ui->app->getContext()->getSystem<io::System>()->getPlugin(file::Path(file) );

        if (!writerPlugin )    
        {
            throw std::runtime_error(string::Format("{0}: Cannot open").arg(file));
        }
        
        const auto& info = player->ioInfo();
        if (info.video.empty())
        {
            throw std::runtime_error("No video information");
        }

        auto startTime = timeRange.start_time();
        auto   endTime = timeRange.end_time_inclusive();
        auto currentTime = startTime;
        
        imaging::Info outputInfo;
        outputInfo.size = ui->uiView->getRenderSize();
        outputInfo.pixelType = info.video[0].pixelType;

        bool running = true;
        while ( running )
        {
            player->seek( currentTime );
            std::cerr << "processing " << currentTime << " " << player
                      << std::endl;
            time::sleep( std::chrono::milliseconds(100) );
            if ( ! Fl::check() ) break;
            currentTime += otime::RationalTime(1, currentTime.rate());
            if (currentTime > timeRange.end_time_inclusive())
            {
                running = false;
            }
        }
    }

}
