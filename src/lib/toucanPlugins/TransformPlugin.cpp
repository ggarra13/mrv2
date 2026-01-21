// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "TransformPlugin.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <Imath/ImathVec.h>

TransformPlugin::TransformPlugin(const std::string& group, const std::string& name) :
    Plugin(group, name)
{}

TransformPlugin::~TransformPlugin()
{}

OfxStatus TransformPlugin::_describeAction(OfxImageEffectHandle handle)
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

OfxStatus TransformPlugin::_describeInContextAction(OfxImageEffectHandle handle, OfxPropertySetHandle inArgs)
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

OfxStatus TransformPlugin::_renderAction(
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

CropPlugin* CropPlugin::_plugin = nullptr;

CropPlugin::CropPlugin() :
    TransformPlugin("toucan", "Crop")
{}

CropPlugin::~CropPlugin()
{}

void CropPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new CropPlugin;
    }
    _plugin->_host = host;
}

OfxStatus CropPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus CropPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    TransformPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Position");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "size", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Size");

    return kOfxStatOK;
}

OfxStatus CropPlugin::_createInstance(OfxImageEffectHandle handle)
{
    TransformPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "pos", &_posParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "size", &_sizeParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus CropPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t pos[2] = { 0, 0 };
    int64_t size[2] = { 0, 0 };
    _paramSuite->paramGetValue(_posParam[handle], &pos[0], &pos[1]);
    _paramSuite->paramGetValue(_sizeParam[handle], &size[0], &size[1]);

    const auto crop = OIIO::ImageBufAlgo::cut(
        sourceBuf,
        OIIO::ROI(pos[0], pos[0] + size[0], pos[1], pos[1] + size[1]));
    OIIO::ImageBufAlgo::copy(outputBuf, crop);

    return kOfxStatOK;
}

FlipPlugin* FlipPlugin::_plugin = nullptr;

FlipPlugin::FlipPlugin() :
    TransformPlugin("toucan", "Flip")
{}

FlipPlugin::~FlipPlugin()
{}

void FlipPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new FlipPlugin;
    }
    _plugin->_host = host;
}

OfxStatus FlipPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus FlipPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    OIIO::ImageBufAlgo::flip(
        outputBuf,
        sourceBuf,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));
    return kOfxStatOK;
}

FlopPlugin* FlopPlugin::_plugin = nullptr;

FlopPlugin::FlopPlugin() :
    TransformPlugin("toucan", "Flop")
{}

FlopPlugin::~FlopPlugin()
{}

void FlopPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new FlopPlugin;
    }
    _plugin->_host = host;
}

OfxStatus FlopPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus FlopPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    OIIO::ImageBufAlgo::flop(
        outputBuf,
        sourceBuf,
        OIIO::ROI(
            renderWindow.x1,
            renderWindow.x2,
            renderWindow.y1,
            renderWindow.y2));
    return kOfxStatOK;
}

ResizePlugin* ResizePlugin::_plugin = nullptr;

ResizePlugin::ResizePlugin() :
    TransformPlugin("toucan", "Resize")
{}

ResizePlugin::~ResizePlugin()
{}

void ResizePlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new ResizePlugin;
    }
    _plugin->_host = host;
}

OfxStatus ResizePlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus ResizePlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    TransformPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "size", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Size");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "filter_name", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Filter name");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "filter_width", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Filter width");

    return kOfxStatOK;
}

OfxStatus ResizePlugin::_createInstance(OfxImageEffectHandle handle)
{
    TransformPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "size", &_sizeParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "filter_name", &_filterNameParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "filter_width", &_filterWidthParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus ResizePlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t size[2] = { 0, 0 };
    std::string filterName;
    double filterWidth = 0.0;
    _paramSuite->paramGetValue(_sizeParam[handle], &size[0], &size[1]);
    _paramSuite->paramGetValue(_filterNameParam[handle], &filterName);
    _paramSuite->paramGetValue(_filterWidthParam[handle], &filterWidth);

    OIIO::ImageBufAlgo::resize(
        outputBuf,
        sourceBuf,
        filterName,
        filterWidth,
        OIIO::ROI(0, size[0], 0, size[1]));

    return kOfxStatOK;
}

RotatePlugin* RotatePlugin::_plugin = nullptr;

RotatePlugin::RotatePlugin() :
    TransformPlugin("toucan", "Rotate")
{}

RotatePlugin::~RotatePlugin()
{}

void RotatePlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new RotatePlugin;
    }
    _plugin->_host = host;
}

OfxStatus RotatePlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus RotatePlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    TransformPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "angle", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Angle");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "filter_name", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Filter name");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeDouble, "filter_width", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 0.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Filter width");

    return kOfxStatOK;
}

OfxStatus RotatePlugin::_createInstance(OfxImageEffectHandle handle)
{
    TransformPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "angle", &_angleParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "filter_name", &_filterNameParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "filter_width", &_filterWidthParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus RotatePlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    double angle = 0.0;
    std::string filterName;
    double filterWidth = 0.0;
    _paramSuite->paramGetValue(_angleParam[handle], &angle);
    _paramSuite->paramGetValue(_filterNameParam[handle], &filterName);
    _paramSuite->paramGetValue(_filterWidthParam[handle], &filterWidth);

    OIIO::ImageBufAlgo::rotate(
        outputBuf,
        sourceBuf,
        angle / 360.F * 2.F * M_PI,
        filterName,
        filterWidth);

    return kOfxStatOK;
}

namespace
{
    std::vector<OfxPlugin> plugins =
    {
        { kOfxImageEffectPluginApi, 1, "toucan:Crop", 1, 0, CropPlugin::setHostFunc, CropPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Flip", 1, 0, FlipPlugin::setHostFunc, FlipPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Flop", 1, 0, FlopPlugin::setHostFunc, FlopPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Resize", 1, 0, ResizePlugin::setHostFunc, ResizePlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Rotate", 1, 0, RotatePlugin::setHostFunc, RotatePlugin::mainEntryPoint }
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
