// SPDX-License-Identifier: BSD-3-Clause

#include <tlIO/Cache.h>
#include <tlIO/FFmpeg.h>
#include <tlIO/FFmpegReadPrivate.h>

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

    AVPixelFormat pixelFormat(image::PixelType type, int referenceBits)
    {
        using P = image::PixelType;
        switch (type)
        {
        case P::RGB_U8: return AV_PIX_FMT_RGB24;
        case P::RGB_U16: return AV_PIX_FMT_RGB48LE;
        case P::RGBA_U8: return AV_PIX_FMT_RGBA;
        case P::RGBA_U16: return AV_PIX_FMT_RGBA64LE;
        case P::YUV_420P_U8: return AV_PIX_FMT_YUV420P;
        case P::YUV_422P_U8: return AV_PIX_FMT_YUV422P;
        case P::YUV_444P_U8: return AV_PIX_FMT_YUV444P;
        case P::YUV_420P_U10: return AV_PIX_FMT_YUV420P10LE;
        case P::YUV_422P_U10: return AV_PIX_FMT_YUV422P10LE;
        case P::YUV_444P_U10: return AV_PIX_FMT_YUV444P10LE;
        case P::YUV_420P_U12: return AV_PIX_FMT_YUV420P12LE;
        case P::YUV_422P_U12: return AV_PIX_FMT_YUV422P12LE;
        case P::YUV_444P_U12: return AV_PIX_FMT_YUV444P12LE;
        case P::YUV_420P_U16: return AV_PIX_FMT_YUV420P16LE;
        case P::YUV_422P_U16: return AV_PIX_FMT_YUV422P16LE;
        case P::YUV_444P_U16: return AV_PIX_FMT_YUV444P16LE;
        case P::YUV_420SP_U8: return AV_PIX_FMT_NV12;
        case P::YUV_422SP_U8: return AV_PIX_FMT_NV16;
        case P::YUV_444SP_U8: return AV_PIX_FMT_NV24;
        case P::YUV_420SP_U16: return referenceBits > 10 ? AV_PIX_FMT_P016LE : AV_PIX_FMT_P010LE;
        case P::YUV_422SP_U16: return referenceBits > 10 ? AV_PIX_FMT_P216LE : AV_PIX_FMT_P210LE;
        case P::YUV_444SP_U16: return referenceBits > 10 ? AV_PIX_FMT_P416LE : AV_PIX_FMT_P410LE;
        default: throw std::runtime_error("Unsupported test pixel format");
        }
    }

    // Upstream renders native semi-planar hardware output. Convert only in the
    // test so its samples can be compared with the planar software reference.
    std::shared_ptr<image::Image> normalize(const image::Image& reference, const image::Image& input)
    {
        const int bits = image::getBitDepth(reference.getPixelType());
        std::shared_ptr<AVFrame> frame(av_frame_alloc(), [](AVFrame* f) { av_frame_free(&f); });
        require(bool(frame), "Cannot allocate comparison frame");
        frame->format = pixelFormat(reference.getPixelType(), bits);
        frame->width = input.getWidth();
        frame->height = input.getHeight();
        require(av_frame_get_buffer(frame.get(), 32) >= 0, "Cannot allocate comparison planes");
        std::unique_ptr<SwsContext, decltype(&sws_freeContext)> context(
            sws_getContext(frame->width, frame->height, pixelFormat(input.getPixelType(), bits),
                           frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
                           SWS_POINT, nullptr, nullptr, nullptr), sws_freeContext);
        require(bool(context), "Cannot create comparison scaler");
        const uint8_t* planes[3] = {input.getPlaneData(0), input.getPlaneData(1), input.getPlaneData(2)};
        int strides[3] = {input.getLineSize(0), input.getLineSize(1), input.getLineSize(2)};
        require(sws_scale(context.get(), planes, strides, 0, frame->height,
                          frame->data, frame->linesize) == frame->height, "Cannot convert comparison frame");
        const uint8_t* output[3] = {frame->data[0], frame->data[1], frame->data[2]};
        return image::Image::create(reference.getInfo(), frame, output, frame->linesize);
    }

    double compare(const image::Image& a, const image::Image& input)
    {
        const auto normalized = a.getPixelType() != input.getPixelType() ? normalize(a, input) : nullptr;
        const auto& b = normalized ? *normalized : input;
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
        require(!ffmpeg::getReadOptions({{"FFmpeg/HardwareDecode", "0"}}).hwAccel,
                "Legacy software option ignored");
        require(ffmpeg::getReadOptions({{"FFmpeg/HardwareDecode", "1"}}).hwAccel,
                "Legacy hardware option ignored");
        require(!ffmpeg::getReadOptions({{"FFmpeg/HardwareDecode", "1"}, {"FFmpeg/HWAccel", "0"}}).hwAccel,
                "Canonical hardware option did not take precedence");
        require(ffmpeg::getReadOptions({{"FFmpeg/HWDriver", "videotoolbox"}}).hwDriver == "videotoolbox",
                "Hardware driver option ignored");
        const file::Path path(argv[1]);
        const std::string expected(argv[2]);
        io::Options options = {
            {"FFmpeg/HWAccel", "0"},
            {"FFmpeg/YUVToRGBConversion", argv[3]},
            {"FFmpeg/ThreadCount", argv[5]},
            {"FFmpeg/VideoBufferSize", "1"}};
        std::vector<uint8_t> bytes;
        std::vector<file::MemoryRead> memory;
        if (std::string(argv[4]) == "1")
        {
            std::ifstream stream(argv[1], std::ios::binary);
            bytes.assign(std::istreambuf_iterator<char>(stream), {});
            memory.emplace_back(std::shared_ptr<void>(), bytes.data(), bytes.size());
        }
        auto cache = io::Cache::create();
        cache->setMax(0);
        auto software = ffmpeg::VideoRead::create(path, memory, options, {});
        software->setCache(cache);
        const auto info = wait(software->getInfo());
        require(!info.video.empty(), "No software video info");
        options["FFmpeg/HWAccel"] = "1";
        auto automatic = ffmpeg::VideoRead::create(path, memory, options, {});
        automatic->setCache(cache);
        const auto autoInfo = wait(automatic->getInfo());
        require(autoInfo.video.size() == info.video.size() && !autoInfo.video.empty(), "No automatic video info");
        require(autoInfo.video[0].size == info.video[0].size, "Hardware changed video dimensions");
        require(autoInfo.video[0].videoLevels == info.video[0].videoLevels &&
                autoInfo.video[0].yuvCoefficients == info.video[0].yuvCoefficients,
                "Hardware changed color interpretation");
        require(info.videoTime && autoInfo.videoTime == info.videoTime, "Hardware changed time range");

        std::vector<std::pair<std::shared_ptr<image::Image>, std::shared_ptr<image::Image>>> held;
        std::string backend;
        double worst = 100;
        // Sequential reads, forward/backward seeks, and the final drained frame.
        for (int frame : {0, 1, 2, 9, 4, 11, 10, 9, 8, 7, 0})
        {
            const auto time = info.videoTime->start_time() +
                otime::RationalTime(frame, info.videoTime->duration().rate());
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
