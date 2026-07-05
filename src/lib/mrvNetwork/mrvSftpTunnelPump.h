// mrvSftpTunnelPump.h
#pragma once
#include <rtc/rtc.hpp>
#include <Poco/Net/StreamSocket.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

namespace mrv
{
    // Pumps bytes between a local TCP socket and a WebRTC DataChannel.
    // onFinished fires exactly once, when either side closes — the
    // owner uses it to remove this pump from whatever container holds it.
    class TcpDataChannelPump
        : public std::enable_shared_from_this<TcpDataChannelPump>
    {
    public:
        TcpDataChannelPump(Poco::Net::StreamSocket socket,
                            std::shared_ptr<rtc::DataChannel> channel)
            : socket_(std::move(socket)),
            channel_(std::move(channel))
        {
            const size_t kLowThreshold = 64 * 1024;
            highThreshold_ = 1024 * 1024;

            channel_->setBufferedAmountLowThreshold(kLowThreshold);

            
            channel_->onBufferedAmountLow([weak = weak_from_this()]()
                {
                    if (auto self = weak.lock())
                    {
                        std::lock_guard<std::mutex> lk(self->mutex_);
                        self->paused_ = false;
                        self->cv_.notify_one();
                    }
                });

            
            channel_->onMessage([weak = weak_from_this()](rtc::message_variant msg)
                {
                    auto self = weak.lock();
                    if (!self)
                        return;
                    if (!std::holds_alternative<rtc::binary>(msg))
                        return;
                    auto& data = std::get<rtc::binary>(msg);
                    self->writeAllToSocket(
                        reinterpret_cast<const char*>(data.data()), data.size());
                });

            channel_->onClosed([weak = weak_from_this()]()
                {
                    if (auto self = weak.lock())
                        self->finish();
                    // If weak.lock() returns null, the pump is already destroyed
                    // and there's nothing to do — no dangling pointer access.
                });
        }

        void setOnFinished(std::function<void()> cb) {
            onFinished_ = std::move(cb);
        }

        void start()
        {
            reader_ = std::thread([self = shared_from_this()]()
                                   { self->readLoop(); });
        }

        // Safe to call multiple times (e.g. explicit stop() racing
        // with the channel's own onClosed firing).
        void stop() { finish(); }

        ~TcpDataChannelPump()
        {
            running_ = false;
            {
                std::lock_guard<std::mutex> lk(mutex_);
                paused_ = false;
            }
            cv_.notify_all();
            
            if (reader_.joinable())
            {
                if (reader_.get_id() == std::this_thread::get_id())
                {
                    // We are being destroyed from within our own reader
                    // thread (the last shared_ptr reference was released as
                    // readLoop() returned). Joining here would self-deadlock
                    // -- join() would throw, and even catching that, a
                    // still-joinable std::thread's own destructor calls
                    // std::terminate() unconditionally. Detach instead; the
                    // thread is finishing its own execution right now, so
                    // there's nothing left to wait for.
                    reader_.detach();
                }
                else
                {
                    reader_.join();
                }
            }
        }

    private:
        void readLoop()
        {
            std::vector<char> buf(64 * 1024);
            while (running_)
            {
                {
                    std::unique_lock<std::mutex> lk(mutex_);
                    cv_.wait(lk, [this]() { return !paused_ || !running_; });
                }
                if (!running_)
                    break;

                int n = 0;
                try
                {
                    n = socket_.receiveBytes(buf.data(),
                                              static_cast<int>(buf.size()));
                }
                catch (const Poco::Exception&)
                {
                    n = 0;
                }

                if (n <= 0)
                {
                    if (channel_->isOpen())
                        channel_->close();
                    break;
                }

                try
                {
                    channel_->send(
                        reinterpret_cast<const std::byte*>(buf.data()),
                        static_cast<size_t>(n));
                }
                catch (const std::exception&)
                {
                    break;
                }

                if (channel_->bufferedAmount() > highThreshold_)
                {
                    std::lock_guard<std::mutex> lk(mutex_);
                    paused_ = true;
                }
            }
            finish();
        }

        void writeAllToSocket(const char* data, size_t len)
        {
            size_t off = 0;
            while (off < len && running_)
            {
                int n = 0;
                try
                {
                    n = socket_.sendBytes(data + off,
                                          static_cast<int>(len - off));
                }
                catch (const Poco::Exception&)
                {
                    n = -1;
                }
                if (n <= 0)
                {
                    running_ = false;
                    break;
                }
                off += static_cast<size_t>(n);
            }
        }

        // Runs at most once; safe to call from either the reader
        // thread or the DataChannel's callback thread.
        void finish()
        {
            bool expected = false;
            if (!finished_.compare_exchange_strong(expected, true))
                return; // already finished

            try
            {
                socket_.shutdown();
                socket_.close();
            }
            catch (const Poco::Exception&) {}

            running_ = false;
            {
                std::lock_guard<std::mutex> lk(mutex_);
                paused_ = false;
            }
            cv_.notify_all();

            if (onFinished_)
                onFinished_();
        }

        Poco::Net::StreamSocket socket_;
        std::shared_ptr<rtc::DataChannel> channel_;
        std::function<void()> onFinished_;

        std::thread reader_;
        std::atomic<bool> running_{true};
        std::atomic<bool> finished_{false};

        std::mutex mutex_;
        std::condition_variable cv_;
        bool paused_ = false;
        size_t highThreshold_ = 0;
    };

} // namespace mrv
