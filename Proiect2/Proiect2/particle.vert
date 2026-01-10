#version 330 core

layout(location=0) in vec3 in_Position;
layout(location=1) in float in_Life;
layout(location=2) in float in_Size;

out float Life;

uniform mat4 view;
uniform mat4 projection;

void main() {
    Life = in_Life;
    gl_Position = projection * view * vec4(in_Position, 1.0);
    gl_PointSize = in_Size;
}
