// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
//
// This file is part of the proprietary features of Serial Studio and must not be used or
// included in builds distributed under the GNU General Public License unless explicitly
// permitted by a commercial agreement.
//
// Vertex-colored stroke shader for the 3D plot's stereo passes. Vendored rather than borrowed
// from Qt's built-in scene-graph set so the uniform block below and the writer that fills it
// are both ours, and neither can drift when Qt reorganizes its own shaders.

#version 440

layout(location = 0) in vec2 vertexCoord;
layout(location = 1) in vec4 vertexColor;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = ubuf.matrix * vec4(vertexCoord, 0.0, 1.0);
    color = vertexColor * ubuf.opacity;
}
