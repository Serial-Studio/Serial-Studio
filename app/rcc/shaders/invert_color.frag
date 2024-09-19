#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D source;

void main() {
    vec4 color = texture(source, qt_TexCoord0);
    color.rgb = vec3(1.0) - color.rgb;
    fragColor = vec4(color.rgb, color.a) * qt_Opacity;
}
