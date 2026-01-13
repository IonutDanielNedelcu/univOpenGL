#version 330 core

// Atribute de intrare
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_TexCoord;

// Variabile de iesire catre Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out vec3 inViewPos;
out vec4 ex_Color;
out vec2 tex_Coord; // Aici le trimitem mai departe

// Variabile uniforme
uniform mat4 myMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

void main(void)
{
    gl_Position = projection * view * myMatrix * vec4(in_Position, 1.0);
    
    FragPos = mat3(myMatrix) * in_Position;
    Normal = mat3(myMatrix) * in_Normal;

    // Transmiterea coordonatelor de texturare
    tex_Coord = vec2(in_TexCoord.x, 1.0 - in_TexCoord.y); 
}
