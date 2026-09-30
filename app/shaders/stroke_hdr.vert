// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Vertex half of the emissive stroke material (spec 0089). Same contract as Qt's vertex-color
// material -- premultiplied vertex colors scaled by opacity -- vendored so the uniform block
// and the writer in StrokeHdrMaterial.cpp cannot drift apart.

#version 440

layout(location = 0) in vec2 vertexCoord;
layout(location = 1) in vec4 vertexColor;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
    float intensity;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = ubuf.matrix * vec4(vertexCoord, 0.0, 1.0);
    color = vertexColor * ubuf.opacity;
}
