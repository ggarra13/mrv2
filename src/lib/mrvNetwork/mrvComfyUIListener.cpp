// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvNetwork/mrvComfyUIListener.h"

#include "mrvApp/mrvApp.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvIO.h"

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Exception.h>

#include <tlCore/Image.h>

namespace
{
    const char* kModule = "comfy";
}

namespace mrv
{
    namespace
    {
        struct FileData
        {
            std::string fileName;
        };

        void open_data_file_cb(FileData* d)
        {

            App* app = App::app;
            auto model = app->filesModel();

            if (model->observeFiles()->getSize() < 1)
            {
                App::app->open(d->fileName);
                delete d;
                return;
            }
            
            auto item = model->observeA()->get();
            auto path = item->path.get();

            if (path != d->fileName)
            {
                App::app->open(d->fileName);
            }
            
            refresh_file_cache_cb(nullptr, nullptr);
            delete d;

            panel::refreshThumbnails();
        }
    } // namespace
    
} // namespace

namespace mrv
{
    using namespace tl;

    class ComfyConnection : public Poco::Net::TCPServerConnection
    {
    public:
        ComfyConnection(const Poco::Net::StreamSocket& socket) :
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
                        Fl::add_timeout(
                            0.0, (Fl_Timeout_Handler)open_data_file_cb, data);
                    }
                }
            }
        }
    };

    ComfyUIListener::ComfyUIListener(uint16_t port) :
        server(
            new Poco::Net::TCPServerConnectionFactoryImpl<ComfyConnection>(), port)
    {
        try
        {
            server.start();
            LOG_STATUS(_("ComfyUI listening on port ") << port);
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
