#version 450

layout(location = 0) in vec3 o_Color;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D u_Texture;

void main() {
    //outColor = vec4(o_Color * texture(u_Texture, vec2(0, 0)).rgb, 1.0);
    outColor = texture(u_Texture, vec2(0, 0));
}