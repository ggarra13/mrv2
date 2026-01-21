// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "ColorPlugin.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

ColorPlugin::ColorPlugin(const std::string& group, const std::string& name) :
    Plugin(group, name)
{}

ColorPlugin::~ColorPlugin()
{}

OfxStatus ColorPlugin::_describeAction(OfxImageEffectHandle handle)
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

OfxStatus ColorPlugin::_describeInContextAction(OfxImageEffectHandle handle, OfxPropertySetHandle inArgs)
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

OfxStatus ColorPlugin::_renderAction(
    OfxImageEffectHandle handle ,
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

ColorConvertPlugin* ColorConvertPlugin::_plugin = nullptr;

ColorConvertPlugin::ColorConvertPlugin() :
    ColorPlugin("toucan", "ColorConvert")
{}

ColorConvertPlugin::~ColorConvertPlugin()
{}

void ColorConvertPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new ColorConvertPlugin;
    }
    _plugin->_host = host;
}

OfxStatus ColorConvertPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus ColorConvertPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    ColorPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "fromspace", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "From Space");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "tospace", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "To Space");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeBoolean, "premult", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, true);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Premultiplied");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "context_key", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Context Key");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "context_value", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Context Value");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "color_config", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "ColorConfig");

    return kOfxStatOK;
}

OfxStatus ColorConvertPlugin::_createInstance(OfxImageEffectHandle handle)
{
    ColorPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "fromspace", &_fromSpaceParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "tospace", &_toSpaceParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "premult", &_premultParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "context_key", &_contextKeyParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "context_value", &_contextValueParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color_config", &_colorConfigParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus ColorConvertPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    std::string fromSpace;
    std::string toSpace;
    int premult = 0;
    std::string contextKey;
    std::string contextValue;
    std::string colorConfigValue;
    _paramSuite->paramGetValue(_fromSpaceParam[handle], &fromSpace);
    _paramSuite->paramGetValue(_toSpaceParam[handle], &toSpace);
    _paramSuite->paramGetValue(_premultParam[handle], &premult);
    _paramSuite->paramGetValue(_contextKeyParam[handle], &contextKey);
    _paramSuite->paramGetValue(_contextValueParam[handle], &contextValue);
    _paramSuite->paramGetValue(_colorConfigParam[handle], &colorConfigValue);

    const std::string colorConfigPath(colorConfigValue);
    std::shared_ptr<OIIO::ColorConfig> colorConfig;
    auto i = _colorConfigs.find(colorConfigPath);
    if (i != _colorConfigs.end())
    {
        colorConfig = i->second;
    }
    else
    {
        colorConfig = std::make_shared<OIIO::ColorConfig>(colorConfigPath);
        _colorConfigs[colorConfigPath] = colorConfig;
        //for (int i = 0; i < colorConfig->getNumColorSpaces(); ++i)
        //{
        //    std::cout << "Color space: " << colorConfig->getColorSpaceNameByIndex(i) << std::endl;
        //}
    }

    if (colorConfig)
    {
        OIIO::ImageBufAlgo::colorconvert(
            outputBuf,
            sourceBuf,
            fromSpace,
            toSpace,
            premult,
            contextKey,
            contextValue,
            colorConfig.get());
    }
    return kOfxStatOK;
}

PremultPlugin* PremultPlugin::_plugin = nullptr;

PremultPlugin::PremultPlugin() :
    ColorPlugin("toucan", "Premult")
{}

PremultPlugin::~PremultPlugin()
{}

void PremultPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new PremultPlugin;
    }
    _plugin->_host = host;
}

OfxStatus PremultPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus PremultPlugin::_render(
    OfxImageEffectHandle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    OIIO::ImageBufAlgo::premult(outputBuf, sourceBuf);
    return kOfxStatOK;
}

UnpremultPlugin* UnpremultPlugin::_plugin = nullptr;

UnpremultPlugin::UnpremultPlugin() :
    ColorPlugin("toucan", "Unpremult")
{}

UnpremultPlugin::~UnpremultPlugin()
{}

void UnpremultPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new UnpremultPlugin;
    }
    _plugin->_host = host;
}

OfxStatus UnpremultPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus UnpremultPlugin::_render(
    OfxImageEffectHandle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    OIIO::ImageBufAlgo::unpremult(outputBuf, sourceBuf);
    return kOfxStatOK;
}

namespace
{
    std::vector<OfxPlugin> plugins =
    {
        { kOfxImageEffectPluginApi, 1, "toucan:ColorConvert", 1, 0, ColorConvertPlugin::setHostFunc, ColorConvertPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Premult", 1, 0, PremultPlugin::setHostFunc, PremultPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Unpremult", 1, 0, UnpremultPlugin::setHostFunc, UnpremultPlugin::mainEntryPoint }
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
