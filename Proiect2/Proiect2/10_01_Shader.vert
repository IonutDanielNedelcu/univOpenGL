#version 330 core

// Atribute de intrare
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_TexCoord; // Aici intra coordonatele UV din OBJ

// Variabile de iesire catre Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out vec3 inViewPos;
out vec4 ex_Color;
out vec2 tex_Coord; // Aici le trimitem mai departe

uniform mat4 myMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos; // Pozitia camerei (pt specular)

void main(void)
{
    gl_Position = projection * view * myMatrix * vec4(in_Position, 1.0);
    
    FragPos = mat3(myMatrix) * in_Position;
    Normal = mat3(myMatrix) * in_Normal;
    
    inViewPos = viewPos;
    ex_Color = vec4(1.0, 1.0, 1.0, 1.0); // Culoare default alba

    // Transmiterea coordonatelor de texturare
    // Uneori texturile sunt rasturnate, deci folosim 1.0 - y daca e cazul
    tex_Coord = vec2(in_TexCoord.x, 1.0 - in_TexCoord.y); 
}
