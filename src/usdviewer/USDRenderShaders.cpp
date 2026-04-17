// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace usd
    {
        std::string vertexDummy()
        {
            return R"(#version 450
layout(location = 0) in vec3 vPos;

layout(location = 0) out vec3 Peye;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
     mat4 model;
     mat4 view;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 1.0);   
    Peye = (transform.view * transform.model * vec4(vPos, 1.0)).xyz;
})";
        }

        std::string fragmentDummy()
        {
            return R"(#version 450
layout(location = 0) in vec3 Peye;
layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
void main()
{
    vec3 dx = dFdx(Peye);
    vec3 dy = dFdy(Peye);
    vec3 N = normalize(cross(dx, dy));

    // Simple light direction
    vec3 L = normalize(Peye);

    // Diffuse (Lambert)
    float diff = max(dot(N, L), 0.0);

    // Add a bit of ambient so it's not fully black
    float ambient = 0.2;

    vec3 finalColor = pc.color.rgb * (ambient + diff);

    outColor = vec4(finalColor, pc.color.a);
    
})";
        }
        
        
        std::string vertexUSD()
        {
            return R"(#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexture;

layout(location = 0) out vec3 Peye;  // ← view-space position (Peye)
layout(location = 1) out vec2 fTexture;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
     mat4 model;
     mat4 view;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 1.0);
    fTexture = vTexture;
    Peye = (transform.view * transform.model * vec4(vPos, 1.0)).xyz;
})";
        }
        
        std::string fragmentUSD()
        {
            return R"(#version 450

layout(location = 0) in vec3 Peye;
layout(location = 1) in vec2 fTexture;

layout(binding = 1) uniform sampler2D u_DiffuseMap;
layout(binding = 2) uniform sampler2D u_EmissiveMap;
layout(binding = 3) uniform sampler2D u_MetallicMap;
layout(binding = 4) uniform sampler2D u_RoughnessMap;
layout(binding = 5) uniform sampler2D u_NormalMap;
layout(binding = 6) uniform sampler2D u_AOMap;
layout(binding = 7) uniform sampler2D u_OpacityMap;

layout(set = 0, binding = 8, std140) uniform Parameters {
     float opacityThreshold;
} param;


layout(set = 0, binding = 9, std140) uniform Scene {
     vec3 camPos;
} scene;

layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
// ─────────────────────────────────────────────
//  Hydra-accurate PBR helpers (copied verbatim from previewSurface.glslfx)
// ─────────────────────────────────────────────
const float PI      = 3.14159265359;
const float EPSILON = 0.0001;

vec3 SchlickFresnel(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float NormalDistribution(float roughness, float NdotH) {
    float alpha  = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (alpha2 - 1.0) + 1.0);
    denom        = denom * denom * PI;
    return (alpha2 + EPSILON) / denom;
}

float Geometric(float roughness, float NdotL, float NdotV) {
    float alpha = roughness * roughness;
    float k     = alpha * 0.5;                    // ← exact Hydra k
    float G1V   = NdotV / (NdotV * (1.0 - k) + k);
    float G1L   = NdotL / (NdotL * (1.0 - k) + k);
    return G1V * G1L;
}

mat3 ComputeTBNMatrix(vec3 P, vec3 N, vec2 st) {
    vec3 dPdx = dFdx(P); vec3 dPdy = dFdy(P);
    vec3 sigmaX = dPdx - dot(dPdx, N) * N;
    vec3 sigmaY = dPdy - dot(dPdy, N) * N;
    float flipSign = dot(dPdy, cross(N, dPdx)) < 0 ? -1.0 : 1.0;

    vec2 dSTdx = dFdx(st); vec2 dSTdy = dFdy(st);
    float det = dot(dSTdx, vec2(dSTdy.y, -dSTdy.x));
    float signDet = det < 0 ? -1.0 : 1.0;
    vec2 invC0 = signDet * vec2(dSTdy.y, -dSTdx.y);

    vec3 T = sigmaX * invC0.x + sigmaY * invC0.y;
    if (abs(det) > 0.0) T = normalize(T);
    vec3 B = (signDet * flipSign) * cross(N, T);

    return mat3(T, B, N);
}

void main()
{
    vec2 st = fTexture;

    // Material (move to UBO later if you want)
    vec4  u_Material_diffuseColor = vec4(1.0);
    float u_Material_metallic = 1.0;
    float u_Material_roughness = 1.0;
    float u_Material_aoStrength = 1.0;

    float opacity   = texture(u_OpacityMap, st).a;
    if (opacity < param.opacityThreshold)
    {
       discard;
    }

    // ── Sample textures ───────────────────────
    vec3 albedo = texture(u_DiffuseMap,   st).rgb * pc.color.rgb;
    float metallic  = texture(u_MetallicMap,  st).r * u_Material_metallic;
    float roughness = texture(u_RoughnessMap, st).r * u_Material_roughness;
    float ao        = mix(1.0, texture(u_AOMap, st).r, u_Material_aoStrength);
    vec3 emissive   = texture(u_EmissiveMap, st).rgb;

    // Clamp to physically plausible range
    roughness = clamp(roughness, 0.05, 1.0);
    metallic  = clamp(metallic,  0.0,  1.0);

    // ── Normal from normal map ────────────────
    vec3 Nt  = texture(u_NormalMap, st).rgb * 2.0 - 1.0; // [0,1] → [-1,1]
    vec3 N_base = normalize(cross(dFdx(Peye), dFdy(Peye)));
    mat3 TBN = ComputeTBNMatrix(Peye, N_base, st);
    vec3 N = normalize(TBN * Nt);

    // ── View / Light / Half vectors (Hydra eye-space style) ──
    vec3 V = normalize(Peye - scene.camPos);
    vec3 L = V;
    vec3 H = normalize(L + V);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // ── Base reflectance (F0) ─────────────────
    // Dialectrics use 0.04; metals use their albedo colour.
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance specular (exact Hydra evaluateDirectSpecular)
    float NDF = NormalDistribution(roughness, NdotH);
    float G   = Geometric(roughness, NdotL, NdotV);
    vec3  F   = SchlickFresnel(HdotV, F0);

    vec3 numerator   = NDF * G * F;
    float denom      = 4.0 * NdotL * NdotV + EPSILON;
    vec3 specular    = numerator / denom;

    // Diffuse + energy conservation
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    // ── Radiance & final colour ───────────────
    vec3  u_Scene_lightColor = vec3(1.0);
    float dist     = length(L);
    float atten    = 1.0 / (dist * dist);          // inverse-square falloff
    vec3  radiance = u_Scene_lightColor; // * atten;
 
    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // Simulate environment reflection
    vec3 reflectionVector = reflect(-V, N);

    // Fake environment light (a gradient from sky to ground)
    vec3 envColor = mix(vec3(0.2, 0.2, 0.2), vec3(0.8, 0.9, 1.0),
                        reflectionVector.y * 0.5 + 0.5);

   // Dielectrics get flat ambient, Metals get the environment reflection
   vec3 ambientDiffuse = vec3(0.03) * albedo;
   vec3 ambientSpecular = envColor * F0 * (1.0 - roughness); 

    vec3 ambient = (ambientDiffuse + ambientSpecular) * ao;

    // Combine ambient + diffuse + specular
    vec3 color = ambient + Lo + emissive;

    // ── Tone mapping (Reinhard) + gamma  ─────── 
    // color = color / (color + vec3(1.0));            // HDR → LDR
    // color = pow(color, vec3(1.0 / 2.2));            // linear → sRGB

    outColor = vec4(color, opacity);

    // VERIFIED: albedo and ao are okay.
    // outColor = vec4(albedo, 1.0);

    // VERIFIED: Ambient occlusion works correctly
    // outColor = vec4(ambient, opacity);

    // VERIFIED: normal (N) is faceted but correct!
    // outColor = vec4((N + 1) / 2, opacity);

    // VERIFIED: diffuse is correct for metallic
    //outColor = vec4(diffuse, opacity);

    // VERIFIED: specular is correct 
    // outColor = vec4(specular, opacity);

    // VERIFIED: opacity works correctly.
    // outColor = vec4(vec3(opacity), opacity);

    // INCORRECT: normal mapping does not work correctly.

})";
        }
        
        std::string vertexSTs()
        {
            return R"(#version 450
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexture;
layout(location = 0) out vec2 fTexture;
layout(location = 1) out vec3 fragPosition;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
     mat4 model;
     mat4 view;
} transform;

void main()
{
    fragPosition = vPos;  // no need to transform this one for STs
    fTexture = vTexture;
    gl_Position = transform.mvp * vec4(vPos, 1.0);
})";
        }

        std::string fragmentSTs()
        {
            return R"(#version 450
layout(location = 0) in vec2 fTexture;
layout(location = 1) in vec3 inPosition;

layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
void main()
{
      outColor = vec4(fTexture.r, fTexture.g, 0, 1);
})";
        }
        
        
    } // namespace usd
} // namespace tl
