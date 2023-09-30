#include <iostream>
#include <thread>

#include <FL/Fl.H>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Exception.h>

#include "mrvNetwork/mrvImageListener.h"

#include "mrvApp/App.h"

namespace mrv
{
    namespace
    {
        struct ImageData
        {
            std::string fileName;
            App* app;
        };

        void open_file_cb(ImageData* d)
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
        catch (Poco::Exception&)
        {
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
        catch (Poco::Exception& e)
        {
        }
    }

    ImageListener::ImageListener(App* app, uint16_t port) :
        app_(app),
        serverSocket(port)
    {
        Poco::Timespan timeout(2, 0); // 2 Sec
        serverSocket.setReceiveTimeout(timeout);

        acceptThread = new std::thread([this] { run(); });
    }

    void ImageListener::stop()
    {
        running = false;
        if (acceptThread && acceptThread->joinable())
            acceptThread->join();
        serverSocket.close();
    }

    void ImageListener::run()
    {
        running = true;
        while (running)
        {
            try
            {
                Poco::Net::StreamSocket clientSocket =
                    serverSocket.acceptConnection();
                Poco::Net::SocketStream stream(clientSocket);

                // Read and process incoming image data here
                std::string fileName;
                stream >> fileName;

                if (!fileName.empty())
                {
                    // Process the image data as needed
                    ImageData* data = new ImageData;
                    data->app = app_;
                    data->fileName = fileName;
                    Fl::add_timeout(
                        0.0, (Fl_Timeout_Handler)open_file_cb, data);
                }
                // Close the client socket
                clientSocket.close();
            }
            catch (Poco::Exception&)
            {
            }
        }
    }

} // namespace mrv
