#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 inViewPos;
in vec3 inLightPos;
in vec4 ex_Color;
in vec2 tex_Coord; 

out vec4 out_Color;

uniform sampler2D myTexture;
uniform float globalColorMul;
uniform float globalAlphaMul;
uniform vec3 fireLightPos;
uniform vec3 fireLightColor;
uniform float fireLightIntensity;

// Bulb lights
uniform int bulbCount;
const int MAX_BULBS = 128;
uniform vec3 bulbPos[MAX_BULBS];
uniform vec3 bulbColor[MAX_BULBS];
uniform float bulbIntensity[MAX_BULBS];

// Smoke uniforms
uniform vec3 smokeRegionMin;
uniform vec3 smokeRegionMax;
uniform float smokeRange;
uniform vec3 smokeColor;
uniform float smokeIntensity;

// Flags
uniform int allowBlack;
uniform int useSolidColor;
uniform vec3 solidColor;

void main(void)
{
    // 1. Becuri fizice (Solid Color)
    if (useSolidColor == 1) {
        out_Color = vec4(solidColor, 1.0);
        return; 
    }

    vec4 texColor = texture(myTexture, tex_Coord);
    
    // Filtre
    if(texColor.a < 0.1) discard;
    if (allowBlack == 0) {
        if(texColor.r < 0.1 && texColor.g < 0.1 && texColor.b < 0.1) discard;
    }
    
    vec3 objectColor = texColor.rgb; 
    
    // --- 2. LUMINA GLOBALA ---
    vec3 globalLightColor = vec3(0.4, 0.4, 0.5); 
    vec3 ambient = 0.8 * globalLightColor;
    vec3 norm = normalize(Normal);
    vec3 moonDir = normalize(vec3(0.3, -1.0, 0.5));
    float diff = max(dot(norm, -moonDir), 0.0); 
    vec3 diffuse = diff * globalLightColor;
    
    vec3 result = (ambient + diffuse) * objectColor;

    // --- 3. ILUMINAREA DIN INSTALATIE ---
    for (int i = 0; i < bulbCount && i < MAX_BULBS; ++i) {
        vec3 bdir = bulbPos[i] - FragPos;
        float bdist = length(bdir);
        // Atenuare lenta
        float batt = bulbIntensity[i] / (1.0 + 0.05 * bdist + 0.01 * bdist * bdist);
        vec3 lightColor = bulbColor[i];
        float bdot = max(dot(norm, normalize(bdir)), 0.0);
        
        // Aici pastram inmultirea cu objectColor pentru ca vrem ca becurile 
        // sa ilumineze bradul si mobila realist
        result += (lightColor * objectColor) * bdot * batt * 1.5;
        result += lightColor * batt * 0.15; 
    }

    // --- 4. ILUMINARE FOC (REVENIRE LA VARIANTA VECHE) ---
    vec3 fireDir = fireLightPos - FragPos;
    float fireDist = length(fireDir);
    float fireAtt = fireLightIntensity / (1.0 + 1.0 * fireDist + 0.5 * fireDist * fireDist);
    vec3 fireL = fireLightColor * fireAtt;
    float fireDot = max(dot(norm, normalize(fireDir)), 0.0);
    
    // MODIFICARE AICI: Am scos " * objectColor ".
    // Acum focul se adauga DIRECT peste imagine.
    // Daca bradul e verde, focul tot PORTOCALIU ramane.
    result += fireL * fireDot * 1.5; // * 1.5 pentru stralucire
    result += fireL * 0.5;           // Glow ambiental mai puternic in jurul focului

    // --- 5. No smoke in main shader (smoke rendered by particle system)
    float finalAlpha = 1.0;
    vec3 finalCol = result * globalColorMul;

    out_Color = vec4(finalCol, clamp(finalAlpha * globalAlphaMul, 0.0, 1.0));
}