
#include "mrvNetwork/mrvTcpDataChannelPump.h"

namespace mrv
{
        std::shared_ptr<TcpDataChannelPump> TcpDataChannelPump::create(
            Poco::Net::StreamSocket socket,
            std::shared_ptr<rtc::DataChannel> channel)
        {
            auto out = std::make_shared<TcpDataChannelPump>(socket, channel);
            out->init();
            return out;
        }
}
