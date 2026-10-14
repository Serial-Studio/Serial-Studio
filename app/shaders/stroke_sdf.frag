// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Fragment half of the signed-distance plot stroke (spec 0097). Coverage is the distance to the
// segment through the same profile the tessellated stroke draws: opaque out to `face`, a linear
// feather to zero at `face + feather`. Interior ends are round, so consecutive segments join
// round at any turn angle without fan geometry; ends flagged flat are cut at the end plane, as
// the tessellated stroke caps runs and NaN gaps. Under an HDR window the color is boosted in
// linear light; the transfer bodies mirror core/Ui/Misc/HdrTransfer.h line for line and
// tst_hdr_transfer pins the contract.

#version 440

layout(location = 0) in vec2 pixel;
layout(location = 1) in vec4 seg;
layout(location = 2) in float capFlags;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 color;
    float opacity;
    float face;
    float feather;
    float intensity;
} ubuf;

const float kSrgbLinearCut  = 0.0031308;
const float kSrgbEncodedCut = 0.04045;
const float kInvSrgbSlope   = 1.0 / 12.92;
const float kInvSrgbScale   = 1.0 / 1.055;
const float kInvSrgbGamma   = 1.0 / 2.4;

float eotfExt(float encoded)
{
    if (encoded <= 0.0)
        return 0.0;
    if (encoded <= kSrgbEncodedCut)
        return encoded * kInvSrgbSlope;
    if (encoded <= 1.0)
        return pow((encoded + 0.055) * kInvSrgbScale, 2.4);
    return encoded;
}

float oetfExt(float lin)
{
    if (lin <= 0.0)
        return 0.0;
    if (lin <= kSrgbLinearCut)
        return lin * 12.92;
    if (lin <= 1.0)
        return 1.055 * pow(lin, kInvSrgbGamma) - 0.055;
    return lin;
}

float boostEncoded(float encoded, float intensity)
{
    return oetfExt(eotfExt(encoded) * max(1.0, intensity));
}

void main()
{
    vec2 a = seg.xy;
    vec2 ab = seg.zw - seg.xy;
    float len2 = dot(ab, ab);
    float t = len2 > 0.0 ? dot(pixel - a, ab) / len2 : 0.0;

    int flags = int(capFlags + 0.5);
    if ((t < 0.0 && (flags & 1) != 0) || (t > 1.0 && (flags & 2) != 0))
        discard;

    float d = length(pixel - (a + clamp(t, 0.0, 1.0) * ab));
    float coverage = clamp((ubuf.face + ubuf.feather - d) / ubuf.feather, 0.0, 1.0);
    if (coverage <= 0.0)
        discard;

    float alpha = ubuf.color.a * coverage * ubuf.opacity;
    vec3 rgb = ubuf.color.rgb;
    if (ubuf.intensity > 1.0)
        rgb = vec3(boostEncoded(rgb.r, ubuf.intensity),
                   boostEncoded(rgb.g, ubuf.intensity),
                   boostEncoded(rgb.b, ubuf.intensity));

    fragColor = vec4(rgb * alpha, alpha);
}
