// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlCore/StringFormat.h>

// -----------------------------------------------------------------------------
//  WGSL versions of the renderer's shaders.
//
//  Binding rules (they must match the order of the declarations in
//  Render::_drawVideo's shader setup, because tl::wgpu::Shader numbers the
//  bindings of @group(0) in declaration order, and a texture takes TWO
//  bindings: the texture at N and its sampler at N + 1):
//
//    * The push block (PushConstants: mvp + color) is ALWAYS @binding(0).
//      Declare it FIRST: addPush() before any addTexture()/createUniform().
//
//    shader                    bindings
//    ------------------------  ----------------------------------------------
//    rect/wipe, mesh,          0 push
//    colorMesh, hard, soft
//    text, texture             0 push, 1/2 textureSampler
//    butterfly, difference,    0 push, 1/2 textureSampler,
//    multiply, add                     3/4 textureSamplerB
//    image                     0 push, 1 ubo, 2/3 textureSampler0,
//                              4/5 textureSampler1, 6/7 textureSampler2
//    display                   0 transform (vertex), 1/2 textureSampler,
//                              3 uboLevels, 4 uboNormalize, 5 uboColor,
//                              6 ubo
//
//  Every vertex/fragment source is its own module with the entry point
//  "main" (what Shader's two-source constructor expects).
//
//  OCIO and libplacebo generate GLSL, which cannot be used here, so they are
//  a no-op: displayFragmentSource() ignores those arguments.
//
//  textureSampleLevel() is used instead of textureSample() everywhere: it has
//  no derivative-uniformity requirement (the old GLSL sampled inside
//  non-uniform branches, which WGSL rejects) and none of the textures have
//  mipmaps anyway.
// -----------------------------------------------------------------------------

namespace tl
{
    namespace timeline_wgpu
    {
        // Guard the WGSL struct layouts below against changes to the host
        // structs in RenderStructs.h.
        static_assert(sizeof(PushConstants) == 80, "WGSL PushConstants");
        static_assert(sizeof(UBOTexture) == 64, "WGSL UBO (image)");
        static_assert(sizeof(UBOLevels) == 24, "WGSL Levels");
        static_assert(sizeof(UBONormalize) == 48, "WGSL Normalize");
        static_assert(sizeof(UBOColor) == 112, "WGSL Color");
        static_assert(sizeof(UBOOptions) == 28, "WGSL UBO (display)");

        namespace
        {
            // The push block, @binding(0) in every shader that has one.
            const char* const kPushBlock = R"WGSL(
struct PushConstants
{
    mvp   : mat4x4<f32>,
    color : vec4<f32>,
};
@group(0) @binding(0) var<uniform> pc : PushConstants;
)WGSL";
        } // namespace

        // ---------------------------------------------------------------
        //  Vertex shaders
        // ---------------------------------------------------------------

        std::string vertexSTs()
        {
            return std::string(kPushBlock) + R"WGSL(
struct VSOut
{
    @builtin(position) position : vec4<f32>,
    @location(0) fTexture : vec2<f32>,
    @location(1) fragPosition : vec3<f32>,
};

@vertex
fn main(@location(0) vPos : vec3<f32>,
        @location(1) vTexture : vec2<f32>) -> VSOut
{
    var r : VSOut;
    r.fragPosition = vPos;
    r.fTexture = vTexture;
    r.position = pc.mvp * vec4<f32>(vPos, 1.0);
    return r;
}
)WGSL";
        }

        std::string vertexSource()
        {
            return std::string(kPushBlock) + R"WGSL(
struct VSOut
{
    @builtin(position) position : vec4<f32>,
    @location(0) fTexture : vec2<f32>,
    @location(1) fPos : vec3<f32>,
};

@vertex
fn main(@location(0) vPos : vec3<f32>,
        @location(1) vTexture : vec2<f32>) -> VSOut
{
    var r : VSOut;
    r.position = pc.mvp * vec4<f32>(vPos, 1.0);
    r.fTexture = vTexture;
    r.fPos = vPos;
    return r;
}
)WGSL";
        }

        std::string vertex2Source()
        {
            return std::string(kPushBlock) + R"WGSL(
struct VSOut
{
    @builtin(position) position : vec4<f32>,
    @location(0) fTexture : vec2<f32>,
};

@vertex
fn main(@location(0) vPos : vec2<f32>,
        @location(1) vTexture : vec2<f32>) -> VSOut
{
    var r : VSOut;
    r.position = pc.mvp * vec4<f32>(vPos, 0.0, 1.0);
    r.fTexture = vTexture;
    return r;
}
)WGSL";
        }

        std::string vertex2NoUVsSource()
        {
            return std::string(kPushBlock) + R"WGSL(
@vertex
fn main(@location(0) vPos : vec2<f32>) -> @builtin(position) vec4<f32>
{
    return pc.mvp * vec4<f32>(vPos, 0.0, 1.0);
}
)WGSL";
        }

        std::string colorMeshVertexSource()
        {
            return std::string(kPushBlock) + R"WGSL(
struct VSOut
{
    @builtin(position) position : vec4<f32>,
    @location(0) fColor : vec4<f32>,
};

@vertex
fn main(@location(0) vPos : vec2<f32>,
        @location(1) vColor : vec4<f32>) -> VSOut
{
    var r : VSOut;
    r.position = pc.mvp * vec4<f32>(vPos, 0.0, 1.0);
    r.fColor = vColor;
    return r;
}
)WGSL";
        }

        // ---------------------------------------------------------------
        //  Simple fragment shaders (push block only)
        // ---------------------------------------------------------------

        std::string fragmentSTs()
        {
            return R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    return vec4<f32>(fTexture.x, fTexture.y, 0.0, 1.0);
}
)WGSL";
        }

        std::string meshFragmentSource()
        {
            // No inputs: it is also used with vertex2NoUVsSource(), which
            // has no outputs besides the position.
            return std::string(kPushBlock) + R"WGSL(
@fragment
fn main() -> @location(0) vec4<f32>
{
    return pc.color;
}
)WGSL";
        }

        std::string colorMeshFragmentSource()
        {
            return std::string(kPushBlock) + R"WGSL(
@fragment
fn main(@location(0) fColor : vec4<f32>) -> @location(0) vec4<f32>
{
    return fColor * pc.color;
}
)WGSL";
        }

        std::string hardFragmentSource()
        {
            return std::string(kPushBlock) + R"WGSL(
@fragment
fn main() -> @location(0) vec4<f32>
{
    return pc.color;
}
)WGSL";
        }

        std::string softFragmentSource()
        {
            return std::string(kPushBlock) + R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let v = fTexture - vec2<f32>(0.5, 0.5);
    let ratio = 1.0 - sqrt(v.x * v.x + v.y * v.y);
    let radius = 0.75;
    let feather = 0.25;
    let mult = smoothstep(radius - feather, radius + feather, ratio);
    var c = pc.color;
    c.a = c.a * mult;
    return c;
}
)WGSL";
        }

        // ---------------------------------------------------------------
        //  Textured fragment shaders
        // ---------------------------------------------------------------

        std::string textFragmentSource()
        {
            return std::string(kPushBlock) + R"WGSL(
@group(0) @binding(1) var textureSampler : texture_2d<f32>;
@group(0) @binding(2) var textureSampler_s : sampler;

@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let a = textureSampleLevel(textureSampler, textureSampler_s,
                               fTexture, 0.0).r;
    return vec4<f32>(pc.color.r, pc.color.g, pc.color.b, pc.color.a * a);
}
)WGSL";
        }

        std::string textureFragmentSource()
        {
            return R"WGSL(
@group(0) @binding(1) var textureSampler : texture_2d<f32>;
@group(0) @binding(2) var textureSampler_s : sampler;

@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    return textureSampleLevel(textureSampler, textureSampler_s,
                              fTexture, 0.0); // * pc.color;
}
)WGSL";
        }

        namespace
        {
            // Two textures, the second one named textureSamplerB.
            const char* const kTwoTextures = R"WGSL(
@group(0) @binding(1) var textureSampler : texture_2d<f32>;
@group(0) @binding(2) var textureSampler_s : sampler;
@group(0) @binding(3) var textureSamplerB : texture_2d<f32>;
@group(0) @binding(4) var textureSamplerB_s : sampler;
)WGSL";
        } // namespace

        std::string differenceFragmentSource()
        {
            return std::string(kTwoTextures) + R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let c  = textureSampleLevel(textureSampler,  textureSampler_s,
                                fTexture, 0.0);
    let cB = textureSampleLevel(textureSamplerB, textureSamplerB_s,
                                fTexture, 0.0);
    return vec4<f32>(abs(c.r - cB.r), abs(c.g - cB.g), abs(c.b - cB.b),
                     max(c.a, cB.a));
}
)WGSL";
        }

        std::string multiplyFragmentSource()
        {
            return std::string(kTwoTextures) + R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let c  = textureSampleLevel(textureSampler,  textureSampler_s,
                                fTexture, 0.0);
    let cB = textureSampleLevel(textureSamplerB, textureSamplerB_s,
                                fTexture, 0.0);
    return vec4<f32>(abs(c.r * cB.r), abs(c.g * cB.g), abs(c.b * cB.b),
                     max(c.a, cB.a));
}
)WGSL";
        }

        std::string addFragmentSource()
        {
            return std::string(kTwoTextures) + R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let c  = textureSampleLevel(textureSampler,  textureSampler_s,
                                fTexture, 0.0);
    let cB = textureSampleLevel(textureSamplerB, textureSamplerB_s,
                                fTexture, 0.0);
    return vec4<f32>(abs(c.r + cB.r), abs(c.g + cB.g), abs(c.b + cB.b),
                     max(c.a, cB.a));
}
)WGSL";
        }

        std::string butterflyFragmentSource()
        {
            // The same half of both, the second one mirrored, so that
            // the middle of the picture is on both sides of the seam.
            // (Both samples are always taken, then selected, so this is
            // valid however the compiler analyses control flow.)
            return std::string(kTwoTextures) + R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    let a = textureSampleLevel(textureSampler, textureSampler_s,
                               fTexture, 0.0);
    let b = textureSampleLevel(textureSamplerB, textureSamplerB_s,
                               vec2<f32>(1.0 - fTexture.x, fTexture.y), 0.0);
    return select(b, a, fTexture.x < 0.5);
}
)WGSL";
        }

        // ---------------------------------------------------------------
        //  Image (RGB / YUV) fragment shader
        // ---------------------------------------------------------------

        std::string imageFragmentSource()
        {
            const std::string header = R"WGSL(
struct UBO
{
    yuvCoefficients : vec4<f32>,
    color           : vec4<f32>,
    pixelType       : i32,
    videoLevels     : i32,
    imageChannels   : i32,
    mirrorX         : i32,
    mirrorY         : i32,
};

@group(0) @binding(1) var<uniform> ubo : UBO;

@group(0) @binding(2) var textureSampler0 : texture_2d<f32>;
@group(0) @binding(3) var textureSampler0_s : sampler;
@group(0) @binding(4) var textureSampler1 : texture_2d<f32>;
@group(0) @binding(5) var textureSampler1_s : sampler;
@group(0) @binding(6) var textureSampler2 : texture_2d<f32>;
@group(0) @binding(7) var textureSampler2_s : sampler;

// enum tl::image::PixelType
const PixelType_None              : i32 = 0;
const PixelType_L_U8              : i32 = 1;
const PixelType_L_U16             : i32 = 2;
const PixelType_L_U32             : i32 = 3;
const PixelType_L_F16             : i32 = 4;
const PixelType_L_F32             : i32 = 5;

const PixelType_LA_U8             : i32 = 6;
const PixelType_LA_U16            : i32 = 7;
const PixelType_LA_U32            : i32 = 8;
const PixelType_LA_F16            : i32 = 9;
const PixelType_LA_F32            : i32 = 10;

const PixelType_RGB_U8            : i32 = 11;
const PixelType_RGB_U10           : i32 = 12;
const PixelType_RGB_U16           : i32 = 13;
const PixelType_RGB_U32           : i32 = 14;
const PixelType_RGB_F16           : i32 = 15;
const PixelType_RGB_F32           : i32 = 16;

const PixelType_RGBA_U8           : i32 = 17;
const PixelType_RGBA_U16          : i32 = 18;
const PixelType_RGBA_U32          : i32 = 19;
const PixelType_RGBA_F16          : i32 = 20;
const PixelType_RGBA_F32          : i32 = 21;

const PixelType_YUV_420P_U8       : i32 = 22;
const PixelType_YUV_422P_U8       : i32 = 23;
const PixelType_YUV_444P_U8       : i32 = 24;

const PixelType_YUV_420P_U10      : i32 = 25;
const PixelType_YUV_422P_U10      : i32 = 26;
const PixelType_YUV_444P_U10      : i32 = 27;

const PixelType_YUV_420P_U12      : i32 = 28;
const PixelType_YUV_422P_U12      : i32 = 29;
const PixelType_YUV_444P_U12      : i32 = 30;

const PixelType_YUV_420P_U16      : i32 = 31;
const PixelType_YUV_422P_U16      : i32 = 32;
const PixelType_YUV_444P_U16      : i32 = 33;

const PixelType_YUV_420SP_U8      : i32 = 34;
const PixelType_YUV_420SP_U16     : i32 = 35;

const PixelType_YUV_422SP_U8      : i32 = 36;
const PixelType_YUV_422SP_U16     : i32 = 37;

const PixelType_YUV_444SP_U8      : i32 = 38;
const PixelType_YUV_444SP_U16     : i32 = 39;

const PixelType_ARGB_4444_Premult : i32 = 40;

// enum tl::image::VideoLevels
const VideoLevels_FullRange  : i32 = 0;
const VideoLevels_LegalRange : i32 = 1;
)WGSL";

            const std::string functions = R"WGSL(
fn getBitDepth(pixelType : i32) -> f32
{
    if (pixelType == PixelType_YUV_420P_U10 ||
        pixelType == PixelType_YUV_422P_U10 ||
        pixelType == PixelType_YUV_444P_U10)
    {
        return 10.0;
    }
    else if (pixelType == PixelType_YUV_420P_U12 ||
             pixelType == PixelType_YUV_422P_U12 ||
             pixelType == PixelType_YUV_444P_U12)
    {
        return 12.0;
    }
    else if (pixelType == PixelType_YUV_420P_U16 ||
             pixelType == PixelType_YUV_422P_U16 ||
             pixelType == PixelType_YUV_444P_U16 ||
             pixelType == PixelType_YUV_444SP_U16 ||
             pixelType == PixelType_YUV_422SP_U16 ||
             pixelType == PixelType_YUV_420SP_U16)
    {
        return 16.0;
    }
    else if (pixelType == PixelType_ARGB_4444_Premult)
    {
        return 4.0;
    }
    // U8 fallback
    return 8.0;
}

fn sampleTexture(
    textureCoord    : vec2<f32>,
    pixelType       : i32,
    videoLevels     : i32,
    yuvCoefficients : vec4<f32>,
    imageChannels   : i32,
    t0 : texture_2d<f32>, s0 : sampler,
    t1 : texture_2d<f32>, s1 : sampler,
    t2 : texture_2d<f32>, s2 : sampler) -> vec4<f32>
{
    var c = vec4<f32>(0.0);
    var y  = 0.0;
    var cr = 0.0;
    var cb = 0.0;

    if (pixelType >= PixelType_YUV_420P_U8 &&
        pixelType <= PixelType_YUV_444SP_U16)
    {
        // Check semi-Planar types first:
        if (pixelType >= PixelType_YUV_420SP_U8 &&
            pixelType <= PixelType_YUV_444SP_U16)
        {
            // Semi-Planar (NV12 / P010 / P016):
            y = textureSampleLevel(t0, s0, textureCoord, 0.0).r;
            let uv = textureSampleLevel(t1, s1, textureCoord, 0.0);
            cb = uv.r;
            cr = uv.g;
        }
        else
        {
            // Fully Planar (YUV 420P/422P/444P):
            y  = textureSampleLevel(t0, s0, textureCoord, 0.0).r;
            cb = textureSampleLevel(t1, s1, textureCoord, 0.0).r;
            cr = textureSampleLevel(t2, s2, textureCoord, 0.0).r;

            // Normalize packed 10-bit / 12-bit in 16-bit textures
            if (pixelType == PixelType_YUV_420P_U10 ||
                pixelType == PixelType_YUV_422P_U10 ||
                pixelType == PixelType_YUV_444P_U10)
            {
                let rangeScale = 1023.0 / 65535.0;
                y  = y / rangeScale;
                cb = cb / rangeScale;
                cr = cr / rangeScale;
            }
            else if (pixelType == PixelType_YUV_420P_U12 ||
                     pixelType == PixelType_YUV_422P_U12 ||
                     pixelType == PixelType_YUV_444P_U12)
            {
                let rangeScale = 4095.0 / 65535.0;
                y  = y / rangeScale;
                cb = cb / rangeScale;
                cr = cr / rangeScale;
            }
        }

        // 2. Apply Video Levels (Full vs Legal Range)
        if (videoLevels == VideoLevels_FullRange)
        {
            cb = cb - 0.5;
            cr = cr - 0.5;
        }
        else if (videoLevels == VideoLevels_LegalRange)
        {
            let bitDepth = getBitDepth(pixelType);
            let maxValue = pow(2.0, bitDepth) - 1.0;
            let scale = pow(2.0, bitDepth - 8.0);

            let yMin = 16.0 * scale;
            let yMax = 235.0 * scale;
            let cMin = 16.0 * scale;
            let cMax = 240.0 * scale;

            y  = clamp((y  * maxValue - yMin) / (yMax - yMin), 0.0, 1.0);
            cb = clamp((cb * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
            cr = clamp((cr * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
        }

        // 3. YUV to RGB Matrix Transformation
        c.r = y + (yuvCoefficients.x * cr);
        c.g = y - (yuvCoefficients.y * cr) - (yuvCoefficients.z * cb);
        c.b = y + (yuvCoefficients.w * cb);
        c.a = 1.0;
    }
    else
    {
        c = textureSampleLevel(t0, s0, textureCoord, 0.0);

        if (VideoLevels_LegalRange == videoLevels)
        {
            c.r = (c.r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));
            c.g = (c.g - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
            c.b = (c.b - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
        }

        if (1 == imageChannels)
        {
            c.g = c.r;
            c.b = c.r;
            c.a = 1.0;
        }
        else if (2 == imageChannels)
        {
            c.a = c.g;
            c.g = c.r;
            c.b = c.r;
        }
        else if (3 == imageChannels)
        {
            c.a = 1.0;
        }
    }
    return c;
}

@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    var t = fTexture;
    if (1 == ubo.mirrorX)
    {
        t.x = 1.0 - t.x;
    }
    if (0 == ubo.mirrorY)
    {
        t.y = 1.0 - t.y;
    }

    return sampleTexture(t,
                         ubo.pixelType,
                         ubo.videoLevels,
                         ubo.yuvCoefficients,
                         ubo.imageChannels,
                         textureSampler0, textureSampler0_s,
                         textureSampler1, textureSampler1_s,
                         textureSampler2, textureSampler2_s) * ubo.color;
}
)WGSL";
            return header + functions;
        }

        // ---------------------------------------------------------------
        //  Debanding
        // ---------------------------------------------------------------

        std::string debandingFragmentSource(
            const float threshold, const float range, const int iterations,
            const float grain)
        {
            // Defines deband(tex, samp, pos), used by displayFragmentSource()
            // in place of the plain texture read when it is non-empty.
            return string::Format(R"WGSL(
// Simplified pseudocode representation of the deband algorithm

const kTHRESHOLD : f32 = {0};
const kRANGE : f32 = {1};
const kITERATIONS : i32 = {2};
const kGRAIN : f32 = {3};

// PRNG helpers for randomness
fn mod289(x : f32) -> f32 { return x - floor(x / 289.0) * 289.0; }
fn permute(x : f32) -> f32 { return mod289((34.0 * x + 1.0) * x); }
fn rand(x : f32) -> f32 { return fract(x / 41.0); }

// Main sampling function per pixel
fn deband(tex : texture_2d<f32>, samp : sampler, pos : vec2<f32>) -> vec4<f32>
{
    let m = vec3<f32>(pos, rand(pos.x + pos.y)) + vec3<f32>(1.0);
    var h = permute(permute(permute(m.x) + m.y) + m.z);

    let imageSize = vec2<f32>(textureDimensions(tex, 0));
    let invImageSize = vec2<f32>(1.0) / imageSize;

    var col = textureSampleLevel(tex, samp, pos, 0.0);

    for (var i : i32 = 1; i <= kITERATIONS; i = i + 1)
    {
        let fi = f32(i);

        // Compute local average with increasing range (i * kRANGE)
        let rng = fi * kRANGE;

        let dist = rand(h) * rng;
        h = permute(h);
        let dir = rand(h) * 6.2831853;
        h = permute(h);

        // Compute offset vector
        let pt = dist * invImageSize; // Normalize to texture space
        let o = vec2<f32>(cos(dir), sin(dir));

        // Sample 4 points at 90-degree intervals around the pixel
        let r0 = textureSampleLevel(tex, samp,
                                    pos + pt * vec2<f32>(o.x, o.y), 0.0);
        let r1 = textureSampleLevel(tex, samp,
                                    pos + pt * vec2<f32>(-o.y, o.x), 0.0);
        let r2 = textureSampleLevel(tex, samp,
                                    pos + pt * vec2<f32>(-o.x, -o.y), 0.0);
        let r3 = textureSampleLevel(tex, samp,
                                    pos + pt * vec2<f32>(o.y, -o.x), 0.0);

        // Average them
        let avg = (r0 + r1 + r2 + r3) / 4.0;

        // Compute absolute difference
        let diff = abs(col - avg);

        // Dynamic threshold decreases per iteration for finer control
        let iterThreshold = kTHRESHOLD / (fi * 16384.0);

        col = select(avg, col, diff > vec4<f32>(iterThreshold));
    }

    if (kGRAIN > 0.0)
    {
        // Generate a 3D noise vector cheaply
        let noise = vec3<f32>(
            fract(h * 1.234),
            fract(h * 2.345),
            fract(h * 3.456));
        col = vec4<f32>(
            col.rgb + (kGRAIN * (1.0 / 8192.0)) * (noise - vec3<f32>(0.5)),
            col.a);
    }

    return col;
}
)WGSL")
                .arg(threshold)
                .arg(range)
                .arg(iterations)
                .arg(grain);
        }

        // ---------------------------------------------------------------
        //  Display
        // ---------------------------------------------------------------

        std::string displayVertexSource()
        {
            return R"WGSL(
struct Transform
{
    mvp : mat4x4<f32>,
};
@group(0) @binding(0) var<uniform> transform : Transform;

struct VSOut
{
    @builtin(position) position : vec4<f32>,
    @location(0) fTexture : vec2<f32>,
};

@vertex
fn main(@location(0) vPos : vec3<f32>,
        @location(1) vTexture : vec2<f32>) -> VSOut
{
    var r : VSOut;
    r.position = transform.mvp * vec4<f32>(vPos, 1.0);
    r.fTexture = vTexture;
    return r;
}
)WGSL";
        }

        std::string displayFragmentSource(
            const std::string& /*ocioICSDef*/, const std::string& /*ocioICS*/,
            const std::string& /*ocioDef*/, const std::string& /*ocio*/,
            const std::string& /*lutDef*/, const std::string& /*lut*/,
            timeline::LUTOrder /*lutOrder*/, const std::string& /*toneMapDef*/,
            const std::string& /*toneMap*/, const std::string& debandingDef,
            const std::string& /*debanding*/)
        {
            // OCIO, LUT and libplacebo tone mapping snippets are GLSL, so
            // they are ignored (no-op).  The debanding read is chosen here
            // from @p debandingDef instead of using the caller's GLSL
            // statement.
            const std::string header = R"WGSL(
@group(0) @binding(1) var textureSampler : texture_2d<f32>;
@group(0) @binding(2) var textureSampler_s : sampler;

// enum tl::timeline::Channels
const Channels_Color : i32 = 0;
const Channels_Red   : i32 = 1;
const Channels_Green : i32 = 2;
const Channels_Blue  : i32 = 3;
const Channels_Alpha : i32 = 4;
const Channels_Luma  : i32 = 5;

// enum tl::image::VideoLevels
const VideoLevels_FullRange  : i32 = 0;
const VideoLevels_LegalRange : i32 = 1;

struct Levels
{
    enabled : u32,
    inLow   : f32,
    inHigh  : f32,
    gamma   : f32,
    outLow  : f32,
    outHigh : f32,
};
@group(0) @binding(3) var<uniform> uboLevels : Levels;

struct Normalize
{
    enabled : u32,
    minimum : vec4<f32>,
    maximum : vec4<f32>,
};
@group(0) @binding(4) var<uniform> uboNormalize : Normalize;

struct Color
{
    enabled : u32,
    add     : vec3<f32>,
    matrix  : mat4x4<f32>,
    invert  : u32,
};
@group(0) @binding(5) var<uniform> uboColor : Color;

struct Options
{
    channels      : i32,
    mirrorX       : i32,
    mirrorY       : i32,
    dither        : i32,
    softClip      : f32,
    videoLevels   : i32,
    invalidValues : i32,
};
@group(0) @binding(6) var<uniform> ubo : Options;

fn colorFunc(value : vec4<f32>, add : vec3<f32>, m : mat4x4<f32>) -> vec4<f32>
{
    var tmp = vec4<f32>(value.rgb + add, 1.0);
    tmp = tmp * m;
    tmp.w = value.w;
    return tmp;
}

fn levelsFunc(value : vec4<f32>, data : Levels) -> vec4<f32>
{
    // (The GLSL version left tmp undefined when the input range was empty.)
    var tmp = vec3<f32>(0.0);
    let levelsRange = data.inHigh - data.inLow;
    if (levelsRange > 0.0)
    {
        tmp = clamp((value.rgb - vec3<f32>(data.inLow)) / levelsRange,
                    vec3<f32>(0.0), vec3<f32>(1.0));
    }
    if (tmp.x >= 0.0) { tmp.x = pow(tmp.x, data.gamma); }
    if (tmp.y >= 0.0) { tmp.y = pow(tmp.y, data.gamma); }
    if (tmp.z >= 0.0) { tmp.z = pow(tmp.z, data.gamma); }
    return vec4<f32>(tmp * data.outHigh + vec3<f32>(data.outLow), value.a);
}

fn softClipFunc(value : vec4<f32>, softClip : f32) -> vec4<f32>
{
    var v = value;
    let tmp = 1.0 - softClip;
    if (v.x > tmp) { v.x = tmp + (1.0 - exp(-(v.x - tmp) / softClip)) * softClip; }
    if (v.y > tmp) { v.y = tmp + (1.0 - exp(-(v.y - tmp) / softClip)) * softClip; }
    if (v.z > tmp) { v.z = tmp + (1.0 - exp(-(v.z - tmp) / softClip)) * softClip; }
    return v;
}

fn normalizeFunc(value : vec4<f32>, data : Normalize) -> vec4<f32>
{
    return (value - data.minimum) / (data.maximum - data.minimum);
}
)WGSL";

            const std::string sample =
                debandingDef.empty()
                    ? "outColor = textureSampleLevel(textureSampler, "
                      "textureSampler_s, t, 0.0);\n"
                    : "outColor = deband(textureSampler, textureSampler_s, "
                      "t);\n";

            const std::string main1 = R"WGSL(
@fragment
fn main(@location(0) fTexture : vec2<f32>) -> @location(0) vec4<f32>
{
    var outColor = vec4<f32>(0.0);

    var t = fTexture;
    if (1 == ubo.mirrorX)
    {
        t.x = 1.0 - t.x;
    }
    if (1 == ubo.mirrorY)
    {
        t.y = 1.0 - t.y;
    }

    // Read texture with or without debanding.
    )WGSL";

            const std::string main2 = R"WGSL(
    // Video levels.
    if (VideoLevels_LegalRange == ubo.videoLevels)
    {
        let scale = (940.0 - 64.0) / 1023.0;
        let offset = 64.0 / 1023.0;
        outColor.r = outColor.r * scale + offset;
        outColor.g = outColor.g * scale + offset;
        outColor.b = outColor.b * scale + offset;
    }

    // Apply color transformations in linear space.
    if (uboColor.enabled != 0u)
    {
        outColor = colorFunc(outColor, uboColor.add, uboColor.matrix);
        if (uboColor.invert != 0u)
        {
            outColor.r = 1.0 - outColor.r;
            outColor.g = 1.0 - outColor.g;
            outColor.b = 1.0 - outColor.b;
        }
    }
    if (ubo.softClip > 0.0)
    {
        outColor = softClipFunc(outColor, ubo.softClip);
    }

    if (uboLevels.enabled != 0u)
    {
        outColor = levelsFunc(outColor, uboLevels);
    }
    if (uboNormalize.enabled != 0u)
    {
        outColor = normalizeFunc(outColor, uboNormalize);
    }
    if (ubo.invalidValues == 1)
    {
        if (outColor.r < 0.0 || outColor.r > 1.0 ||
            outColor.g < 0.0 || outColor.g > 1.0 ||
            outColor.b < 0.0 || outColor.b > 1.0 ||
            outColor.a < 0.0 || outColor.a > 1.0)
        {
            outColor.r = 1.0;
            outColor.g = outColor.g * 0.5;
            outColor.b = outColor.b * 0.5;
        }
    }

    // Swizzle for the channels display.
    if (Channels_Red == ubo.channels)
    {
        outColor.g = outColor.r;
        outColor.b = outColor.r;
    }
    else if (Channels_Green == ubo.channels)
    {
        outColor.r = outColor.g;
        outColor.b = outColor.g;
    }
    else if (Channels_Blue == ubo.channels)
    {
        outColor.r = outColor.b;
        outColor.g = outColor.b;
    }
    else if (Channels_Alpha == ubo.channels)
    {
        outColor.r = outColor.a;
        outColor.g = outColor.a;
        outColor.b = outColor.a;
    }
    else if (Channels_Luma == ubo.channels)
    {
        let luma = (outColor.r + outColor.g + outColor.b) / 3.0;
        outColor.r = luma;
        outColor.g = luma;
        outColor.b = luma;
    }

    return outColor;
}
)WGSL";

            return header + debandingDef + main1 + sample + main2;
        }

    } // namespace timeline_wgpu
} // namespace tl
