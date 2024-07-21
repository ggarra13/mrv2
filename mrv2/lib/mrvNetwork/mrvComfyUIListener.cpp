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

#include "mrvNetwork/mrvComfyUIListener.h"

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
                        std::cerr << "got image size=" << size << std::endl;

                        image::PixelType pixelType;
                        nBytes = socket().receiveBytes(
                            &pixelType, sizeof(pixelType));
                        pixelType = static_cast<image::PixelType>(
                            ntohl(static_cast<uint32_t>(pixelType)));

                        if (nBytes <= 0)
                        {
                            isOpen = false;
                        }
                        else
                        {

                            std::cerr << "got pixelType=" << pixelType
                                      << std::endl;
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
        server.start();
    }

    ComfyUIListener::~ComfyUIListener()
    {
        server.stop();
    }

} // namespace mrv
