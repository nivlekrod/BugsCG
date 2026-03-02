#version 120

uniform sampler2D uTexture;
uniform float uTime;

varying vec2 vTexCoord;

void main()
{
    // Converte UV para coordenadas polares centradas
    vec2 center = vec2(0.5, 0.5);
    vec2 uv = vTexCoord - center;
    float dist = length(uv);
    float angle = atan(uv.y, uv.x);

    // Rotação do redemoinho: mais forte no centro
    float swirl = 3.0 * (1.0 - smoothstep(0.0, 0.5, dist));
    angle += swirl * sin(uTime * 1.5) + uTime * 2.0;

    // Reconstroi UV com a rotação aplicada
    vec2 swirlUV = center + dist * vec2(cos(angle), sin(angle));

    // Distorção ondulante adicional
    float wave = sin(dist * 20.0 - uTime * 4.0) * 0.02;
    swirlUV += wave * normalize(uv + 0.001);

    vec4 texColor = texture2D(uTexture, swirlUV);

    // Pulso de brilho
    float pulse = 0.8 + 0.2 * sin(uTime * 3.0);
    
    // Glow verde no centro
    float glow = smoothstep(0.45, 0.0, dist) * 0.6;
    vec3 glowColor = vec3(0.2, 1.0, 0.3) * glow * pulse;

    vec3 finalColor = texColor.rgb * pulse + glowColor;

    // Fade circular nas bordas
    float edgeFade = smoothstep(0.52, 0.42, dist);

    gl_FragColor = vec4(finalColor, texColor.a * edgeFade);
}
