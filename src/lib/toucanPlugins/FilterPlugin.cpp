// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "FilterPlugin.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

FilterPlugin::FilterPlugin(const std::string& group, const std::string& name) :
    Plugin(group, name)
{}

FilterPlugin::~FilterPlugin()
{}

OfxStatus FilterPlugin::_describeAction(OfxImageEffectHandle handle)
{
    Plugin::_describeAction(handle);

    OfxPropertySetHandle effectProps;
    _effectSuite->getPropertySet(handle, &effectProps);
    _propSuite->propSetString(
        effectProps,
        kOfxImageEffectPropSupportedContexts,
        0,
        kOfxImageEffectContextFilter);

    return kOfxStatOK;
}

OfxStatus FilterPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    Plugin::_describeInContextAction(handle, inArgs);

    OfxPropertySetHandle sourceProps;
    OfxPropertySetHandle outputProps;
    _effectSuite->clipDefine(handle, "Source", &sourceProps);
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
            sourceProps,
            kOfxImageEffectPropSupportedComponents,
            i,
            components[i].c_str());
        _propSuite->propSetString(
            outputProps,
            kOfxImageEffectPropSupportedComponents,
            i,
            components[i].c_str());
    }
    for (int i = 0; i < pixelDepths.size(); ++i)
    {
        _propSuite->propSetString(
            sourceProps,
            kOfxImageEffectPropSupportedPixelDepths,
            i,
            pixelDepths[i].c_str());
        _propSuite->propSetString(
            outputProps,
            kOfxImageEffectPropSupportedPixelDepths,
            i,
            pixelDepths[i].c_str());
    }
    
    return kOfxStatOK;
}

OfxStatus FilterPlugin::_renderAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    OfxTime time;
    OfxRectI renderWindow;
    _propSuite->propGetDouble(inArgs, kOfxPropTime, 0, &time);
    _propSuite->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

    OfxImageClipHandle sourceClip = nullptr;
    OfxImageClipHandle outputClip = nullptr;
    OfxPropertySetHandle sourceImage = nullptr;
    OfxPropertySetHandle outputImage = nullptr;
    _effectSuite->clipGetHandle(handle, "Source", &sourceClip, nullptr);
    _effectSuite->clipGetHandle(handle, "Output", &outputClip, nullptr);
    if (sourceClip && outputClip)
    {
        _effectSuite->clipGetImage(sourceClip, time, nullptr, &sourceImage);
        _effectSuite->clipGetImage(outputClip, time, nullptr, &outputImage);
        if (sourceImage && outputImage)
        {
            const OIIO::ImageBuf sourceBuf = propSetToBuf(_propSuite, sourceImage);
            OIIO::ImageBuf outputBuf = propSetToBuf(_propSuite, outputImage);
            _render(handle, sourceBuf, outputBuf, renderWindow, inArgs);
        }
    }

    if (sourceImage)
    {
        _effectSuite->clipReleaseImage(sourceImage);
    }
    if (outputImage)
    {
        _effectSuite->clipReleaseImage(outputImage);
    }

    return kOfxStatOK;
}

BlurPlugin* BlurPlugin::_plugin = nullptr;

BlurPlugin::BlurPlugin() :
    FilterPlugin("toucan", "Blur")
{}

BlurPlugin::~BlurPlugin()
{}

void BlurPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new BlurPlugin;
    }
    _plugin->_host = host;
}

OfxStatus BlurPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus BlurPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    FilterPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "radius", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 10.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Radius");

    return kOfxStatOK;
}

OfxStatus BlurPlugin::_createInstance(OfxImageEffectHandle handle)
{
    FilterPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "radius", &_radiusParam[handle], nullptr);
    
    return kOfxStatOK;
}

OfxStatus BlurPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double radius = 0.0;
    _paramSuite->paramGetValue(_radiusParam[handle], &radius);

    const OIIO::ImageBuf k = OIIO::ImageBufAlgo::make_kernel(
        "gaussian",
        radius,
        radius);

    OIIO::ImageBufAlgo::convolve(
        outputBuf,
        sourceBuf,
        k,
        true,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));

    return kOfxStatOK;
}

ColorMapPlugin* ColorMapPlugin::_plugin = nullptr;

ColorMapPlugin::ColorMapPlugin() :
    FilterPlugin("toucan", "ColorMap")
{}

ColorMapPlugin::~ColorMapPlugin()
{}

void ColorMapPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new ColorMapPlugin;
    }
    _plugin->_host = host;
}

OfxStatus ColorMapPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus ColorMapPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    FilterPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "map_name", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "plasma");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Map Name");

    return kOfxStatOK;
}

OfxStatus ColorMapPlugin::_createInstance(OfxImageEffectHandle handle)
{
    FilterPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "map_name", &_mapNameParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus ColorMapPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    // Apply the color map.
    std::string mapName = "plasma";
    _paramSuite->paramGetValue(_mapNameParam[handle], &mapName);
    OIIO::ImageBufAlgo::color_map(
        outputBuf,
        sourceBuf,
        -1,
        mapName,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));

    // Copy the alpha channel.
    OIIO::ImageBufAlgo::copy(
        outputBuf,
        sourceBuf,
        OIIO::TypeUnknown,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2,
            0,
            1,
            3,
            4));

    return kOfxStatOK;
}

InvertPlugin* InvertPlugin::_plugin = nullptr;

InvertPlugin::InvertPlugin() :
    FilterPlugin("toucan", "Invert")
{}

InvertPlugin::~InvertPlugin()
{}

void InvertPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new InvertPlugin;
    }
    _plugin->_host = host;
}

OfxStatus InvertPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus InvertPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    // Invert the color channels.
    OIIO::ImageBufAlgo::invert(
        outputBuf,
        sourceBuf,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2,
            0,
            1,
            0,
            3));

    // Copy the alpha channel.
    OIIO::ImageBufAlgo::copy(
        outputBuf,
        sourceBuf,
        OIIO::TypeUnknown,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2,
            0,
            1,
            3,
            4));

    return kOfxStatOK;
}

PowPlugin* PowPlugin::_plugin = nullptr;

PowPlugin::PowPlugin() :
    FilterPlugin("toucan", "Pow")
{}

PowPlugin::~PowPlugin()
{}

void PowPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new PowPlugin;
    }
    _plugin->_host = host;
}

OfxStatus PowPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus PowPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    FilterPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "value", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Value");

    return kOfxStatOK;
}

OfxStatus PowPlugin::_createInstance(OfxImageEffectHandle handle)
{
    FilterPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "value", &_valueParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus PowPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double value = 1.0;
    _paramSuite->paramGetValue(_valueParam[handle], &value);

    OIIO::ImageBufAlgo::pow(
        outputBuf,
        sourceBuf,
        value,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));

    return kOfxStatOK;
}

SaturatePlugin* SaturatePlugin::_plugin = nullptr;

SaturatePlugin::SaturatePlugin() :
    FilterPlugin("toucan", "Saturate")
{}

SaturatePlugin::~SaturatePlugin()
{}

void SaturatePlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new SaturatePlugin;
    }
    _plugin->_host = host;
}

OfxStatus SaturatePlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus SaturatePlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    FilterPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "value", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Value");

    return kOfxStatOK;
}

OfxStatus SaturatePlugin::_createInstance(OfxImageEffectHandle handle)
{
    FilterPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "value", &_valueParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus SaturatePlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double value = 1.0;
    _paramSuite->paramGetValue(_valueParam[handle], &value);

    OIIO::ImageBufAlgo::saturate(
        outputBuf,
        sourceBuf,
        value,
        0,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));

    return kOfxStatOK;
}

UnsharpMaskPlugin* UnsharpMaskPlugin::_plugin = nullptr;

UnsharpMaskPlugin::UnsharpMaskPlugin() :
    FilterPlugin("toucan", "UnsharpMask")
{}

UnsharpMaskPlugin::~UnsharpMaskPlugin()
{}

void UnsharpMaskPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new UnsharpMaskPlugin;
    }
    _plugin->_host = host;
}

OfxStatus UnsharpMaskPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus UnsharpMaskPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    FilterPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "kernel", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "gaussian");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Kernel");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "width", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Width");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "contrast", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Contrast");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "threshold", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Threshold");

    return kOfxStatOK;
}

OfxStatus UnsharpMaskPlugin::_createInstance(OfxImageEffectHandle handle)
{
    FilterPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "kernel", &_kernelParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "width", &_widthParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "contrast", &_contrastParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "threshold", &_thresholdParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus UnsharpMaskPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    std::string kernel = "gaussian";
    double width = 3.0;
    double contrast = 1.0;
    double threshold = 0.0;
    _paramSuite->paramGetValue(_kernelParam[handle], &kernel);
    _paramSuite->paramGetValue(_widthParam[handle], &width);
    _paramSuite->paramGetValue(_contrastParam[handle], &contrast);
    _paramSuite->paramGetValue(_thresholdParam[handle], &threshold);

    //! \bug The unsharp_mask() function does not seem to be working?
    OIIO::ImageBufAlgo::unsharp_mask(
        outputBuf,
        sourceBuf,
        kernel,
        width,
        contrast,
        threshold,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));

    return kOfxStatOK;
}

namespace
{
    std::vector<OfxPlugin> plugins =
    {
        { kOfxImageEffectPluginApi, 1, "toucan:Blur", 1, 0, BlurPlugin::setHostFunc, BlurPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:ColorMap", 1, 0, ColorMapPlugin::setHostFunc, ColorMapPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Invert", 1, 0, InvertPlugin::setHostFunc, InvertPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Pow", 1, 0, PowPlugin::setHostFunc, PowPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Saturate", 1, 0, SaturatePlugin::setHostFunc, SaturatePlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:UnsharpMask", 1, 0, UnsharpMaskPlugin::setHostFunc, UnsharpMaskPlugin::mainEntryPoint }
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
