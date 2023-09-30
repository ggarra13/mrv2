
#pragma once

#include <tlCore/Util.h>

#include <Poco/Net/ServerSocket.h>

namespace mrv
{
    class App;

    const int kPORT_NUMBER = 55120;

    class ImageSender
    {
    public:
        ImageSender(uint16_t port = kPORT_NUMBER);

        bool isRunning();
        void sendImage(const std::string& imageData);

    private:
        Poco::Net::SocketAddress address;
        Poco::Net::StreamSocket socket;
    };

    class ImageListener
    {
    public:
        ImageListener(App* app, uint16_t port = kPORT_NUMBER);
        ~ImageListener();

        void run();
        void stop();

    private:
        App* app_ = nullptr;
        Poco::Net::ServerSocket serverSocket;
        std::thread* acceptThread = nullptr;
        bool running = false;
    };
} // namespace mrv
