// Serial Studio - https://serial-studio.com/
//
// SPDX-FileCopyrightText: 2020-2026 Alex Spataru <https://aspatru.com>
// SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
//
// This file is part of the proprietary features of Serial Studio and must not be used or
// included in builds distributed under the GNU General Public License unless explicitly
// permitted by a commercial agreement.
//
// Fragment half of the stereo stroke shader. The channel isolation that makes the two eyes
// independent is a pipeline color-write mask, not a shader branch, so this stage stays a
// straight pass-through of the interpolated premultiplied vertex color.

#version 440

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = color;
}
