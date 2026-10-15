// SPDX-License-Identifier: BSD-3-Clause

#include <tlIO/Cache.h>
#include <tlIO/FFmpeg.h>

#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace tl;

namespace
{
    void require(bool value, const std::string& message)
    {
        if (!value)
            throw std::runtime_error(message);
    }

    template<typename T> T wait(std::future<T> future)
    {
        require(future.wait_for(std::chrono::seconds(10)) == std::future_status::ready,
                "Timed out waiting for the decoder");
        return future.get();
    }

    double compare(const image::Image& a, const image::Image& b)
    {
        require(a.getSize() == b.getSize(), "Frame size differs");
        require(a.getPixelType() == b.getPixelType(), "Pixel type differs");
        const bool planar = a.getPixelType() >= image::PixelType::YUV_420P_U8 &&
            a.getPixelType() <= image::PixelType::YUV_444P_U16;
        require(a.getPlaneCount() == (planar ? 3 : 1), "Missing software planes");
        require(b.getPlaneCount() == (planar ? 3 : 1), "Missing hardware planes");
        const int bits = image::getBitDepth(a.getPixelType());
        const int bytes = bits > 8 ? 2 : 1;
        const int channels = planar ? 1 : image::getChannelCount(a.getPixelType());
        int subX = 0, subY = 0;
        if (planar)
        {
            const int type = (int(a.getPixelType()) - int(image::PixelType::YUV_420P_U8)) % 3;
            subX = type != 2;
            subY = type == 0;
        }
        double sum = 0;
        double channelError[4] = {};
        size_t samples = 0;
        for (int plane = 0; plane < (planar ? 3 : 1); ++plane)
        {
            const int w = (a.getWidth() + ((plane ? 1 << subX : 1) - 1)) >> (plane ? subX : 0);
            const int h = (a.getHeight() + ((plane ? 1 << subY : 1) - 1)) >> (plane ? subY : 0);
            const auto dataA = a.getPlaneData(plane);
            const auto dataB = b.getPlaneData(plane);
            require(dataA && dataB, "Null image plane");
            const int strideA = planar ? a.getLineSize(plane) : w * channels * bytes;
            const int strideB = planar ? b.getLineSize(plane) : w * channels * bytes;
            require(strideA >= w * channels * bytes && strideB >= w * channels * bytes,
                    "Invalid plane stride");
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w * channels; ++x)
                {
                    uint16_t va = dataA[y * strideA + x * bytes];
                    uint16_t vb = dataB[y * strideB + x * bytes];
                    if (bytes == 2)
                    {
                        std::memcpy(&va, dataA + y * strideA + x * bytes, 2);
                        std::memcpy(&vb, dataB + y * strideB + x * bytes, 2);
                    }
                    const double delta = (double(va) - vb) / ((1 << bits) - 1);
                    sum += delta * delta;
                    channelError[x % channels] += delta * delta;
                    ++samples;
                }
            }
        }
        const double psnr = sum ? -10 * std::log10(sum / samples) : 100;
        if (psnr < 30 && !planar)
        {
            std::cerr << "Channel PSNR:";
            for (int i = 0; i < channels; ++i)
                std::cerr << ' ' << (channelError[i] ?
                    -10 * std::log10(channelError[i] / (samples / channels)) : 100);
            std::cerr << '\n';
        }
        require(psnr >= 30, "Hardware/software pixels differ: PSNR=" + std::to_string(psnr));
        return psnr;
    }
}

int main(int argc, char** argv)
{
    try
    {
        require(argc == 6, "Usage: tlffmpeg-decode-test FILE BACKEND RGB MEMORY THREADS");
        const file::Path path(argv[1]);
        const std::string expected(argv[2]);
        io::Options options = {
            {"FFmpeg/HardwareDecode", "0"},
            {"FFmpeg/YUVToRGBConversion", argv[3]},
            {"FFmpeg/ThreadCount", argv[5]},
            {"FFmpeg/VideoBufferSize", "1"}};
        std::vector<uint8_t> bytes;
        std::vector<file::MemoryRead> memory;
        if (std::string(argv[4]) == "1")
        {
            std::ifstream stream(argv[1], std::ios::binary);
            bytes.assign(std::istreambuf_iterator<char>(stream), {});
            memory.emplace_back(bytes.data(), bytes.size());
        }
        auto cache = io::Cache::create();
        cache->setMax(0);
        auto software = ffmpeg::Read::create(path, memory, options, cache, {});
        const auto info = wait(software->getInfo());
        require(!info.video.empty(), "No software video info");
        options["FFmpeg/HardwareDecode"] = "1";
        auto automatic = ffmpeg::Read::create(path, memory, options, cache, {});
        const auto autoInfo = wait(automatic->getInfo());
        require(autoInfo.video == info.video, "Hardware changed video info");
        require(autoInfo.videoTime == info.videoTime, "Hardware changed time range");

        std::vector<std::pair<std::shared_ptr<image::Image>, std::shared_ptr<image::Image>>> held;
        std::string backend;
        double worst = 100;
        // Sequential reads, forward/backward seeks, and the final drained frame.
        for (int frame : {0, 1, 2, 9, 4, 11, 10, 9, 8, 7, 0})
        {
            const auto time = info.videoTime.start_time() +
                otime::RationalTime(frame, info.videoTime.duration().rate());
            const auto a = wait(software->readVideo(time));
            const auto b = wait(automatic->readVideo(time));
            require(a.image && b.image, "Missing decoded frame");
            require(a.time == time && b.time == time, "Wrong frame timestamp");
            const auto& tags = b.image->getTags();
            backend = tags.at("Video Decode Backend");
            require(a.image->getTags().at("Video Decode Backend") == "Software",
                    "Explicit software mode used hardware");
            require(expected == "auto" || expected == "fallback" || backend == expected,
                    "Unexpected backend: " + backend);
            require(a.image->getTags().at("otioClipTime") == tags.at("otioClipTime"),
                    "Hardware returned a different frame timestamp");
            const auto& hdrA = a.image->getHDR();
            const auto& hdrB = b.image->getHDR();
            require(bool(hdrA) == bool(hdrB), "Lost HDR metadata");
            if (hdrA && hdrB)
                require(hdrA->eotf == hdrB->eotf, "Changed HDR transfer function");
            try
            {
                worst = std::min(worst, compare(*a.image, *b.image));
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error(std::string(e.what()) +
                    ", frame=" + std::to_string(frame) + ", backend=" + backend +
                    ", format=" + tags.at("FFmpeg Pixel Format") +
                    ", software time=" + a.image->getTags().at("otioClipTime") +
                    ", hardware time=" + tags.at("otioClipTime"));
            }
            held.emplace_back(a.image, b.image);
        }
        if (expected == "fallback")
            require(backend == "Software", "Did not remain in software mode after failure");
        // Images must keep their planes alive after both decoders are destroyed.
        software.reset();
        automatic.reset();
        for (const auto& pair : held)
            compare(*pair.first, *pair.second);
        std::cout << path.get() << ": " << backend << ", RGB=" << argv[3]
                  << ", memory=" << argv[4] << ", threads=" << argv[5]
                  << ", minimum PSNR=" << worst << " dB\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
