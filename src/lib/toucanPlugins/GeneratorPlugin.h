// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "Plugin.h"

#include <OpenImageIO/imagebuf.h>

class GeneratorPlugin : public Plugin
{
public:
    GeneratorPlugin(const std::string& group, const std::string& name);

    virtual ~GeneratorPlugin() = 0;

protected:
    virtual OfxStatus _render(
        OfxImageEffectHandle,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) = 0;

    OfxStatus _describeAction(OfxImageEffectHandle) override;
    OfxStatus _describeInContextAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle) override;
    OfxStatus _createInstance(OfxImageEffectHandle) override;
    OfxStatus _renderAction(
        OfxImageEffectHandle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs) override;

private:
    std::map<OfxImageEffectHandle, OfxParamHandle> _sizeParam;
};

class CheckersPlugin : public GeneratorPlugin
{
public:
    CheckersPlugin();

    virtual ~CheckersPlugin();

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
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static CheckersPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _checkerSizeParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _color1Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _color2Param;
};

class FillPlugin : public GeneratorPlugin
{
public:
    FillPlugin();

    virtual ~FillPlugin();

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
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static FillPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _colorParam;
};

class GradientPlugin : public GeneratorPlugin
{
public:
    GradientPlugin();

    virtual ~GradientPlugin();

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
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static GradientPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _color1Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _color2Param;
    std::map<OfxImageEffectHandle, OfxParamHandle> _verticalParam;
};

class NoisePlugin : public GeneratorPlugin
{
public:
    NoisePlugin();

    virtual ~NoisePlugin();

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
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static NoisePlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _typeParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _aParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _bParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _monoParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _seedParam;
};
