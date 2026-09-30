// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Output transform for HDR windows (spec 0089): the whole window composites in encoded sRGB
// inside an RGBA16F layer (byte-identical to SDR rendering), and this fullscreen pass owns the
// conversion the 2D scene graph never does -- sRGB to linear, >1.0 emissive pass-through, and
// the Windows SDR-white multiply (1.0 on display-referred macOS). The transfer bodies mirror
// core/Ui/Misc/HdrTransfer.h line for line; tst_hdr_transfer pins the contract.

#version 440

layout(location = 0) in vec2 qt_TexCoord0;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float sdrWhiteScale;
} ubuf;

layout(binding = 1) uniform sampler2D source;

const float kSrgbEncodedCut = 0.04045;
const float kInvSrgbSlope   = 1.0 / 12.92;
const float kInvSrgbScale   = 1.0 / 1.055;

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

void main()
{
    vec4 texel = texture(source, qt_TexCoord0);
    float alpha = texel.a;
    vec3 rgb = alpha > 0.0 ? texel.rgb / alpha : vec3(0.0);
    vec3 lin = vec3(eotfExt(rgb.r), eotfExt(rgb.g), eotfExt(rgb.b)) * ubuf.sdrWhiteScale;
    fragColor = vec4(lin * alpha, alpha) * ubuf.qt_Opacity;
}
