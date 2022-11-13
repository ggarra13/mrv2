// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <mrvPlayApp/mrvDevicesModel.h>

#include <tlDevice/IDeviceSystem.h>

#include <tlCore/Context.h>

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "devicesmodel";
}

namespace mrv
{
    
    bool DevicesModelData::operator == (const DevicesModelData& other) const
    {
        return
            devices == other.devices &&
            deviceIndex == other.deviceIndex &&
            displayModes == other.displayModes &&
            displayModeIndex == other.displayModeIndex &&
            pixelTypes == other.pixelTypes &&
            pixelTypeIndex == other.pixelTypeIndex &&
            videoLevels == other.videoLevels &&
            hdrMode == other.hdrMode &&
            hdrData == other.hdrData;
    }

    struct DevicesModel::Private
    {
        std::vector<device::DeviceInfo> deviceInfo;
        int deviceIndex = 0;
        int displayModeIndex = 0;
        int pixelTypeIndex = 0;
        imaging::VideoLevels videoLevels = imaging::VideoLevels::LegalRange;
        device::HDRMode hdrMode = device::HDRMode::FromFile;
        imaging::HDRData hdrData;
        std::shared_ptr<observer::Value<DevicesModelData> > data;
        std::shared_ptr<observer::ListObserver<device::DeviceInfo> > deviceInfoObserver;
    };

    void DevicesModel::_init(const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        DBG;
        p.data = observer::Value<DevicesModelData>::create();

        DBG;

        _update();

        if (auto deviceSystem = context->getSystem<device::IDeviceSystem>())
        {
            p.deviceInfoObserver = observer::ListObserver<device::DeviceInfo>::create(
                deviceSystem->observeDeviceInfo(),
                [this](const std::vector<device::DeviceInfo>& value)
                    {
                        _p->deviceInfo = value;
                        _update();
                    });
        }
    }

    DevicesModel::DevicesModel() :
        _p(new Private)
    {
        DBG;
    }

    DevicesModel::~DevicesModel()
    {}

    std::shared_ptr<DevicesModel> DevicesModel::create(const std::shared_ptr<system::Context>& context)
    {

        DBG;
        auto out = std::shared_ptr<DevicesModel>(new DevicesModel);

        DBG;
        out->_init(context);

        DBG;
        return out;
    }

    std::shared_ptr<observer::IValue<DevicesModelData> > DevicesModel::observeData() const
    {
        return _p->data;
    }

    void DevicesModel::setDeviceIndex(int index)
    {
        TLRENDER_P();
        if (index == p.deviceIndex)
            return;
        p.deviceIndex = index;
        _update();
    }

    void DevicesModel::setDisplayModeIndex(int index)
    {
        TLRENDER_P();
        if (index == p.displayModeIndex)
            return;
        p.displayModeIndex = index;
        _update();
    }

    void DevicesModel::setPixelTypeIndex(int index)
    {
        TLRENDER_P();
        if (index == p.pixelTypeIndex)
            return;
        p.pixelTypeIndex = index;
        _update();
    }

    void DevicesModel::setVideoLevels(imaging::VideoLevels value)
    {
        TLRENDER_P();
        if (value == p.videoLevels)
            return;
        p.videoLevels = value;
        _update();
    }

    void DevicesModel::setHDRMode(device::HDRMode value)
    {
        TLRENDER_P();
        if (value == p.hdrMode)
            return;
        p.hdrMode = value;
        _update();
    }

    void DevicesModel::setHDRData(const imaging::HDRData& value)
    {
        TLRENDER_P();
        if (value == p.hdrData)
            return;
        p.hdrData = value;
        _update();
    }

    void DevicesModel::_update()
    {
        TLRENDER_P();

        DevicesModelData data;

        data.devices.push_back("None");
        for (const auto& i : p.deviceInfo)
        {
            data.devices.push_back(i.name);
        }
        data.deviceIndex = p.deviceIndex;

        data.displayModes.push_back("None");
        if (!p.deviceInfo.empty() && p.deviceIndex >= 1 && (p.deviceIndex - 1) < p.deviceInfo.size())
        {
            for (const auto& i : p.deviceInfo[p.deviceIndex - 1].displayModes)
            {
                data.displayModes.push_back(i.name);
            }
            data.displayModeIndex = p.displayModeIndex;
        }

        data.pixelTypes.push_back(device::PixelType::None);
        if (!p.deviceInfo.empty() && p.deviceIndex >= 1 && (p.deviceIndex - 1) < p.deviceInfo.size())
        {
            for (const auto& i : p.deviceInfo[p.deviceIndex - 1].pixelTypes)
            {
                data.pixelTypes.push_back(i);
            }
            data.pixelTypeIndex = p.pixelTypeIndex;
        }

        data.videoLevels = p.videoLevels;

        data.hdrMode = p.hdrMode;
        data.hdrData = p.hdrData;

        p.data->setIfChanged(data);
    }
}
