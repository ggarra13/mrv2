// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define PYBIND11_DETAILED_ERROR_MESSAGES
namespace py = pybind11;

#include <tlCore/Vector.h>
#include <tlCore/Image.h>
#include <tlCore/StringFormat.h>

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/LUTOptions.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvStereo3DOptions.h"
#include "mrvCore/mrvEnvironmentMapOptions.h"

#include "mrViewer.h"

namespace tl
{
    namespace image
    {
        inline std::ostream& operator<<(std::ostream& o, const Mirror& a)
        {
            o << "<mrv2.image.Mirror x=" << (a.x ? "True" : "False")
              << " y=" << (a.y ? "True" : "False") << ">";
            return o;
        }
    } // namespace image

    namespace timeline
    {
        inline std::ostream& operator<<(std::ostream& o, const ImageFilters& a)
        {
            o << "<mrv2.image.ImageFilters minify=" << a.minify
              << " magnify=" << a.magnify << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const Color& a)
        {
            o << "<mrv2.image.Color enabled=" << (a.enabled ? "True" : "False")
              << " add=<mrv2.math.Vector3f " << a.add << ">"
              << " brightness=<mrv2.math.Vector3f " << a.brightness << ">"
              << " contrast=<mrv2.math.Vector3f " << a.contrast << ">"
              << " saturation=<mrv2.math.Vector3f " << a.saturation << ">"
              << " tint=" << a.tint
              << " invert=" << (a.invert ? "True" : "False") << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const Levels& a)
        {
            o << "<mrv2.image.Levels enabled=" << (a.enabled ? "True" : "False")
              << " inLow=" << a.inLow << " inHigh=" << a.inHigh
              << " gamma=" << a.gamma << " outLow=" << a.outLow
              << " outHigh=" << a.outHigh << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const EXRDisplay& a)
        {
            o << "<mrv2.image.EXRDisplay enabled="
              << (a.enabled ? "True" : "False") << " exposure=" << a.exposure
              << " defog=" << a.defog << " kneeLow=" << a.kneeLow
              << " kneeHigh=" << a.kneeHigh << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const SoftClip& a)
        {
            o << "<mrv2.image.SoftClip enabled="
              << (a.enabled ? "True" : "False") << " value=" << a.value << ">";
            return o;
        }

        inline std::ostream&
        operator<<(std::ostream& o, const DisplayOptions& a)
        {
            o << "<mrv2.image.DisplayOptions channels" << a.channels
              << " mirror=" << a.mirror << " color=" << a.color
              << " levels=" << a.levels << " softClip=" << a.softClip
              << " imageFilters=" << a.imageFilters
              << " videoLevels=" << a.videoLevels << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& s, const ImageOptions& o)
        {
            s << "<mrv2.image.imageOptions videoLevels=" << o.videoLevels
              << " alphaBlend=" << o.alphaBlend
              << " imageFilters=" << o.imageFilters << ">";
            return s;
        }

        inline std::ostream& operator<<(std::ostream& s, const LUTOptions& o)
        {
            s << "<mrv2.image.LUTOptions fileName=" << o.fileName
              << " order=" << o.order << ">";
            return s;
        }

        inline std::ostream&
        operator<<(std::ostream& s, const BackgroundOptions& o)
        {
            s << "<mrv2.image.BackgroundOptions type=" << o.type
              << " solidColor=" << o.solidColor
              << " checkersSize=" << o.checkersSize
              << " checkersColor0=" << o.checkersColor0
              << " checkersColor1=" << o.checkersColor1 << ">";
            return s;
        }

    } // namespace timeline
} // namespace tl

namespace mrv
{
    inline std::ostream&
    operator<<(std::ostream& s, const EnvironmentMapOptions& o)
    {
        s << "<mrv2.image.EnvironmentMapOptions type=" << o.type
          << " horizonalAperture=" << o.horizontalAperture
          << " verticalAperture=" << o.verticalAperture
          << " focalLength=" << o.focalLength << " rotateX=" << o.rotateX
          << " rotateY=" << o.rotateY << " subdivisionX=" << o.subdivisionX
          << " subdivisionY=" << o.subdivisionY
          << " spin=" << (o.spin ? "True" : "False") << ">";
        return s;
    }

    inline std::ostream& operator<<(std::ostream& s, const Stereo3DOptions& o)
    {
        std::stringstream ss;
        ss << o.input;
        s << "<mrv2.image.Stereo3DOptions input="
          << (ss.str() == "None" ? "kNone" : ss.str()) << " output=" << o.output
          << " eyeSeparation=" << o.eyeSeparation
          << " swapEyes=" << (o.swapEyes ? "True" : "False") << ">";
        return s;
    }

    namespace image
    {
        tl::timeline::BackgroundOptions backgroundOptions()
        {
            ViewerUI* ui = App::ui;
            return ui->uiView->getBackgroundOptions();
        }

        void setBackgroundOptions(const tl::timeline::BackgroundOptions& value)
        {
            ViewerUI* ui = App::ui;
            ui->uiView->setBackgroundOptions(value);
        }

        std::string ocioConfig()
        {
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;
            const char* out = uiPrefs->uiPrefsOCIOConfig->value();
            if (!out)
                return "";
            return out;
        }

        void setOcioConfig(const std::string config)
        {
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;
            if (config.empty())
            {
                throw std::runtime_error(
                    _("OCIO config file cannot be empty."));
            }
            if (!file::isReadable(config))
            {
                std::string err = string::Format(_("OCIO config '{0}' does not "
                                                   "exist or is not readable."))
                                      .arg(config);
                throw std::runtime_error(err);
            }

            const char* oldconfig = uiPrefs->uiPrefsOCIOConfig->value();
            if (oldconfig && strlen(oldconfig) > 0)
            {
                // Same config file.  Nothing to do.
                if (config == oldconfig)
                    return;
            }

            uiPrefs->uiPrefsOCIOConfig->value(config.c_str());
            Preferences::OCIO(ui);
        }

        std::string ocioIcs()
        {
            auto uiICS = App::ui->uiICS;
            int idx = uiICS->value();
            if (idx < 0 || idx >= uiICS->children())
                return "";
            return uiICS->label();
        }

        void setOcioIcs(const unsigned value)
        {
            auto uiICS = App::ui->uiICS;
            if (value >= uiICS->children())
                return;
            uiICS->value(value);
            uiICS->do_callback();
        }

        std::vector<std::string> ocioIcsList()
        {
            auto uiICS = App::ui->uiICS;
            std::vector<std::string> out;
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label())
                    continue;

                out.push_back(item->label());
            }
            return out;
        }

        void setOcioIcs(const std::string& name)
        {
            auto uiICS = App::ui->uiICS;
            int value = -1;
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label())
                    continue;

                if (name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Ics '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiICS->value(value);
            uiICS->do_callback();
        }

        std::string ocioView()
        {
            auto uiOCIOView = App::ui->OCIOView;
            int idx = uiOCIOView->value();
            if (idx < 0 || idx >= uiOCIOView->children())
                return "";
            return uiOCIOView->label();
        }

        std::vector<std::string> ocioViewList()
        {
            auto uiOCIOView = App::ui->OCIOView;
            std::vector<std::string> out;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label())
                    continue;

                out.push_back(item->label());
            }
            return out;
        }

        void setOcioView(const unsigned value)
        {
            auto uiOCIOView = App::ui->OCIOView;
            if (value >= uiOCIOView->children())
                return;
            uiOCIOView->value(value);
            uiOCIOView->do_callback();
        }

        void setOcioView(const std::string& name)
        {
            auto uiOCIOView = App::ui->OCIOView;
            int value = -1;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label())
                    continue;

                if (name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Display/View '{0}'."))
                        .arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiOCIOView->value(value);
            uiOCIOView->do_callback();
        }
    } // namespace image
} // namespace mrv

void mrv2_image(pybind11::module& m)
{
    using namespace tl;

    py::module image = m.def_submodule("image");
    image.doc() = _(R"PYTHON(
Image module.

Contains all classes and enums related to image controls. 
)PYTHON");

    py::class_<image::Color4f>(image, "Color4f")
        .def(
            py::init<float, float, float, float>(), py::arg("r"), py::arg("g"),
            py::arg("b"), py::arg("a") = 1.F)
        .def_readwrite("r", &image::Color4f::r, _("Red Channel."))
        .def_readwrite("g", &image::Color4f::g, _("Green Channel."))
        .def_readwrite("b", &image::Color4f::b, _("Blue Channel."))
        .def_readwrite("a", &image::Color4f::a, _("Alpha Channel."))
        .def(
            "__repr__",
            [](const image::Color4f& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image mirroring.");

    py::class_<image::Mirror>(image, "Mirror")
        .def(py::init<bool, bool>(), py::arg("x") = false, py::arg("y") = false)
        .def_readwrite("x", &image::Mirror::x, _("Flip image on X."))
        .def_readwrite("y", &image::Mirror::y, _("Flip image on Y."))
        .def(
            "__repr__",
            [](const image::Mirror& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image mirroring.");

    // Cannot be timeline as it clashes with timeline::Color class
    py::class_<timeline::Color>(image, "Color")
        .def(py::init<>())
        .def(
            py::init<
                bool, math::Vector3f, math::Vector3f, math::Vector3f,
                math::Vector3f, float, bool>(),
            py::arg("enabled"), py::arg("add"), py::arg("brightness"),
            py::arg("contrast"), py::arg("saturation"), py::arg("tint"),
            py::arg("invert"))
        .def_readwrite(
            "enabled", &timeline::Color::enabled, _("Enabled Levels."))
        .def_readwrite(
            "add", &timeline::Color::add,
            _("Add a :class:`mrv2.math.Vector3f` to image."))
        .def_readwrite(
            "brightness", &timeline::Color::brightness,
            _("Change a :class:`mrv2.math.Vector3f` of brightness"
              " to image."))
        .def_readwrite(
            "contrast", &timeline::Color::contrast,
            _("Change a :class:`mrv2.math.Vector3f` of contrast"
              " to image."))
        .def_readwrite(
            "saturation", &timeline::Color::saturation,
            _("Change a :class:`mrv2.math.Vector3f` of saturation"
              " to image."))
        .def_readwrite(
            "tint", &timeline::Color::tint,
            _("Change tint of image to image between 0 and 1."))
        .def_readwrite(
            "invert", &timeline::Color::invert, _("Invert the color values."))
        .def(
            "__repr__",
            [](const timeline::Color& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Color values.");

    py::class_<timeline::Levels>(image, "Levels")
        .def(
            py::init<bool, float, float, float, float, float>(),
            py::arg("enabled") = false, py::arg("inLow") = 0.F,
            py::arg("inHigh") = 1.F, py::arg("gamma") = 1.F,
            py::arg("outLow") = 0.F, py::arg("outHigh") = 1.F)
        .def_readwrite(
            "enabled", &timeline::Levels::enabled, _("Enabled Levels."))
        .def_readwrite(
            "inLow", &timeline::Levels::inLow, _("In Low Level value."))
        .def_readwrite(
            "inHigh", &timeline::Levels::inHigh, _("In High Level value."))
        .def_readwrite(
            "gamma", &timeline::Levels::gamma, _("Gamma Level value."))
        .def_readwrite(
            "outLow", &timeline::Levels::outLow, _("Out Low Level value."))
        .def_readwrite(
            "outHigh", &timeline::Levels::outHigh, _("Out High Level value."))
        .def(
            "__repr__",
            [](const timeline::Levels& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Levels values.");

    py::class_<timeline::EXRDisplay>(image, "EXRDisplay")
        .def(
            py::init<bool, float, float, float, float>(),
            py::arg("enabled") = false, py::arg("exposure") = 0.F,
            py::arg("defog") = 0.F, py::arg("kneeLow") = 0.F,
            py::arg("kneeHigh") = 5.F)
        .def_readwrite(
            "enabled", &timeline::EXRDisplay::enabled,
            _("Enabled EXR display."))
        .def_readwrite(
            "exposure", &timeline::EXRDisplay::exposure, _("Exposure value."))
        .def_readwrite("defog", &timeline::EXRDisplay::defog, _("Defog value."))
        .def_readwrite(
            "kneeLow", &timeline::EXRDisplay::kneeLow, _("kneeLow value."))
        .def_readwrite(
            "kneeHigh", &timeline::EXRDisplay::kneeHigh, _("kneeHigh value."))
        .def(
            "__repr__",
            [](const timeline::EXRDisplay& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("EXR display values.");

    py::class_<timeline::SoftClip>(image, "SoftClip")
        .def(
            py::init<bool, float>(), py::arg("enabled") = false,
            py::arg("value") = 0.F)
        .def_readwrite(
            "enabled", &timeline::SoftClip::enabled, _("Enabled Soft Clip."))
        .def_readwrite(
            "value", &timeline::SoftClip::value, _("Soft clip value."))
        .def(
            "__repr__",
            [](const timeline::SoftClip& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Soft clip value.");

    py::class_<timeline::ImageFilters>(image, "ImageFilters")
        .def(py::init<>())
        .def(
            py::init<timeline::ImageFilter, timeline::ImageFilter>(),
            py::arg("minify") = timeline::ImageFilter::Nearest,
            py::arg("magnify") = timeline::ImageFilter::Nearest)
        .def_readwrite(
            "minify", &timeline::ImageFilters::minify,
            _("Minify filter :class:`mrv2.image.ImageFilter`."))
        .def_readwrite(
            "magnify", &timeline::ImageFilters::magnify,
            _("Magnify filter :class:`mrv2.image.ImageFilter`."))
        .def(
            "__repr__",
            [](const timeline::ImageFilters& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image filters.");

    py::class_<timeline::DisplayOptions>(image, "DisplayOptions")
        .def(py::init<>())
        .def(
            py::init<
                timeline::Channels, image::Mirror, timeline::Color,
                timeline::Levels, timeline::EXRDisplay, timeline::SoftClip,
                timeline::ImageFilters, image::VideoLevels>(),
            py::arg("channels") = timeline::Channels::Color, py::arg("mirror"),
            py::arg("color"), py::arg("levels"), py::arg("exrDisplay"),
            py::arg("softClip"), py::arg("imageFilters"),
            py::arg("videoLevels") = image::VideoLevels::FullRange)
        .def_readwrite(
            "channels", &timeline::DisplayOptions::channels,
            _("Color channels :class:`mrv2.image.Channels`."))
        .def_readwrite(
            "mirror", &timeline::DisplayOptions::mirror,
            _("Mirror on X, Y or both :class:`mrv2.image.Mirror`."))
        .def_readwrite(
            "color", &timeline::DisplayOptions::color,
            _("Color options :class:`mrv2.image.Color`."))
        .def_readwrite(
            "levels", &timeline::DisplayOptions::levels,
            _("Levels options :class:`mrv2.image.Levels`."))
        .def_readwrite(
            "exrDisplay", &timeline::DisplayOptions::exrDisplay,
            _("EXR Display options :class:`mrv2.image.EXRDisplay`.."))
        .def_readwrite(
            "softClip", &timeline::DisplayOptions::softClip,
            _("Soft Clip options :class:`mrv2.image.SoftClip`.."))
        .def(
            "__repr__",
            [](const timeline::DisplayOptions& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Display options.");

    py::class_<timeline::LUTOptions>(image, "LUTOptions")
        .def(py::init<>())
        .def(
            py::init<bool, std::string, timeline::LUTOrder>(),
            py::arg("enabled") = false, py::arg("fileName") = "",
            py::arg("order") = timeline::LUTOrder::First)
        .def_readwrite(
            "enable", &timeline::LUTOptions::enabled, _("LUT enabled."))
        .def_readwrite(
            "fileName", &timeline::LUTOptions::fileName, _("LUT filename."))
        .def_readwrite(
            "order", &timeline::LUTOptions::order,
            _("LUT transformation order."))
        .def(
            "__repr__",
            [](const timeline::LUTOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("LUT options.");

    py::class_<timeline::ImageOptions>(image, "ImageOptions")
        .def(py::init<>())
        .def(
            py::init<
                timeline::InputVideoLevels, timeline::AlphaBlend,
                timeline::ImageFilters>(),
            py::arg("videoLevels"), py::arg("alphaBlend"),
            py::arg("imageFilters"))
        .def_readwrite(
            "videoLevels", &timeline::ImageOptions::videoLevels,
            _("Video Levels."))
        .def_readwrite(
            "alphaBlend", &timeline::ImageOptions::alphaBlend,
            _("Alpha blending algorithm"))
        .def_readwrite(
            "imageFilters", &timeline::ImageOptions::imageFilters,
            _("Image Filters"))
        .def(
            "__repr__",
            [](const timeline::ImageOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image options.");

    py::class_<mrv::EnvironmentMapOptions>(image, "EnvironmentMapOptions")
        .def(py::init<>())
        .def(
            py::init<
                mrv::EnvironmentMapOptions::Type, float, float, float, float,
                float, unsigned, unsigned, bool>(),
            py::arg("type") = mrv::EnvironmentMapOptions::kNone,
            py::arg("horizontalAperture") = 24.F,
            py::arg("verticalAperture") = 0.F, py::arg("focalLength") = 45.F,
            py::arg("rotateX") = 0.F, py::arg("rotateY") = 0.F,
            py::arg("subdivisionX") = 36, py::arg("subdivisionY") = 36,
            py::arg("spin") = true)
        .def_readwrite(
            "type", &mrv::EnvironmentMapOptions::type,
            _("Environment Map type."))
        .def_readwrite(
            "horizontalAperture",
            &mrv::EnvironmentMapOptions::horizontalAperture,
            _("Horizontal aperture"))
        .def_readwrite(
            "verticalAperture", &mrv::EnvironmentMapOptions::verticalAperture,
            _("Vertical aperture"))
        .def_readwrite(
            "focalLength", &mrv::EnvironmentMapOptions::focalLength,
            _("Focal Length"))
        .def_readwrite(
            "rotateX", &mrv::EnvironmentMapOptions::rotateX, _("Rotation on X"))
        .def_readwrite(
            "rotateY", &mrv::EnvironmentMapOptions::rotateY, _("Rotation on Y"))
        .def_readwrite(
            "subdivisionX", &mrv::EnvironmentMapOptions::subdivisionX,
            _("Subdivision on X"))
        .def_readwrite(
            "subdivisionY", &mrv::EnvironmentMapOptions::subdivisionY,
            _("Subdivision on Y"))
        .def_readwrite("spin", &mrv::EnvironmentMapOptions::spin, _("Spin"))
        .def(
            "__repr__",
            [](const mrv::EnvironmentMapOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("EnvironmentMap options.");

    py::class_<mrv::Stereo3DOptions>(image, "Stereo3DOptions")
        .def(py::init<>())
        .def(
            py::init< mrv::Stereo3DInput, mrv::Stereo3DOutput, float, bool>(),
            py::arg("input"), py::arg("output"), py::arg("eyeSeparation") = 0.F,
            py::arg("swapEyes") = false)
        .def_readwrite(
            "input", &mrv::Stereo3DOptions::input,
            _("Stereo 3d input :class:`mrv2.image.StereoInput`.."))
        .def_readwrite(
            "output", &mrv::Stereo3DOptions::input,
            _("Stereo 3d output :class:`mrv2.image.StereoOutput`.."))
        .def_readwrite(
            "eyeSeparation", &mrv::Stereo3DOptions::eyeSeparation,
            _("Separation between left and right eye."))
        .def_readwrite(
            "swapEyes", &mrv::Stereo3DOptions::swapEyes,
            _("Swap left and right eye"))
        .def(
            "__repr__",
            [](const mrv::Stereo3DOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Stereo3D options.");

    py::class_<timeline::BackgroundOptions>(image, "BackgroundOptions")
        .def(py::init<>())
        .def(
            py::init<
                timeline::Background, image::Color4f, image::Color4f,
                image::Color4f, math::Size2i>(),
            py::arg("type") = timeline::Background::Transparent,
            py::arg("solidColor") = image::Color4f(0.F, 0.F, 0.F),
            py::arg("checkersColor0") = image::Color4f(1.F, 1.F, 1.F),
            py::arg("checkersColor1") = image::Color4f(0.5F, 0.5F, 0.5F),
            py::arg("checkersSize") = math::Size2i(100, 100))
        .def_readwrite(
            "type", &timeline::BackgroundOptions::type,
            _("Background type :class:`mrv2.image.Background`.."))
        .def_readwrite(
            "solidColor", &timeline::BackgroundOptions::solidColor,
            _("Solid Color :class:`mrv2.image.Color4f`.."))
        .def_readwrite(
            "checkersColor0", &timeline::BackgroundOptions::checkersColor0,
            _("Checkers Color0 :class:`mrv2.image.Color4f`.."))
        .def_readwrite(
            "checkersColor1", &timeline::BackgroundOptions::checkersColor1,
            _("Checkers Color1 :class:`mrv2.image.Color4f`.."))
        .def_readwrite(
            "checkersSize", &timeline::BackgroundOptions::checkersSize,
            _("Checkers Size :class:`mrv2.math.Size2i`.."))
        .def(
            "__repr__",
            [](const timeline::BackgroundOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Background options.");

    image.def(
        "backgroundOptions", &mrv::image::backgroundOptions,
        _("Gets the current background options."));

    image.def(
        "setBackgroundOptions", &mrv::image::setBackgroundOptions,
        _("Sets the current background options."));

    image.def(
        "ocioConfig", &mrv::image::ocioConfig,
        _("Gets the current OCIO config file."));

    image.def(
        "setOcioConfig", &mrv::image::setOcioConfig,
        _("Sets the current OCIO config file."));

    image.def(
        "ocioIcs", &mrv::image::ocioIcs,
        _("Gets the current input color space."));

    image.def(
        "ocioIcsList", &mrv::image::ocioIcsList,
        _("Gets a list of all input color spaces."));

    image.def(
        "setOcioIcs",
        py::overload_cast<const unsigned>(&mrv::image::setOcioIcs),
        _("Set the input color space."), py::arg("index"));

    image.def(
        "setOcioIcs",
        py::overload_cast<const std::string&>(&mrv::image::setOcioIcs),
        _("Set the input color space."), py::arg("ics"));

    image.def(
        "ocioView", &mrv::image::ocioView, _("Gets the current Display/View."));

    image.def(
        "setOcioView",
        py::overload_cast<const unsigned>(&mrv::image::setOcioView),
        _("Set an OCIO Display/View."), py::arg("index"));

    image.def(
        "setOcioView",
        py::overload_cast<const std::string&>(&mrv::image::setOcioView),
        _("Set an OCIO Display/View."), py::arg("view"));
}
