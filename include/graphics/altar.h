#pragma once
#include <GL/glew.h>

struct RenderAssets;

// Constantes exportadas para uso em drawlevel (iluminação dos losangos)
static const int NUM_PILARES_ALTAR = 4;
static const float RAIO_ALTAR = 14.0f;

// Desenha o altar (4 pilares com losangos + pirâmide de degraus + esfera) na posição especificada
void drawAltar(float centerX, float centerZ, const RenderAssets &r, float time);

// Verifica se a posição (px,pz) colide com a geometria do altar (pirâmide + pilares)
bool altarBlocksMovement(float centerX, float centerZ, float px, float pz, float playerRadius);

// Calcula as posições XZ dos 4 pilares em coordenadas de mundo
void getAltarPillarPositions(float centerX, float centerZ, float outX[4], float outZ[4]);

// Sistema de partículas de fogo verde (chamar uma vez, atualizar por frame)
void iniciaParticulasFogo(int maxParticulas);
void atualizaParticulasFogo(float dt);
