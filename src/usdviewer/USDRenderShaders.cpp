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

layout(location = 0) out vec3 fPos;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
     mat4 model;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 1.0);   
    fPos = (transform.model * vec4(vPos, 1.0)).xyz;
})";
        }

        std::string fragmentDummy()
        {
            return R"(#version 450
layout(location = 0) in vec3 fPos;
layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
void main()
{
    vec3 dx = dFdx(fPos);
    vec3 dy = dFdy(fPos);
    vec3 N = normalize(cross(dx, dy));

    // Simple light direction
    vec3 lightPos = vec3(0.0, 0.0, 0.0);
    vec3 L = normalize(fPos - lightPos);

    // Diffuse (Lambert)
    float diff = max(dot(N, L), 0.0);

    // Add a bit of ambient so it's not fully black
    float ambient = 0.2;

    vec3 finalColor = pc.color.rgb * (ambient + diff);

    outColor = vec4(finalColor, pc.color.a);
})";
        }
        
        std::string vertexPBR()
        {
            return R"(#version 450
// ─────────────────────────────────────────────
//  Per-vertex attributes
// ─────────────────────────────────────────────
// layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec2 a_TexCoord;
// layout(location = 3) in vec4 a_Tangent;   // xyz = tangent, w = bitangent sign

layout(location = 0) in vec3  a_Position;       // GL_HALF_FLOAT  → 6 bytes
layout(location = 1) in int   a_NormalPacked;   // GL_INT_2_10_10_10_REV → 4 bytes
layout(location = 2) in vec2  a_TexCoord;       // GL_HALF_FLOAT  → 4 bytes
layout(location = 3) in int   a_TangentPacked;  // GL_INT_2_10_10_10_REV → 4 bytes

// ─────────────────────────────────────────────
//  Uniforms
// ─────────────────────────────────────────────
layout(std140, binding = 2) uniform Transform {
    mat4 model;         // object → world
    mat4 view;          // world  → camera
    mat4 projection;    // camera → clip
    mat3 normalMatrix;  // transpose(inverse(model)), for correct normal transform
} u_Transform;

// ─────────────────────────────────────────────
//  Outputs to the fragment shader
// ─────────────────────────────────────────────
out VertexData {
    vec3 FragPos;   // world-space position
    vec3 Normal;    // world-space normal
    vec2 TexCoord;  // UV (passed through unchanged)
    mat3 TBN;       // tangent-space → world-space rotation matrix
} vs_out;

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
void main() {

    // ── World-space position ──────────────────
    vec4 worldPos   = u_Transform.model * vec4(a_Position, 1.0);
    vs_out.FragPos  = worldPos.xyz;

    // ── World-space normal ────────────────────
    // Use the normal matrix to handle non-uniform scaling correctly.
    vs_out.Normal   = normalize(u_Transform.normalMatrix * a_Normal);

    // ── UV passthrough ────────────────────────
    vs_out.TexCoord = a_TexCoord;

    // ── TBN matrix ───────────────────────────
    // Re-orthogonalise T against N (Gram-Schmidt) so that any slight
    // imprecision in the mesh data or normal-matrix transform doesn't
    // cause the basis to become non-orthogonal.
    vec3 N = vs_out.Normal;
    vec3 T = normalize(u_Transform.normalMatrix * a_Tangent.xyz);
    T = normalize(T - dot(T, N) * N);       // make T perpendicular to N

    // a_Tangent.w encodes the handedness of the bitangent (±1).
    // Using cross(N, T) instead of cross(T, N) keeps a right-handed basis.
    vec3 B = cross(N, T) * a_Tangent.w;

    // Columns of TBN transform a vector from tangent space to world space,
    // which is exactly what the fragment shader needs to unpack the normal map.
    vs_out.TBN = mat3(T, B, N);

    // ── Clip-space position ───────────────────
    gl_Position = u_Transform.projection
                * u_Transform.view
                * worldPos;
}

)";
        }
        
        std::string vertexUSD()
        {
            return R"(#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexture;

layout(location = 0) out vec3 fPos;
layout(location = 1) out vec2 fTexture;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
     mat4 model;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 1.0);
    fTexture = vTexture;
    // fPos = vPos;
    fPos = (transform.model * vec4(vPos, 1.0)).xyz;
})";
        }
        
        std::string fragmentUSD()
        {
            return R"(#version 450
layout(location = 0) in vec3 fPos;
layout(location = 1) in vec2 fTexture;

layout(binding = 1) uniform sampler2D u_DiffuseMap;
layout(binding = 2) uniform sampler2D u_MetallicMap;
layout(binding = 3) uniform sampler2D u_RoughnessMap;
layout(binding = 4) uniform sampler2D u_NormalMap;
layout(binding = 5) uniform sampler2D u_AOMap;
layout(binding = 6) uniform sampler2D u_OpacityMap;

layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
const float PI      = 3.14159265359;
const float EPSILON = 0.0001;

// ═════════════════════════════════════════════
//  PBR helper functions
// ═════════════════════════════════════════════

// Calculation of TBN matrix and terminology based on "Surface
// Gradient-Based Bump Mapping Framework" (2020)
mat3
ComputeTBNMatrix(vec3 P, vec3 N, vec2 st)
{
    // Get screen space derivatives of position
    vec3 dPdx = dFdx(P);
    vec3 dPdy = dFdy(P);

    // Ensure position derivatives are perpendicular to N
    vec3 sigmaX = dPdx - dot(dPdx, N) * N;
    vec3 sigmaY = dPdy - dot(dPdy, N) * N;

    float flipSign = dot(dPdy, cross(N, dPdx)) < 0 ? -1 : 1;

    // Get screen space derivatives of st
    vec2 dSTdx = dFdx(st);
    vec2 dSTdy = dFdy(st);

    // Get determinant and determinant sign of st matrix
    float det = dot(dSTdx, vec2(dSTdy.y, -dSTdy.x));
    float signDet = det < 0 ? -1 : 1;

    // Get first column of inv st matrix
    // Don't divide by det, but scale by its sign
    vec2 invC0 = signDet * vec2(dSTdy.y, -dSTdx.y);

    vec3 T = sigmaX * invC0.x + sigmaY * invC0.y;

    if (abs(det) > 0) {
        T = normalize(T);
    }

    vec3 B = (signDet * flipSign) * cross(N, T);

    return mat3(T, B, N);
}

// Normal Distribution Function – GGX / Trowbridge-Reitz
float NDF_GGX(float NdotH, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d + EPSILON);
}

// Geometry function – Smith's method with Schlick-GGX
float G_SchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness) {
    return G_SchlickGGX(NdotV, roughness)
         * G_SchlickGGX(NdotL, roughness);
}

// Fresnel – Schlick approximation
vec3 Fresnel_Schlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
    vec2 st = fTexture;

    // User's material parameters (\todo: pass as UBO)
    vec4  u_Material_diffuseColor = vec4(1,1,1,1); //pc.color;
    float u_Material_metallic = 1.0;
    float u_Material_roughness = 1.0;
    float u_Material_aoStrength = 1.0;

    // Scene parameters (\todo: pass as UBO? Not needed for a single light from
    //                          camera)
    vec3 u_Scene_camPos    = vec3(0, 0, 0);
    vec3 u_Scene_lightPos  = vec3(0, 0, 0);
    vec3 u_Scene_lightColor = vec3(2, 2, 2);


    // ── Sample textures ───────────────────────
    vec3  albedo    = texture(u_DiffuseMap,   st).rgb * u_Material_diffuseColor.rgb;
    float metallic  = texture(u_MetallicMap,  st).r * u_Material_metallic;
    float roughness = texture(u_RoughnessMap, st).r * u_Material_roughness;
    float opacity   = texture(u_OpacityMap, st).a;
    float ao        = mix(1.0, texture(u_AOMap, st).r, u_Material_aoStrength);

    // Clamp to physically plausible range
    roughness = clamp(roughness, 0.05, 1.0);
    metallic  = clamp(metallic,  0.0,  1.0);

    // ── Normal from normal map ────────────────
    vec3 Nt  = texture(u_NormalMap, st).rgb * 2.0 - 1.0; // [0,1] → [-1,1]
    // vec3 N   = normalize(fs_in.TBN * Nt);                 // tangent → world space

    // \@todo: -faceted- normal (see above for correct calculation)
    vec3 dx = dFdx(fPos);
    vec3 dy = dFdy(fPos);
    vec3 N = normalize(cross(dx, dy));

    mat3 TBN = ComputeTBNMatrix(fPos, N, st);
    N = normalize(TBN * Nt);

    // Normal mapping cannot be done in local space.
    // vec3 N = normalize(cross(dx, dy) + Nt);

    // ── Lighting vectors ──────────────────────
    //vec3 V = normalize(fPos - u_Scene_camPos);  // correct

    // vec3 V = normalize(-fPos);  // correct
    vec3 V = normalize(vec3(0, 0, -1)); // correct

    // Simple light direction (incorrect)
    //vec3 lightPos = vec3(0.0, 0.0, 0.0);
    //vec3 L = normalize(lightPos - fPos);

    vec3 L = normalize(vec3(0, 0, -1)); // correct

    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // ── Base reflectance (F0) ─────────────────
    // Dialectrics use 0.04; metals use their albedo colour.
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ── Cook-Torrance specular BRDF ───────────
    float NDF = NDF_GGX(NdotH, roughness);
    float G   = G_Smith(NdotV, NdotL, roughness);
    vec3  F   = Fresnel_Schlick(HdotV, F0);

    vec3 numerator   = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + EPSILON;
    vec3 specular     = numerator / denominator;

    // ── Diffuse (Lambertian) ──────────────────
    // Energy conservation: metals have no diffuse.
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    // ── Radiance & final colour ───────────────
    float dist     = length(u_Scene_lightPos - fPos);
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
    vec3 color = ambient + Lo;

    // ── Tone mapping (Reinhard) + gamma  ───────  WRONG AND UNNEEDED
    //    If we merge it into vmrv2, we can use libplacebo directly.
    // color = color / (color + vec3(1.0));            // HDR → LDR
    // color = pow(color, vec3(1.0 / 2.2));            // linear → sRGB

    outColor = vec4(color * opacity, opacity);

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
