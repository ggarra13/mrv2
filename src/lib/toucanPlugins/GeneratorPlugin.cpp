// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "GeneratorPlugin.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <Imath/ImathVec.h>

GeneratorPlugin::GeneratorPlugin(const std::string& group, const std::string& name) :
    Plugin(group, name)
{}

GeneratorPlugin::~GeneratorPlugin()
{}

OfxStatus GeneratorPlugin::_describeAction(OfxImageEffectHandle handle)
{
    Plugin::_describeAction(handle);

    OfxPropertySetHandle effectProps;
    _effectSuite->getPropertySet(handle, &effectProps);
    _propSuite->propSetString(
        effectProps,
        kOfxImageEffectPropSupportedContexts,
        0,
        kOfxImageEffectContextGenerator);

    return kOfxStatOK;
}

OfxStatus GeneratorPlugin::_describeInContextAction(OfxImageEffectHandle handle, OfxPropertySetHandle inArgs)
{
    Plugin::_describeInContextAction(handle, inArgs);

    OfxPropertySetHandle outputProps;
    _effectSuite->clipDefine(handle, "Output", &outputProps);
    const std::vector<std::string> components =
    {
        kOfxImageComponentAlpha,
        kOfxImageComponentRGB,
        kOfxImageComponentRGBA
    };
    const std::vector<std::string> pixelDepths =
    {
        kOfxBitDepthByte,
        kOfxBitDepthShort,
        kOfxBitDepthFloat
    };
    for (int i = 0; i < components.size(); ++i)
    {
        _propSuite->propSetString(
            outputProps,
            kOfxImageEffectPropSupportedComponents,
            i,
            components[i].c_str());
    }
    for (int i = 0; i < pixelDepths.size(); ++i)
    {
        _propSuite->propSetString(
            outputProps,
            kOfxImageEffectPropSupportedPixelDepths,
            i,
            pixelDepths[i].c_str());
    }

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "size", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 1280);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 720);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Size");

    return kOfxStatOK;
}

OfxStatus GeneratorPlugin::_createInstance(OfxImageEffectHandle handle)
{
    Plugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "size", &_sizeParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus GeneratorPlugin::_renderAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    OfxTime time;
    OfxRectI renderWindow;
    _propSuite->propGetDouble(inArgs, kOfxPropTime, 0, &time);
    _propSuite->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

    OfxImageClipHandle outputClip = nullptr;
    OfxPropertySetHandle outputImage = nullptr;
    _effectSuite->clipGetHandle(handle, "Output", &outputClip, nullptr);
    if (outputClip)
    {
        _effectSuite->clipGetImage(outputClip, time, nullptr, &outputImage);
        if (outputImage)
        {
            OIIO::ImageBuf outputBuf = propSetToBuf(_propSuite, outputImage);
            _render(handle, outputBuf, renderWindow, inArgs);
        }
    }

    if (outputImage)
    {
        _effectSuite->clipReleaseImage(outputImage);
    }
    return kOfxStatOK;
}

CheckersPlugin* CheckersPlugin::_plugin = nullptr;

CheckersPlugin::CheckersPlugin() :
    GeneratorPlugin("toucan", "Checkers")
{}

CheckersPlugin::~CheckersPlugin()
{}

void CheckersPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new CheckersPlugin;
    }
    _plugin->_host = host;
}

OfxStatus CheckersPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus CheckersPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    GeneratorPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "checkerSize", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 100);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 100);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Checker Size");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color1", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color 1");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color2", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color 2");

    return kOfxStatOK;
}

OfxStatus CheckersPlugin::_createInstance(OfxImageEffectHandle handle)
{
    GeneratorPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "checkerSize", &_checkerSizeParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color1", &_color1Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color2", &_color2Param[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus CheckersPlugin::_render(
    OfxImageEffectHandle handle,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t checkerSize[2] = { 0, 0 };
    double color1[4] = { 0.0, 0.0, 0.0, 0.0 };
    double color2[4] = { 0.0, 0.0, 0.0, 0.0 };
    _paramSuite->paramGetValue(_checkerSizeParam[handle], &checkerSize[0], &checkerSize[1]);
    _paramSuite->paramGetValue(_color1Param[handle], &color1[0], &color1[1], &color1[2], &color1[3]);
    _paramSuite->paramGetValue(_color2Param[handle], &color2[0], &color2[1], &color2[2], &color2[3]);

    OIIO::ImageBufAlgo::checker(
        outputBuf,
        checkerSize[0],
        checkerSize[1],
        1,
        {
            static_cast<float>(color1[0]),
            static_cast<float>(color1[1]),
            static_cast<float>(color1[2]),
            static_cast<float>(color1[3])
        },
        {
            static_cast<float>(color2[0]),
            static_cast<float>(color2[1]),
            static_cast<float>(color2[2]),
            static_cast<float>(color2[3])
        });

    return kOfxStatOK;
}

FillPlugin* FillPlugin::_plugin = nullptr;

FillPlugin::FillPlugin() :
    GeneratorPlugin("toucan", "Fill")
{}

FillPlugin::~FillPlugin()
{}

void FillPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new FillPlugin;
    }
    _plugin->_host = host;
}

OfxStatus FillPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus FillPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    GeneratorPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color");

    return kOfxStatOK;
}

OfxStatus FillPlugin::_createInstance(OfxImageEffectHandle handle)
{
    GeneratorPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "color", &_colorParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus FillPlugin::_render(
    OfxImageEffectHandle handle,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double color[4] = { 0.0, 0.0, 0.0, 0.0 };
    _paramSuite->paramGetValue(
        _colorParam[handle],
        &color[0],
        &color[1],
        &color[2],
        &color[3]);
    OIIO::ImageBufAlgo::fill(
        outputBuf,
        {
            static_cast<float>(color[0]),
            static_cast<float>(color[1]),
            static_cast<float>(color[2]),
            static_cast<float>(color[3])
        });

    return kOfxStatOK;
}

GradientPlugin* GradientPlugin::_plugin = nullptr;

GradientPlugin::GradientPlugin() :
    GeneratorPlugin("toucan", "Gradient")
{}

GradientPlugin::~GradientPlugin()
{}

void GradientPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new GradientPlugin;
    }
    _plugin->_host = host;
}

OfxStatus GradientPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus GradientPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    GeneratorPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color1", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 0.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color 1");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color2", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color 2");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeBoolean, "vertical", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Vertical");

    return kOfxStatOK;
}

OfxStatus GradientPlugin::_createInstance(OfxImageEffectHandle handle)
{
    GeneratorPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "color1", &_color1Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color2", &_color2Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "vertical", &_verticalParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus GradientPlugin::_render(
    OfxImageEffectHandle handle,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double color1[4] = { 0.0, 0.0, 0.0, 0.0 };
    double color2[4] = { 0.0, 0.0, 0.0, 0.0 };
    bool vertical = false;
    _paramSuite->paramGetValue(_color1Param[handle], &color1[0], &color1[1], &color1[2], &color1[3]);
    _paramSuite->paramGetValue(_color2Param[handle], &color2[0], &color2[1], &color2[2], &color2[3]);
    _paramSuite->paramGetValue(_verticalParam[handle], &vertical);

    if (vertical)
    {
        OIIO::ImageBufAlgo::fill(
            outputBuf,
            {
                static_cast<float>(color1[0]),
                static_cast<float>(color1[1]),
                static_cast<float>(color1[2]),
                static_cast<float>(color1[3])
            },
            {
                static_cast<float>(color2[0]),
                static_cast<float>(color2[1]),
                static_cast<float>(color2[2]),
                static_cast<float>(color2[3])
            },
            OIIO::ROI());
    }
    else
    {
        const auto& spec = outputBuf.spec();
        auto gradient = OIIO::ImageBuf(OIIO::ImageSpec(spec.height, spec.width, spec.nchannels));
        OIIO::ImageBufAlgo::fill(
            gradient,
            {
                static_cast<float>(color1[0]),
                static_cast<float>(color1[1]),
                static_cast<float>(color1[2]),
                static_cast<float>(color1[3])
            },
            {
                static_cast<float>(color2[0]),
                static_cast<float>(color2[1]),
                static_cast<float>(color2[2]),
                static_cast<float>(color2[3])
            },
            OIIO::ROI());
        OIIO::ImageBufAlgo::copy(
            outputBuf,
            OIIO::ImageBufAlgo::rotate270(gradient));
    }

    return kOfxStatOK;
}

NoisePlugin* NoisePlugin::_plugin = nullptr;

NoisePlugin::NoisePlugin() :
    GeneratorPlugin("toucan", "Noise")
{}

NoisePlugin::~NoisePlugin()
{}

void NoisePlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new NoisePlugin;
    }
    _plugin->_host = host;
}

OfxStatus NoisePlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus NoisePlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    GeneratorPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "type", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "gaussian");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Type");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "a", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "A");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "b", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "B");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeBoolean, "mono", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Monochrome");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger, "seed", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Seed");

    return kOfxStatOK;
}

OfxStatus NoisePlugin::_createInstance(OfxImageEffectHandle handle)
{
    GeneratorPlugin::_createInstance(handle);
    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "type", &_typeParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "a", &_aParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "b", &_bParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "mono", &_monoParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "seed", &_seedParam[handle], nullptr);
    return kOfxStatOK;
}

OfxStatus NoisePlugin::_render(
    OfxImageEffectHandle handle,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    std::string type;
    double a = 0.0;
    double b = 0.0;
    int mono = 0;
    int64_t seed = 0;
    _paramSuite->paramGetValue(_typeParam[handle], &type);
    _paramSuite->paramGetValue(_aParam[handle], &a);
    _paramSuite->paramGetValue(_bParam[handle], &b);
    _paramSuite->paramGetValue(_monoParam[handle], &mono);
    _paramSuite->paramGetValue(_seedParam[handle], &seed);

    OIIO::ImageBufAlgo::noise(
        outputBuf,
        type,
        a,
        b,
        mono,
        seed);

    return kOfxStatOK;
}

namespace
{
    std::vector<OfxPlugin> plugins =
    {
        { kOfxImageEffectPluginApi, 1, "toucan:Checkers", 1, 0, CheckersPlugin::setHostFunc, CheckersPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Fill", 1, 0, FillPlugin::setHostFunc, FillPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Gradient", 1, 0, GradientPlugin::setHostFunc, GradientPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Noise", 1, 0, NoisePlugin::setHostFunc, NoisePlugin::mainEntryPoint }
    };
}

extern "C"
{
    int OfxGetNumberOfPlugins(void)
    {
        return plugins.size();
    }

    OfxPlugin* OfxGetPlugin(int index)
    {
        if (index >= 0 && index < plugins.size())
        {
            return &plugins[index];
        }
        return 0;
    }
}
