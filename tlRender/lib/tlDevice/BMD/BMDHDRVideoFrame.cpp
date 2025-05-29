// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputPrivate.h>

namespace tl
{
    namespace bmd
    {
        /*HRESULT DLHDRVideoFrame::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            IID iunknown = IID_IUnknown;
            if (ppv == nullptr)
                return E_INVALIDARG;
            if (memcmp(&iid, &iunknown, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrame*>(this);
            else if (memcmp(&iid, &IID_IDeckLinkVideoFrame, sizeof(REFIID)) ==
        0) *ppv = static_cast<IDeckLinkVideoFrame*>(this); else if (memcmp(&iid,
        &IID_IDeckLinkVideoFrameMetadataExtensions, sizeof(REFIID)) == 0) *ppv =
        static_cast<IDeckLinkVideoFrameMetadataExtensions*>(this); else
            {
                *ppv = nullptr;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        ULONG DLHDRVideoFrame::AddRef(void)
        {
            return ++_refCount;
        }

        ULONG DLHDRVideoFrame::Release(void)
        {
            ULONG newRefValue = --_refCount;
            if (newRefValue == 0)
                delete this;
            return newRefValue;
        }

        HRESULT DLHDRVideoFrame::GetInt(BMDDeckLinkFrameMetadataID metadataID,
        int64_t* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRElectroOpticalTransferFunc:
                *value = _hdrData.eotf;
                break;
            case bmdDeckLinkFrameMetadataColorspace:
                *value = bmdColorspaceRec2020;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFloat(BMDDeckLinkFrameMetadataID metadataID,
        double* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedX:
                *value = _hdrData.redPrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedY:
                *value = _hdrData.redPrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenX:
                *value = _hdrData.greenPrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenY:
                *value = _hdrData.greenPrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueX:
                *value = _hdrData.bluePrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueY:
                *value = _hdrData.bluePrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointX:
                *value = _hdrData.whitePrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointY:
                *value = _hdrData.whitePrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRMaxDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.getMax();
                break;
            case bmdDeckLinkFrameMetadataHDRMinDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.getMin();
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumContentLightLevel:
                *value = _hdrData.maxCLL;
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumFrameAverageLightLevel:
                *value = _hdrData.maxFALL;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFlag(BMDDeckLinkFrameMetadataID, BOOL*
        value)
        {
            *value = false;
            return E_INVALIDARG;
        }

        HRESULT DLHDRVideoFrame::GetString(BMDDeckLinkFrameMetadataID, BSTR*
        value)
        {
            *value = nullptr;
            return E_INVALIDARG;
        }

        HRESULT	DLHDRVideoFrame::GetBytes(BMDDeckLinkFrameMetadataID metadataID,
        void* buffer, uint32_t* bufferSize)
        {
            *bufferSize = 0;
            return E_INVALIDARG;
        }*/
    }
} // namespace tl
