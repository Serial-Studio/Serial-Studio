// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
//
// Vertex half of the signed-distance plot stroke (spec 0097): passes each quad corner, the
// segment it belongs to and its flat-cap flags through to the fragment stage, which turns the
// distance to the segment into coverage. The uniform block mirrors SdfStrokeMaterial.cpp.

#version 440

layout(location = 0) in vec2 vertexCoord;
layout(location = 1) in vec4 segment;
layout(location = 2) in float caps;

layout(location = 0) out vec2 pixel;
layout(location = 1) out vec4 seg;
layout(location = 2) out float capFlags;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 color;
    float opacity;
    float face;
    float feather;
    float intensity;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    pixel = vertexCoord;
    seg = segment;
    capFlags = caps;
    gl_Position = ubuf.matrix * vec4(vertexCoord, 0.0, 1.0);
}
