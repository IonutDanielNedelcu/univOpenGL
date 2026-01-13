#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 tex_Coord; 

in vec3 inLightPos;
in vec3 inViewPos;
in vec4 ex_Color;

out vec4 out_Color;

uniform sampler2D myTexture;
uniform float globalColorMul;
uniform float globalAlphaMul;

// Uniforme pentru foc
uniform vec3 fireLightPos;
uniform vec3 fireLightColor;
uniform float fireLightIntensity;

// Uniforme pentru becuri
uniform int bulbCount;
const int MAX_BULBS = 128;
uniform vec3 bulbPos[MAX_BULBS];
uniform vec3 bulbColor[MAX_BULBS];
uniform float bulbIntensity[MAX_BULBS];

// Uniforme pentru fum
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
    // Daca 'useSolidColor' e activat ignoram calculele de lumina
    if (useSolidColor == 1) {
        out_Color = vec4(solidColor, 1.0);
        return; 
    }

    vec4 texColor = texture(myTexture, tex_Coord);
    
    // Transparenta din PNG: daca pixelul e transparent in poza ii dam discard
    if(texColor.a < 0.1) discard;

    // Daca 'allowBlack' e 0 stergem tot ce e negru
    if (allowBlack == 0) {
        if(texColor.r < 0.1 && texColor.g < 0.1 && texColor.b < 0.1) discard;
    }
    
    vec3 objectColor = texColor.rgb; 
    // Incepem calculul luminii
    vec3 result = vec3(0.0);
    vec3 norm = normalize(Normal); // Directia suprafetei


    // Lumina ambientala: simulam o lumina albastra-rece nocturna
    vec3 moonColor = vec3(0.4, 0.4, 0.5); 
    vec3 ambient = 0.8 * moonColor;
    
    // Lumina difuza (directa)
    vec3 moonDir = normalize(vec3(0.3, -1.0, 0.5)); // Directia de unde vine lumina
    float diff = max(dot(norm, -moonDir), 0.0);     // Cat de direct loveste suprafata
    vec3 diffuse = diff * moonColor;
    
    // Adaugam lumina de baza peste culoarea obiectului
    result = (ambient + diffuse) * objectColor;


    // Lumina de la instalatie
    for (int i = 0; i < bulbCount && i < MAX_BULBS; ++i) {
        // Calculam vectorul de la bec la pixel
        vec3 lightDir = bulbPos[i] - FragPos;
        float distance = length(lightDir);
        
        // Calculam atenuarea 
        // Formula: Intensitate / (1 + distanta + distanta^2)
        float attenuation = bulbIntensity[i] / (1.0 + 0.05 * distance + 0.01 * distance * distance);

        // Cat de perpendicular loveste lumina becului suprafata
        float diffImpact = max(dot(norm, normalize(lightDir)), 0.0);

        // Adaugam lumina becului (CuloareBec * CuloareObiect)
        result += (bulbColor[i] * objectColor) * diffImpact * attenuation * 1.5;

        result += bulbColor[i] * attenuation * 0.15; 
    }


    // Lumina de la foc
    {
        vec3 fireDir = fireLightPos - FragPos;
        float fireDist = length(fireDir);
        
        // Atenuare pentru foc
        float fireAtt = fireLightIntensity / (1.0 + 1.0 * fireDist + 0.5 * fireDist * fireDist);
        
        vec3 fireL = fireLightColor * fireAtt;
        float fireDot = max(dot(norm, normalize(fireDir)), 0.0);
        
        // Adaugam culoarea focului fara sa o inmultim cu culoarea obiectului
        result += fireL * fireDot * 1.5;
        result += fireL * 0.5; 
    }


    // Aplicam multiplicatorul global (daca vrem sa stingem tot)
    vec3 finalCol = result * globalColorMul;
    float finalAlpha = 1.0; // Opacitate completa

    out_Color = vec4(finalCol, clamp(finalAlpha * globalAlphaMul, 0.0, 1.0));
}