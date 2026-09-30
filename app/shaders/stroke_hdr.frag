// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Fragment half of the emissive stroke material (spec 0089): boosts the interpolated
// premultiplied vertex color in linear light so curve strokes exceed SDR white under the
// window's output transform. The transfer bodies mirror core/Ui/Misc/HdrTransfer.h line for
// line; tst_hdr_transfer pins the contract.

#version 440

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
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
    float alpha = color.a;
    vec3 rgb = alpha > 0.0 ? color.rgb / alpha : vec3(0.0);
    vec3 boosted = vec3(boostEncoded(rgb.r, ubuf.intensity),
                        boostEncoded(rgb.g, ubuf.intensity),
                        boostEncoded(rgb.b, ubuf.intensity));
    fragColor = vec4(boosted * alpha, alpha);
}
