// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Emissive boost for HdrBoost.qml (spec 0089): re-renders a hidden SDR source with its linear
// light multiplied by the intensity uniform, so the window's output transform lands it at
// intensity x SDR white. Works on premultiplied texels: un-premultiply, boost each channel in
// linear light, re-encode, re-premultiply. The transfer bodies mirror
// core/Ui/Misc/HdrTransfer.h line for line; tst_hdr_transfer pins the contract.

#version 440

layout(location = 0) in vec2 qt_TexCoord0;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float boost;
} ubuf;

layout(binding = 1) uniform sampler2D source;

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
    vec4 texel = texture(source, qt_TexCoord0);
    float alpha = texel.a;
    vec3 rgb = alpha > 0.0 ? texel.rgb / alpha : vec3(0.0);
    vec3 boosted = vec3(boostEncoded(rgb.r, ubuf.boost),
                        boostEncoded(rgb.g, ubuf.boost),
                        boostEncoded(rgb.b, ubuf.boost));
    fragColor = vec4(boosted * alpha, alpha) * ubuf.qt_Opacity;
}
