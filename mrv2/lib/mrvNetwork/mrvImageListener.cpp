// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <thread>

#include <FL/Fl.H>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Exception.h>

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvImageListener.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "listen";
}

namespace mrv
{
    namespace
    {
        struct FileData
        {
            std::string fileName;
            App* app;
        };

        void open_data_file_cb(FileData* d)
        {
            d->app->open(d->fileName);
            delete d;
        }
    } // namespace

    ImageSender::ImageSender(uint16_t port) :
        address("127.0.0.1", port)
    {
    }

    bool ImageSender::isRunning()
    {
        try
        {
            socket.connect(address);
            socket.close();
            return true;
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR(e.displayText());
            // Connection failed; no sender is running.
            return false;
        }
    }

    void ImageSender::sendImage(const std::string& fileName)
    {
        try
        {
            socket.connect(address);
            Poco::Net::SocketStream stream(socket);
            stream << fileName;
            stream.flush();
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR( e.displayText() );
        }
    }

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
            std::string fileName;
            while (isOpen)
            {
                if (socket().poll(timeOut, Poco::Net::Socket::SELECT_READ) ==
                    false)
                {
                    // Timed out...
                }
                else
                {
                    int nBytes = -1;
                    fileName.resize(4096); // should be big enough

                    try
                    {
                        nBytes = socket().receiveBytes(
                            fileName.data(), fileName.size());
                    }
                    catch (const Poco::Exception& e)
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
                        fileName.resize(nBytes);
                        FileData* data = new FileData;
                        data->fileName = fileName;
                        data->app = ViewerUI::app;
                        Fl::add_timeout(
                            0.0, (Fl_Timeout_Handler)open_data_file_cb, data);
                    }
                }
            }
        }
    };

    ImageListener::ImageListener(App* app, uint16_t port) :
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

    ImageListener::~ImageListener()
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
