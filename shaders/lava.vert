#version 120

varying vec2 vTexCoord;
varying vec3 vWorldPos;

void main()
{
    // posição padrão usando a matriz fixa
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // pega as coordenadas de textura do pipeline fixo
    vTexCoord = gl_MultiTexCoord0.st;

    // posição no mundo para recorte circular
    vWorldPos = gl_Vertex.xyz;
}
