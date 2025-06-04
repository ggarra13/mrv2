// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

//
// Small portions of this code have been borrowed from OpenImageIO.
// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/OpenImageIO/oiio
//

#include <tlIO/RAW.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <libraw/libraw.h>

#define LIBRAW_ERROR(function, ret)                                            \
    if (ret)                                                                   \
    {                                                                          \
        throw std::runtime_error(string::Format("{0} - {1}")                   \
                                     .arg(#function)                           \
                                     .arg(libraw_strerror(ret)));              \
    }

namespace
{
    const char* libraw_filter_to_str(unsigned int filters)
    {
        // Convert the libraw filter pattern description
        // into a slightly more human readable string
        // LibRaw/internal/defines.h:166
        switch (filters)
        {
            // CYGM
        case 0xe1e4e1e4:
            return "GMYC";
        case 0x1b4e4b1e:
            return "CYGM";
        case 0x1e4b4e1b:
            return "YCGM";
        case 0xb4b4b4b4:
            return "GMCY";
        case 0x1e4e1e4e:
            return "CYMG";

            // RGB
        case 0x16161616:
            return "BGRG";
        case 0x61616161:
            return "GRGB";
        case 0x49494949:
            return "GBGR";
        case 0x94949494:
            return "RGBG";
        default:
            break;
        }
        return "";
    }

    void get_local_time(const time_t* time, struct tm* converted_time)
    {
#ifdef _WIN32
        localtime_s(converted_time, time);
#else
        localtime_r(time, converted_time);
#endif
    }
} // namespace

namespace tl
{
    namespace raw
    {
        namespace
        {

            class File
            {
            public:
                File(
                    const std::string& fileName, const file::MemoryRead* memory)
                {
                    int ret;
                    {
                        // LibRaw is not thread safe.  We use a static mutex
                        // so only one threads constructs a LibRaw at a
                        // time.
                        std::lock_guard<std::mutex> lock(_mutex);
                        _processor.reset(new LibRaw());
                    }

                    _memory = memory;

                    _openFile(fileName);

                    _info.video.resize(1);
                    auto& info = _info.video[0];

                    const auto& sizes(_processor->imgdata.sizes);

                    info.size.w = sizes.iwidth;
                    info.size.h = sizes.iheight;
                    info.size.pixelAspectRatio = sizes.pixel_aspect;
                    info.layout.mirror.y = true;

                    // Save Tags
                    auto& tags = _info.tags;

                    // Use some aliases
                    const auto& idata(_processor->imgdata.idata);
                    const auto& color(_processor->imgdata.color);
                    const auto& other(_processor->imgdata.other);
                    if (idata.make[0])
                        _storeTag("Make", idata.make);
                    if (idata.model[0])
                        _storeTag("Model", idata.model);
                    _storeTag("Normalized Make", idata.normalized_make);
                    _storeTag("Normaliized Model", idata.normalized_model);
                    if (idata.software[0])
                        _storeTag("Software", idata.software);
                    else if (color.model2[0])
                        _storeTag("Software", color.model2);
                    _storeTag("Exif:Flash", (int)color.flash_used);

                    _storeTag("Orientation", _getOrientation(sizes.flip));
                    _storeTag("ISO Speed Ratings", other.iso_speed);
                    _storeTag("Exposure Time", other.shutter);
                    _storeTag("Shutter Speed Value", -std::log2(other.shutter));
                    _storeTag("FNumber", other.aperture);
                    _storeTag(
                        "Aperture Value", 2.0f * std::log2(other.aperture));
                    _storeTag("Focal Length", other.focal_len);

                    struct tm m_tm;
                    get_local_time(&_processor->imgdata.other.timestamp, &m_tm);
                    char datetime[20];
                    strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", &m_tm);
                    _storeTag("DateTime", datetime);
                    _storeTag("raw:ShotOrder", other.shot_order);
                    if (other.parsed_gps.gpsparsed)
                    {
                        _storeTag("GPS:Latitude", other.parsed_gps.latitude);
                        _storeTag("GPS:Longitude", other.parsed_gps.longitude);
                        _storeTag(
                            "GPS:TimeStamp", other.parsed_gps.gpstimestamp);
                        _storeTag("GPS:Altitude", other.parsed_gps.altitude);
                        _storeTag("GPS:LatitudeRef", &other.parsed_gps.latref);
                        _storeTag(
                            "GPS:LongitudeRef", &other.parsed_gps.longref);
                        _storeTag("GPS:AltitudeRef", &other.parsed_gps.altref);
                        _storeTag("GPS:Status", &other.parsed_gps.gpsstatus);
                    }

                    const auto& makernotes(_processor->imgdata.makernotes);
                    const auto& common(makernotes.common);
                    _storeTag("Exif:Humidity", common.exifHumidity);
                    _storeTag("Exif:Pressure", common.exifPressure);
                    _storeTag("Exif:WaterDepth", common.exifWaterDepth);
                    _storeTag("Exif:Acceleration", common.exifAcceleration);
                    _storeTag(
                        "Exif:CameraElevationAngle",
                        common.exifCameraElevationAngle);
                    _storeTag(
                        "raw:pre_mul", color.pre_mul[0], color.pre_mul[3]);
                    _storeTag(
                        "raw:cam_mul", color.cam_mul[0], color.cam_mul[3]);
                    _storeTag(
                        "raw:rgb_cam", color.rgb_cam[0][0],
                        color.rgb_cam[2][3]);
                    _storeTag(
                        "raw:cam_xyz", color.cam_xyz[0][0],
                        color.cam_xyz[3][2]);

                    _processor->recycle();

                    info.pixelType = image::PixelType::RGB_U16;
                    info.layout.endian = memory::Endian::LSB;
                }

                const io::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    int ret;
                    io::VideoData out;
                    out.time = time;
                    const auto& info = _info.video[0];
                    out.image = image::Image::create(info);

                    auto tags = _info.tags;
                    tags["otioClipName"] = fileName;
                    {
                        std::stringstream ss;
                        ss << time;
                        tags["otioClipTime"] = ss.str();
                    }
                    out.image->setTags(tags);

                    auto& params(_processor->imgdata.params);

                    // Output 16-bit images
                    params.output_bps = 16;

                    // Some default parameters
                    params.no_auto_bright = 1;
                    params.adjust_maximum_thr = 0.0f;
                    params.user_sat = 0;
                    params.use_camera_wb = 1;
                    params.user_flip = -1;
                    params.user_qual = 3;

                    // Handle white balance
                    const auto& color = _processor->imgdata.color;
                    const auto& idata = _processor->imgdata.idata;

                    auto is_rgbg_or_bgrg = [&](unsigned int filters)
                    {
                        std::string filter(libraw_filter_to_str(filters));
                        return filter == "RGBG" || filter == "BGRG";
                    };
                    float norm[4] = {
                        color.cam_mul[0], color.cam_mul[1], color.cam_mul[2],
                        color.cam_mul[3]};

                    if (is_rgbg_or_bgrg(idata.filters))
                    {
                        // normalize white balance around green
                        norm[0] /= norm[1];
                        norm[1] /= norm[1];
                        norm[2] /= norm[3] > 0 ? norm[3] : norm[1];
                        norm[3] /= norm[3] > 0 ? norm[3] : norm[1];
                    }
                    params.user_mul[0] = norm[0];
                    params.user_mul[1] = norm[1];
                    params.user_mul[2] = norm[2];
                    params.user_mul[3] = norm[3];

                    // Handle camera matrix
                    params.use_camera_matrix = 1;

                    // Highlight adjustment
                    // 0  = Clip
                    // 1  = Unclip
                    // 2  = Blend
                    params.highlight = 0;

                    // Handle LCMS color correction with sRGB as output
                    params.output_color = 1;
                    params.gamm[0] = 1.0 / 2.4;
                    params.gamm[1] = 12.92;

                    _openFile(fileName);

                    float old_max_thr = params.adjust_maximum_thr;

                    // Disable max threshold for highlight adjustment
                    params.adjust_maximum_thr = 0.0f;

                    const auto& sizes(_processor->imgdata.sizes);

                    ret = _processor->raw2image_ex(/*substract_black=*/true);
                    LIBRAW_ERROR(raw2image_ex, ret);

                    ret = _processor->adjust_maximum();
                    LIBRAW_ERROR(adjust_maximum, ret);

                    float unadjusted = _processor->imgdata.color.maximum;

                    params.adjust_maximum_thr =
                        (old_max_thr == 0.0f) ? 1.0 : old_max_thr;

                    ret = _processor->adjust_maximum();
                    LIBRAW_ERROR(adjust_maximum, ret);

                    // Restore old max threashold
                    params.adjust_maximum_thr = old_max_thr;

                    ret = _processor->dcraw_process();
                    LIBRAW_ERROR(dcraw_process, ret);

                    _image = _processor->dcraw_make_mem_image(&ret);
                    LIBRAW_ERROR(dcraw_make_mem_image, ret);
                    if (!_image)
                    {
                        throw std::runtime_error(
                            "dcraw_make_mem_image returned null");
                    }

                    if (_image->type != LIBRAW_IMAGE_BITMAP)
                    {
                        throw std::runtime_error("Not a bitmap image");
                    }

                    if (_image->colors == 3)
                    {
                        memcpy(
                            out.image->getData(), _image->data,
                            _image->data_size);
                    }
                    else if (_image->colors == 1)
                    {
                        uint16_t* data =
                            reinterpret_cast<uint16_t*>(out.image->getData());
                        for (size_t i = 0; i < _image->data_size; ++i)
                        {
                            const size_t j = i * 3;
                            data[j] = _image->data[i];
                            data[j + 1] = _image->data[i];
                            data[j + 2] = _image->data[i];
                        }
                    }
                    else
                    {
                        throw std::runtime_error("Unsupport color depth");
                    }
                    _processor->dcraw_clear_mem(_image);
                    _processor->recycle();

                    return out;
                }

            protected:
                void _openFile(const std::string& fileName)
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    int ret;
                    if (_memory)
                    {
                        ret = _processor->open_buffer(
                            reinterpret_cast<void*>(
                                const_cast<uint8_t*>(_memory->p)),
                            _memory->size);
                        LIBRAW_ERROR(open_buffer, ret);
                    }
                    else
                    {
#ifdef _WIN32
                        const std::wstring wideFileName =
                            string::toWide(fileName);
                        ret = _processor->open_file(wideFileName.c_str());
#else
                        ret = _processor->open_file(fileName.c_str());
#endif
                        LIBRAW_ERROR(open_file, ret);
                    }

                    // Let us unpack the image
                    ret = _processor->unpack();
                    LIBRAW_ERROR(unpack, ret);

                    ret = _processor->adjust_sizes_info_only();
                    LIBRAW_ERROR(adjust_sizes_info_only, ret);
                }

                const char* _getOrientation(int flip)
                {
                    switch (flip)
                    {
                    case 5: // 90 deg counter clockwise
                        return "90 Degrees Counter Clockwise";
                    case 6: // 90 deg clockwise
                        return "90 Degrees Clockwise";
                    case 0: // no rotation
                        return "No Rotation";
                    case 3: // 180 degree rotation
                        return "180 degree rotation";
                    default:
                        return "Unknown";
                    }
                }

                template <typename T>
                void _storeTag(const char* tag, const T& value)
                {
                    std::stringstream ss;
                    ss << value;
                    _info.tags[tag] = ss.str();
                }

                template <typename T>
                void
                _storeTag(const char* tag, const T& value1, const T& value2)
                {
                    std::stringstream ss;
                    ss << value1 << " " << value2;
                    _info.tags[tag] = ss.str();
                }

            private:
                static std::mutex _mutex;
                std::unique_ptr<LibRaw> _processor;
                libraw_processed_image_t* _image = nullptr;
                io::Info _info;
                const file::MemoryRead* _memory;
            };
        } // namespace

        std::mutex File::_mutex;

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
        }

        Read::Read() {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName, const file::MemoryRead* memory)
        {
            io::Info out = File(fileName, memory).getInfo();
            out.videoTime =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, _defaultSpeed),
                    otime::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const otime::RationalTime& time, const io::Options& options)
        {
            return File(fileName, memory).read(fileName, time);
        }
    } // namespace raw
} // namespace tl
