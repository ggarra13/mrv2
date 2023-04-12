
#pragma once

#include <list>
#include <string>

#include <nng/nng.h>

namespace mrv
{
    class TCP
    {
    public:
        TCP();
        virtual ~TCP();

        virtual void sendMessage(const std::string& msg){};
        virtual void receiveMessage(){};
        inline bool hasMessages() const { return m_messages.empty(); }
        std::string popMessage();

    protected:
        nng_socket sock;
        std::list< std::string > m_messages;
    };
} // namespace mrv
