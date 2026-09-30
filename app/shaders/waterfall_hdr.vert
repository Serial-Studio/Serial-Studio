// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
//
// This file is part of the proprietary features of Serial Studio and must not be used or
// included in builds distributed under the GNU General Public License unless explicitly
// permitted by a commercial agreement.
//
// Vertex half of the HDR spectrogram material (spec 0089): a plain textured quad; the ring
// scroll stays a texture-coordinate offset exactly as on the SDR path.

#version 440

layout(location = 0) in vec2 vertexCoord;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 uv;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
    float boost;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = ubuf.matrix * vec4(vertexCoord, 0.0, 1.0);
    uv = texCoord;
}
