#version 450

layout(location = 0) in vec3 i_Pos;
layout(location = 1) in vec3 i_Color;

layout(location = 0) out vec3 o_Color;

void main() {
    gl_Position = vec4(i_Pos, 1.0);
    o_Color = i_Color;
}