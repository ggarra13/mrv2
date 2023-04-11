
#pragma once

#include <string>

namespace mrv
{
    class Server
    {
    public:
        Server( const std::string& host = "localhost", unsigned port = 5800 );
    };
}
