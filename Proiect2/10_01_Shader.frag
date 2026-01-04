#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 inViewPos;
in vec3 inLightPos;
in vec4 ex_Color;
in vec2 tex_Coord; 

out vec4 out_Color;

uniform sampler2D myTexture;

void main(void)
{
    vec4 texColor = texture(myTexture, tex_Coord);
    
    // --- FILTRU AVANSAT ---
    // 1. Daca are Alpha mic (transparenta reala) -> ARUNCA
    if(texColor.a < 0.1)
        discard;

    // 2. Daca este o textura proasta (JPG) cu fundal NEGRU -> ARUNCA
    // Verificam daca rosu+verde+albastru sunt foarte mici (aproape negru)
    if(texColor.r < 0.1 && texColor.g < 0.1 && texColor.b < 0.1)
        discard;
    
    // 3. Daca este o textura cu fundal ALB (unele png-uri) -> ARUNCA
    // (Activeaza liniile de mai jos doar daca bradul are contur alb)
    // if(texColor.r > 0.9 && texColor.g > 0.9 && texColor.b > 0.9)
    //    discard;

    vec3 objectColor = texColor.rgb; 
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Ambient mai puternic pentru brad
    float ambientStrength = 0.6; 
    vec3 ambient = ambientStrength * lightColor;
  	
    // Iluminare Fata-Verso (Two Sided Lighting)
    // abs() asigura ca si spatele frunzei e luminat
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(inLightPos - FragPos);
    float diff = abs(dot(norm, lightDir)); 
    vec3 diffuse = diff * lightColor;
    
    // Specular scazut (frunzele nu sclipesc tare)
    vec3 viewDir = normalize(inViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    vec3 specular = 0.1 * spec * lightColor; 
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    out_Color = vec4(result, 1.0);
}