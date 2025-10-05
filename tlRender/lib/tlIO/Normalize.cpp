// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Normalize.h>

#include <Imath/half.h>

#include <sstream>

namespace tl
{
    namespace io
    {
        std::string serialize(const math::Vector4f& value)
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }

        void normalizeImage(
            math::Vector4f& minValue, math::Vector4f& maxValue,
            const std::shared_ptr<image::Image> in, const image::Info& info,
            const int minX, const int maxX, const int minY, const int maxY)
        {
            const size_t channels = image::getChannelCount(info.pixelType);
            const size_t channelByteCount =
                image::getBitDepth(info.pixelType) / 8;
            const size_t cb = channels * channelByteCount;

            uint8_t* p = in->getData();

            for (short i = 0; i < 4; ++i)
            {
                minValue[i] = std::numeric_limits<float>::max();
                maxValue[i] = std::numeric_limits<float>::min();
            }

            for (int y = minY; y <= maxY; ++y)
            {
                for (int x = minX; x <= maxX; ++x)
                {
                    switch (info.pixelType)
                    {
                    case image::PixelType::L_F16:
                    {
                        float value = static_cast<float>(static_cast<half>(*p));
                        if (value < minValue[0])
                            minValue[0] = value;
                        if (value > maxValue[0])
                            maxValue[0] = value;
                        break;
                    }
                    case image::PixelType::L_F32:
                    {
                        float value = static_cast<float>(*p);
                        if (value < minValue[0])
                            minValue[0] = value;
                        if (value > maxValue[0])
                            maxValue[0] = value;
                        break;
                    }
                    case image::PixelType::RGB_F16:
                    {
                        half* values = reinterpret_cast<half*>(p);
                        for (short i = 0; i < 3; ++i)
                        {
                            if (values[i] < minValue[i])
                                minValue[i] = values[i];
                            if (values[i] > maxValue[i])
                                maxValue[i] = values[i];
                        }
                        break;
                    }
                    case image::PixelType::RGB_F32:
                    {
                        float* values = reinterpret_cast<float*>(p);
                        for (short i = 0; i < 3; ++i)
                        {
                            if (values[i] < minValue[i])
                                minValue[i] = values[i];
                            if (values[i] > maxValue[i])
                                maxValue[i] = values[i];
                        }
                        break;
                    }
                    case image::PixelType::RGBA_F16:
                    {
                        half* values = reinterpret_cast<half*>(p);
                        for (short i = 0; i < 4; ++i)
                        {
                            if (values[i] < minValue[i])
                                minValue[i] = values[i];
                            if (values[i] > maxValue[i])
                                maxValue[i] = values[i];
                        }
                        break;
                    }
                    case image::PixelType::RGBA_F32:
                    {
                        float* values = reinterpret_cast<float*>(p);
                        for (short i = 0; i < 4; ++i)
                        {
                            if (values[i] < minValue[i])
                                minValue[i] = values[i];
                            if (values[i] > maxValue[i])
                                maxValue[i] = values[i];
                        }
                        break;
                    }
                    default:
                        break;
                    }
                    p += cb;
                }
            }

            for (short i = 0; i < 4; ++i)
            {
                if (minValue[i] == maxValue[i])
                {
                    minValue[i] = 0.F;
                    maxValue[i] = 1.F;
                }
            }
        }
    } // namespace io
} // namespace tl
