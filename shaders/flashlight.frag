#version 120
varying vec2 TexCoord;
varying vec3 FragPos; 
varying vec3 Normal;
varying float Dist;  

uniform sampler2D uTexture;
uniform int uFlashlightOn; // <--- NOVA VARIÁVEL

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
    
    float fogFactor = clamp((Dist - 3.0) / 15.0, 0.0, 1.0);
    
    vec3 finalColor = texColor.rgb * (ambient + diffuse);
    finalColor = mix(finalColor, vec3(0.0, 0.0, 0.0), fogFactor);

    gl_FragColor = vec4(finalColor, texColor.a);
}