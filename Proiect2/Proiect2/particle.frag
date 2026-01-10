#version 330 core

in float Life;
out vec4 out_Color;

void main() {
    // Simple visible grey color (opaque) to replace red debug
    out_Color = vec4(0.30, 0.30, 0.30, 0.60);
}
