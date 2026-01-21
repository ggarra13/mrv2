// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "Plugin.h"

#include <OpenImageIO/imagebuf.h>

class DrawPlugin : public Plugin
{
public:
    DrawPlugin(const std::string& group, const std::string& name);

    virtual ~DrawPlugin() = 0;

protected:
    virtual OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) = 0;

    OfxStatus _describeAction(OfxImageEffectHandle) override;
    OfxStatus _describeInContextAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle) override;
    OfxStatus _renderAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs) override;
};

class BoxPlugin : public DrawPlugin
{
public:
    BoxPlugin();

    virtual ~BoxPlugin();

    static void setHostFunc(OfxHost*);

    static OfxStatus mainEntryPoint(
        const char* action,
        const void* handle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs);

protected:
    OfxStatus _describeInContextAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle) override;
    OfxStatus _createInstance(OfxImageEffectHandle) override;
    OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static BoxPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _pos1Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _pos2Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _colorParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _fillParam;
};

class LinePlugin : public DrawPlugin
{
public:
    LinePlugin();

    virtual ~LinePlugin();

    static void setHostFunc(OfxHost*);

    static OfxStatus mainEntryPoint(
        const char* action,
        const void* handle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs);

protected:
    OfxStatus _describeInContextAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle) override;
    OfxStatus _createInstance(OfxImageEffectHandle) override;
    OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static LinePlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _pos1Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _pos2Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _colorParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _skipFirstPointParam;
};

class TextPlugin : public DrawPlugin
{
public:
    TextPlugin();

    virtual ~TextPlugin();

    static void setHostFunc(OfxHost*);

    static OfxStatus mainEntryPoint(
        const char* action,
        const void* handle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs);

protected:
    OfxStatus _describeInContextAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle) override;
    OfxStatus _createInstance(OfxImageEffectHandle) override;
    OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static TextPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _posParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _textParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _fontSizeParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _fontNameParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _colorParam;
};
