

#include "mrvNetwork/mrvTCP.h"

namespace mrv
{
    TCP::TCP() {}

    TCP::~TCP()
    {
        nng_close(sock);
    }

    std::string TCP::popMessage()
    {
        if (m_messages.empty())
            return "";
        auto msg = m_messages.front();
        m_messages.pop_front();
        return msg;
    }
} // namespace mrv
