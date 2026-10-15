// SPDX-License-Identifier: BSD-3-Clause
// Interpose only in the failure-test executable, never in the application.

extern "C"
{
#include <libavutil/hwcontext.h>
}

#include <cstdlib>
#include <dlfcn.h>

extern "C" int av_hwdevice_ctx_create(
    AVBufferRef** context, AVHWDeviceType type, const char* device,
    AVDictionary* options, int flags)
{
    if (std::getenv("MRV2_TEST_FAIL_HWDEVICE"))
        return AVERROR_EXTERNAL;
    static const auto real = reinterpret_cast<decltype(&av_hwdevice_ctx_create)>(
        dlsym(RTLD_NEXT, "av_hwdevice_ctx_create"));
    return real ? real(context, type, device, options, flags) : AVERROR(ENOSYS);
}

extern "C" int av_hwframe_transfer_data(AVFrame* destination, const AVFrame* source, int flags)
{
    static int transfers = 0;
    const auto failAt = std::getenv("MRV2_TEST_FAIL_TRANSFER");
    if (failAt && ++transfers == std::atoi(failAt))
        return AVERROR_EXTERNAL;
    static const auto real = reinterpret_cast<decltype(&av_hwframe_transfer_data)>(
        dlsym(RTLD_NEXT, "av_hwframe_transfer_data"));
    return real ? real(destination, source, flags) : AVERROR(ENOSYS);
}
