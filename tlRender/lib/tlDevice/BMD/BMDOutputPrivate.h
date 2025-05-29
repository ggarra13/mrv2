// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDOutputDevice.h>

#ifndef NOMINMAX
#    define NOMINMAX
#endif // NOMINMAX
#include "platform.h"

#if defined(__APPLE__)
typedef int64_t LONGLONG;
#elif defined(__linux__)
typedef bool BOOL;
typedef int64_t LONGLONG;
#endif // __APPLE__

namespace tl
{
    namespace bmd
    {
        class DLIteratorWrapper
        {
        public:
            ~DLIteratorWrapper()
            {
                if (p)
                    p->Release();
            }

            IDeckLinkIterator* operator->() const { return p; }

            IDeckLinkIterator* p = nullptr;
        };

        class DLDisplayModeIteratorWrapper
        {
        public:
            ~DLDisplayModeIteratorWrapper()
            {
                if (p)
                    p->Release();
            }

            IDeckLinkDisplayModeIterator* operator->() const { return p; }

            IDeckLinkDisplayModeIterator* p = nullptr;
        };

        class DLDisplayModeWrapper
        {
        public:
            ~DLDisplayModeWrapper()
            {
                if (p)
                    p->Release();
            }

            IDeckLinkDisplayMode* operator->() const { return p; }

            IDeckLinkDisplayMode* p = nullptr;
        };

        class DLVideoFrameWrapper
        {
        public:
            ~DLVideoFrameWrapper()
            {
                if (p)
                    p->Release();
            }

            IDeckLinkMutableVideoFrame* operator->() const { return p; }

            IDeckLinkMutableVideoFrame* p = nullptr;
        };

        class DLFrameConversionWrapper
        {
        public:
            ~DLFrameConversionWrapper()
            {
                if (p)
                    p->Release();
            }

            IDeckLinkVideoConversion* operator->() { return p; }

            IDeckLinkVideoConversion* p = nullptr;
        };

        /*class DLHDRVideoFrame :
            public IDeckLinkVideoFrame,
            public IDeckLinkVideoFrameMetadataExtensions
        {
        public:
            DLHDRVideoFrame(std::shared_ptr<IDeckLinkMutableVideoFrame>& frame,
        image::HDRData& hdrData) : _frame(frame), _hdrData(hdrData),
                _refCount(1)
            {}

            virtual ~DLHDRVideoFrame() {}

            HRESULT QueryInterface(REFIID iid, LPVOID* ppv) override;
            ULONG AddRef(void) override;
            ULONG Release(void) override;

            long GetWidth(void) override { return _frame->GetWidth(); }
            long GetHeight(void) override { return _frame->GetHeight(); }
            long GetRowBytes(void) override { return _frame->GetRowBytes(); }
            BMDPixelFormat GetPixelFormat(void) override { return
        _frame->GetPixelFormat(); } BMDFrameFlags GetFlags(void) override {
        return _frame->GetFlags() | bmdFrameContainsHDRMetadata; } HRESULT
        GetBytes(void** buffer) override { return _frame->GetBytes(buffer); }
            HRESULT GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode**
        timecode) override { return _frame->GetTimecode(format, timecode); }
            HRESULT GetAncillaryData(IDeckLinkVideoFrameAncillary** ancillary)
        override { return _frame->GetAncillaryData(ancillary); }

            HRESULT GetInt(BMDDeckLinkFrameMetadataID metadataID, int64_t*
        value) override; HRESULT GetFloat(BMDDeckLinkFrameMetadataID metadataID,
        double* value) override; HRESULT GetFlag(BMDDeckLinkFrameMetadataID
        metadataID, BOOL* value) override; HRESULT
        GetString(BMDDeckLinkFrameMetadataID metadataID, BSTR* value) override;
            HRESULT GetBytes(BMDDeckLinkFrameMetadataID metadataID, void*
        buffer, uint32_t* bufferSize) override;

            void UpdateHDRMetadata(const image::HDRData& metadata) { _hdrData =
        metadata; }

        private:
            std::shared_ptr<IDeckLinkMutableVideoFrame> _frame;
            image::HDRData _hdrData;
            std::atomic<ULONG> _refCount;
        };*/

        class DLOutputCallback : public IDeckLinkVideoOutputCallback,
                                 public IDeckLinkAudioOutputCallback
        {
        public:
            DLOutputCallback(
                IDeckLinkOutput*, const math::Size2i& size, PixelType pixelType,
                const otime::RationalTime& frameRate,
                const audio::Info& audioInfo);

            void setPlayback(timeline::Playback, const otime::RationalTime&);
            void setVideo(
                const std::shared_ptr<DLVideoFrameWrapper>&,
                const otime::RationalTime&);
            void setVolume(float);
            void setMute(bool);
            void setAudioOffset(double);
            void setAudioData(const std::vector<timeline::AudioData>&);

            HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(
                IDeckLinkVideoFrame*, BMDOutputFrameCompletionResult) override;
            HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

            HRESULT STDMETHODCALLTYPE RenderAudioSamples(BOOL preroll) override;

            HRESULT STDMETHODCALLTYPE
            QueryInterface(REFIID iid, LPVOID* ppv) override;
            ULONG STDMETHODCALLTYPE AddRef() override;
            ULONG STDMETHODCALLTYPE Release() override;

        private:
            TLRENDER_PRIVATE();
        };

        class DLWrapper
        {
        public:
            ~DLWrapper()
            {
                if (outputCallback)
                {
                    outputCallback->Release();
                }
                if (output)
                {
                    output->StopScheduledPlayback(0, nullptr, 0);
                    output->DisableVideoOutput();
                    output->DisableAudioOutput();
                    output->Release();
                }
                if (status)
                {
                    status->Release();
                }
                if (config)
                {
                    config->Release();
                }
                if (p)
                {
                    p->Release();
                }
            }

            IDeckLink* p = nullptr;
            IDeckLinkConfiguration* config = nullptr;
            IDeckLinkStatus* status = nullptr;
            IDeckLinkOutput* output = nullptr;
            DLOutputCallback* outputCallback = nullptr;
        };
    } // namespace bmd
} // namespace tl
