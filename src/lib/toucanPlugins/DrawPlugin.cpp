// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "DrawPlugin.h"

#include "Util.h"

#include <OpenImageIO/imagebufalgo.h>

#include <Imath/ImathVec.h>

DrawPlugin::DrawPlugin(const std::string& group, const std::string& name) :
    Plugin(group, name)
{}

DrawPlugin::~DrawPlugin()
{}

OfxStatus DrawPlugin::_describeAction(OfxImageEffectHandle handle)
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

OfxStatus DrawPlugin::_describeInContextAction(OfxImageEffectHandle handle, OfxPropertySetHandle inArgs)
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

OfxStatus DrawPlugin::_renderAction(
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

BoxPlugin* BoxPlugin::_plugin = nullptr;

BoxPlugin::BoxPlugin() :
    DrawPlugin("toucan", "Box")
{}

BoxPlugin::~BoxPlugin()
{}

void BoxPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new BoxPlugin;
    }
    _plugin->_host = host;
}

OfxStatus BoxPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus BoxPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    DrawPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos1", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Upper left corner");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos2", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Lower right corner");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeBoolean, "fill", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, true);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Fill the box");

    return kOfxStatOK;
}

OfxStatus BoxPlugin::_createInstance(OfxImageEffectHandle handle)
{
    DrawPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "pos1", &_pos1Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "pos2", &_pos2Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color", &_colorParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "fill", &_fillParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus BoxPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t pos1[2] = { 0, 0 };
    int64_t pos2[2] = { 0, 0 };
    double color[4] = { 0.0, 0.0, 0.0, 0.0 };
    int fill = 0;
    _paramSuite->paramGetValue(_pos1Param[handle], &pos1[0], &pos1[1]);
    _paramSuite->paramGetValue(_pos2Param[handle], &pos2[0], &pos2[1]);
    _paramSuite->paramGetValue(_colorParam[handle], &color[0], &color[1], &color[2], &color[3]);
    _paramSuite->paramGetValue(_fillParam[handle], &fill);

    OIIO::ImageBufAlgo::copy(outputBuf, sourceBuf);
    OIIO::ImageBufAlgo::render_box(
        outputBuf,
        pos1[0],
        pos1[1],
        pos2[0],
        pos2[1],
        {
            static_cast<float>(color[0]),
            static_cast<float>(color[1]),
            static_cast<float>(color[2]),
            static_cast<float>(color[3])
        },
        fill);

    return kOfxStatOK;
}

LinePlugin* LinePlugin::_plugin = nullptr;

LinePlugin::LinePlugin() :
    DrawPlugin("toucan", "Line")
{}

LinePlugin::~LinePlugin()
{}

void LinePlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new LinePlugin;
    }
    _plugin->_host = host;
}

OfxStatus LinePlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus LinePlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    DrawPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos1", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Start position");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos2", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "End position");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeBoolean, "skip_first_point", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, false);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Skip the first point");

    return kOfxStatOK;
}

OfxStatus LinePlugin::_createInstance(OfxImageEffectHandle handle)
{
    DrawPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "pos1", &_pos1Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "pos2", &_pos2Param[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color", &_colorParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "skip_first_point", &_skipFirstPointParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus LinePlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t pos1[2] = { 0, 0 };
    int64_t pos2[2] = { 0, 0 };
    double color[4] = { 0.0, 0.0, 0.0, 0.0 };
    int skipFirstPoint = 0;
    _paramSuite->paramGetValue(_pos1Param[handle], &pos1[0], &pos1[1]);
    _paramSuite->paramGetValue(_pos2Param[handle], &pos2[0], &pos2[1]);
    _paramSuite->paramGetValue(_colorParam[handle], &color[0], &color[1], &color[2], &color[3]);
    _paramSuite->paramGetValue(_skipFirstPointParam[handle], &skipFirstPoint);

    OIIO::ImageBufAlgo::copy(outputBuf, sourceBuf);
    OIIO::ImageBufAlgo::render_line(
        outputBuf,
        pos1[0],
        pos1[1],
        pos2[0],
        pos2[1],
        {
            static_cast<float>(color[0]),
            static_cast<float>(color[1]),
            static_cast<float>(color[2]),
            static_cast<float>(color[3])
        },
        skipFirstPoint);

    return kOfxStatOK;
}

TextPlugin* TextPlugin::_plugin = nullptr;

TextPlugin::TextPlugin() :
    DrawPlugin("toucan", "Text")
{}

TextPlugin::~TextPlugin()
{}

void TextPlugin::setHostFunc(OfxHost* host)
{
    if (!_plugin)
    {
        _plugin = new TextPlugin;
    }
    _plugin->_host = host;
}

OfxStatus TextPlugin::mainEntryPoint(
    const char* action,
    const void* handle,
    OfxPropertySetHandle inArgs,
    OfxPropertySetHandle outArgs)
{
    return _plugin->_entryPoint(action, handle, inArgs, outArgs);
}

OfxStatus TextPlugin::_describeInContextAction(
    OfxImageEffectHandle handle,
    OfxPropertySetHandle inArgs)
{
    DrawPlugin::_describeInContextAction(handle, inArgs);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    OfxPropertySetHandle props;
    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger2D, "pos", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 0);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 1, 0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Position");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "text", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Text");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeInteger, "font_size", &props);
    _propSuite->propSetInt(props, kOfxParamPropDefault, 0, 16);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Font size");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeString, "font_name", &props);
    _propSuite->propSetString(props, kOfxParamPropDefault, 0, "");
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Font name");

    _paramSuite->paramDefine(paramSet, kOfxParamTypeRGBA, "color", &props);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 0, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 1, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 2, 1.0);
    _propSuite->propSetDouble(props, kOfxParamPropDefault, 3, 1.0);
    _propSuite->propSetString(props, kOfxPropLabel, 0, "Color");

    return kOfxStatOK;
}

OfxStatus TextPlugin::_createInstance(OfxImageEffectHandle handle)
{
    DrawPlugin::_createInstance(handle);

    OfxParamSetHandle paramSet;
    _effectSuite->getParamSet(handle, &paramSet);
    _paramSuite->paramGetHandle(paramSet, "pos", &_posParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "text", &_textParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "font_size", &_fontSizeParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "font_name", &_fontNameParam[handle], nullptr);
    _paramSuite->paramGetHandle(paramSet, "color", &_colorParam[handle], nullptr);

    return kOfxStatOK;
}

OfxStatus TextPlugin::_render(
    OfxImageEffectHandle handle,
    const OIIO::ImageBuf& sourceBuf,
    OIIO::ImageBuf& outputBuf,
    const OfxRectI& renderWindow,
    OfxPropertySetHandle inArgs)
{
    int64_t pos[2] = { 0, 0 };
    std::string text;
    int64_t fontSize = 16;
    std::string fontName;
    double color[4] = { 0.0, 0.0, 0.0, 0.0 };
    _paramSuite->paramGetValue(_posParam[handle], &pos[0], &pos[1]);
    _paramSuite->paramGetValue(_textParam[handle], &text);
    _paramSuite->paramGetValue(_fontSizeParam[handle], &fontSize);
    _paramSuite->paramGetValue(_fontNameParam[handle], &fontName);
    _paramSuite->paramGetValue(_colorParam[handle], &color[0], &color[1], &color[2], &color[3]);

    OIIO::ImageBufAlgo::copy(outputBuf, sourceBuf);
    OIIO::ImageBufAlgo::render_text(
        outputBuf,
        pos[0],
        pos[1],
        text,
        fontSize,
        fontName,
        {
            static_cast<float>(color[0]),
            static_cast<float>(color[1]),
            static_cast<float>(color[2]),
            static_cast<float>(color[3])
        });

    return kOfxStatOK;
}

namespace
{
    std::vector<OfxPlugin> plugins =
    {
        { kOfxImageEffectPluginApi, 1, "toucan:Box", 1, 0, BoxPlugin::setHostFunc, BoxPlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Line", 1, 0, LinePlugin::setHostFunc, LinePlugin::mainEntryPoint },
        { kOfxImageEffectPluginApi, 1, "toucan:Text", 1, 0, TextPlugin::setHostFunc, TextPlugin::mainEntryPoint }
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
