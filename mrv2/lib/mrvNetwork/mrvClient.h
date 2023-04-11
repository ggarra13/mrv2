
#pragma once

#include <string>

namespace mrv
{
    class Client
    {
    public:
        Client( const std::string& ip = "localhost", unsigned port = 5800 );
    };
}
