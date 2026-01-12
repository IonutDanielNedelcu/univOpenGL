#version 330 core

in float Life;
out vec4 out_Color;

void main() {
    // gri semi-transparent
    out_Color = vec4(0.30, 0.30, 0.30, 0.60);
}
