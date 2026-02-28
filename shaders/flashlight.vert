#version 120

// Variáveis que enviaremos para o Fragment Shader
varying vec2 TexCoord;
varying vec3 FragPos; // Posição do pixel em relação à câmera
varying vec3 Normal;
varying float Dist;   // Distância pura para neblina

void main()
{
    // Calcula a posição do vértice no espaço da câmera (View Space)
    // Nesse espaço, a câmera está SEMPRE no (0,0,0)
    vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
    FragPos = viewPos.xyz;
    
    // Passa a textura e normal originais
    TexCoord = gl_MultiTexCoord0.xy;
    Normal = gl_NormalMatrix * gl_Normal;
    
    // Calcula distância para neblina (fog)
    Dist = length(viewPos.xyz);

    // Posição final na tela
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}