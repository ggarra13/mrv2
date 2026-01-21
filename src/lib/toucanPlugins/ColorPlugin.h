// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "Plugin.h"

#include <OpenImageIO/color.h>
#include <OpenImageIO/imagebuf.h>

class ColorPlugin : public Plugin
{
public:
    ColorPlugin(const std::string& group, const std::string& name);

    virtual ~ColorPlugin() = 0;

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

class ColorConvertPlugin : public ColorPlugin
{
public:
    ColorConvertPlugin();

    virtual ~ColorConvertPlugin();

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
    static ColorConvertPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _fromSpaceParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _toSpaceParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _premultParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _contextKeyParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _contextValueParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _colorConfigParam;
    std::map<std::string, std::shared_ptr<OIIO::ColorConfig> > _colorConfigs;
};

class PremultPlugin : public ColorPlugin
{
public:
    PremultPlugin();

    virtual ~PremultPlugin();

    static void setHostFunc(OfxHost*);

    static OfxStatus mainEntryPoint(
        const char* action,
        const void* handle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs);

protected:
    OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static PremultPlugin* _plugin;
};

class UnpremultPlugin : public ColorPlugin
{
public:
    UnpremultPlugin();

    virtual ~UnpremultPlugin();

    static void setHostFunc(OfxHost*);

    static OfxStatus mainEntryPoint(
        const char* action,
        const void* handle,
        OfxPropertySetHandle inArgs,
        OfxPropertySetHandle outArgs);

protected:
    OfxStatus _render(
        OfxImageEffectHandle,
        const OIIO::ImageBuf&,
        OIIO::ImageBuf&,
        const OfxRectI& renderWindow,
        OfxPropertySetHandle inArgs) override;

private:
    static UnpremultPlugin* _plugin;
};
