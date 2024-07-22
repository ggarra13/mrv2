// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Image.h>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Exception.h>

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvComfyUIListener.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "comfy";
}

namespace mrv
{
    using namespace tl;

    class Connection : public Poco::Net::TCPServerConnection
    {
    public:
        Connection(const Poco::Net::StreamSocket& socket) :
            TCPServerConnection(socket)
        {
        }

        void run() override
        {
            bool isOpen = true;
            Poco::Timespan timeOut(10, 0); // 10 seconds
            while (isOpen)
            {
                if (socket().poll(timeOut, Poco::Net::Socket::SELECT_READ) ==
                    false)
                {
                    // Timed out...
                }
                else
                {
                    size_t nBytes = 0;
                    image::Size size;

                    try
                    {
                        nBytes = socket().receiveBytes(&size, sizeof(size));
                        
                        size.w = ntohl(size.w);
                        size.h = ntohl(size.h);

                        static_assert(sizeof(uint32_t) == sizeof(float),
                                      "Unsupported architecture");

                        uint32_t tmp;
                        tmp = ntohl(*((uint32_t*)&size.pixelAspectRatio));
                        size.pixelAspectRatio = *((float*)&tmp);

                        if (!size.isValid())
                            nBytes = 0;
                    }
                    catch (Poco::Exception& e)
                    {
                        // Network errors.
                        isOpen = false;
                    }

                    if (nBytes <= 0)
                    {
                        isOpen = false;
                    }
                    else
                    {

                        image::PixelType pixelType;
                        nBytes = socket().receiveBytes(
                            &pixelType, sizeof(pixelType));

                        static_assert(
                            sizeof(uint32_t) == sizeof(image::PixelType),
                            "Unsupported architecture for pixel type");
                        pixelType = static_cast<image::PixelType>(
                            ntohl(static_cast<uint32_t>(pixelType)));

                        if (nBytes <= 0)
                        {
                            isOpen = false;
                        }
                        else
                        {
                            static uint64_t counter = 0;
                            auto info = image::Info(size, pixelType);
                            info.layout.mirror.y = true;
                            
                            auto image = image::Image::create(info);

                            size_t sum = 0;
                            const size_t total = image->getDataByteCount();
                            while (sum < total)
                            {
                                nBytes = socket().receiveBytes(
                                    image->getData() + sum, total - sum);
                                sum += nBytes;
                                if (nBytes <= 0)
                                    break;
                            }
                            ++counter;
                            std::cerr << "got image " << counter << std::endl;
                            //App::ui->uiView->showImage(image);
                            break;
                        }
                    }
                }
            }
        }
    };

    ComfyUIListener::ComfyUIListener(uint16_t port) :
        server(
            new Poco::Net::TCPServerConnectionFactoryImpl<Connection>(), port)
    {
        try
        {
            server.start();
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR( e.displayText() );
        }
    }

    ComfyUIListener::~ComfyUIListener()
    {
        try
        {
            server.stop();
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR( e.displayText() );
        }
    }

} // namespace mrv
