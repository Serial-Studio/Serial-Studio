// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
//
// This file is part of the proprietary features of Serial Studio and must not be used or
// included in builds distributed under the GNU General Public License unless explicitly
// permitted by a commercial agreement.
//
// Fragment half of the HDR spectrogram material (spec 0089): the ring texture holds 16-bit
// normalized magnitude, the colormap is a 256x1 LUT sampled with filtering (no CPU bake, no
// 8-bit magnitude quantization), and the top ~20% of scale ramps to the emissive boost. The
// transfer bodies mirror core/Ui/Misc/HdrTransfer.h line for line; tst_hdr_transfer pins the
// contract.

#version 440

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
    float boost;
} ubuf;

layout(binding = 1) uniform sampler2D ring;
layout(binding = 2) uniform sampler2D lut;

const float kSrgbLinearCut  = 0.0031308;
const float kSrgbEncodedCut = 0.04045;
const float kInvSrgbSlope   = 1.0 / 12.92;
const float kInvSrgbScale   = 1.0 / 1.055;
const float kInvSrgbGamma   = 1.0 / 2.4;
const float kBoostRampStart = 0.8;

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

void main()
{
    float mag = texture(ring, uv).r;
    vec3 color = texture(lut, vec2(mag, 0.5)).rgb;
    float factor = 1.0 + (max(1.0, ubuf.boost) - 1.0)
                         * smoothstep(kBoostRampStart, 1.0, mag);
    vec3 boosted = vec3(oetfExt(eotfExt(color.r) * factor),
                        oetfExt(eotfExt(color.g) * factor),
                        oetfExt(eotfExt(color.b) * factor));
    fragColor = vec4(boosted, 1.0) * ubuf.opacity;
}
