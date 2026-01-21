// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include "Plugin.h"

#include <OpenImageIO/imagebuf.h>

class TransformPlugin : public Plugin
{
public:
    TransformPlugin(const std::string& group, const std::string& name);

    virtual ~TransformPlugin() = 0;

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

class CropPlugin : public TransformPlugin
{
public:
    CropPlugin();

    virtual ~CropPlugin();

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
    static CropPlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _posParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _sizeParam;
};

class FlipPlugin : public TransformPlugin
{
public:
    FlipPlugin();

    virtual ~FlipPlugin();

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
    static FlipPlugin* _plugin;
};

class FlopPlugin : public TransformPlugin
{
public:
    FlopPlugin();

    virtual ~FlopPlugin();

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
    static FlopPlugin* _plugin;
};

class ResizePlugin : public TransformPlugin
{
public:
    ResizePlugin();

    virtual ~ResizePlugin();

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
    static ResizePlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _sizeParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _filterNameParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _filterWidthParam;
};

class RotatePlugin : public TransformPlugin
{
public:
    RotatePlugin();

    virtual ~RotatePlugin();

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
    static RotatePlugin* _plugin;
    std::map<OfxImageEffectHandle, OfxParamHandle> _angleParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _filterNameParam;
    std::map<OfxImageEffectHandle, OfxParamHandle> _filterWidthParam;
};
