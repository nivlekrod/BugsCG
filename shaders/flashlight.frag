#version 120
varying vec2 TexCoord;
varying vec3 FragPos; 
varying vec3 Normal;
varying float Dist;  
varying vec3 WorldPos;

uniform sampler2D uTexture;
uniform int uFlashlightOn;
uniform vec2 uLavaCenter;
uniform float uLavaFlicker;      // 0 = sem lava, >0 = intensidade do flicker
uniform vec3 uLampData[4];       // xy = posição mundo XZ, z = intensidade (0 = bloqueado/off)
uniform int uIsSprite;           // 1 = sprite (usa uEntityWorldXZ), 0 = tile (usa WorldPos)
uniform vec2 uEntityWorldXZ;    // posição XZ no mundo do sprite

void main() {
    vec4 texColor = texture2D(uTexture, TexCoord);

    vec3 lightDir = vec3(0.0, 0.0, -1.0);
    vec3 lDir = normalize(-FragPos); 
    
    float theta = dot(lDir, normalize(-lightDir));
    float cutOff = 0.94;      
    float outerCutOff = 0.88; 
    
    float epsilon = cutOff - outerCutOff;
    
    // --- LÓGICA DE LIGAR/DESLIGAR ---
    float intensity = 0.0;
    if (uFlashlightOn == 1) {
        intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    }
    
    float attenuation = 1.0 / (1.0 + 0.1 * Dist + 0.02 * (Dist * Dist));
    
    vec3 ambient = vec3(0.005, 0.005, 0.01); // Breu absoluto
    vec3 diffuse = vec3(1.0, 0.95, 0.8) * intensity * attenuation; 

    // Posição XZ para cálculos de iluminação
    vec2 worldXZ = (uIsSprite == 1) ? uEntityWorldXZ : vec2(WorldPos.x, WorldPos.z);

    // Iluminacao da lava (circular, per-fragment)
    vec3 lavaContrib = vec3(0.0);
    if (uLavaFlicker > 0.0) {
        float ld = distance(worldXZ, uLavaCenter);
        float ls = 1.0 - clamp(ld / 28.0, 0.0, 1.0);
        ls *= ls;
        lavaContrib = vec3(0.8, 0.3, 0.05) * ls * uLavaFlicker;
    }

    // Iluminacao das luminarias (circular, per-fragment)
    vec3 lampContrib = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        if (uLampData[i].z <= 0.0) continue;
        float dist = distance(worldXZ, uLampData[i].xy);
        float str = 1.0 - clamp(dist / 16.0, 0.0, 1.0);
        str *= str;
        lampContrib += vec3(0.8, 0.8, 0.85) * str * uLampData[i].z * 0.6;
    }
    
    float fogFactor = clamp((Dist - 3.0) / 15.0, 0.0, 1.0);
    
    vec3 finalColor = texColor.rgb * (ambient + diffuse + lavaContrib + lampContrib);
    finalColor = mix(finalColor, vec3(0.0, 0.0, 0.0), fogFactor);

    gl_FragColor = vec4(finalColor, texColor.a);
}