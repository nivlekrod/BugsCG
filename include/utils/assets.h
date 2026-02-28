#pragma once
#include <GL/glew.h>

struct GameAssets
{
    // --- Telas e Menus ---
    GLuint texMenuBG = 0;
    GLuint texTelaWin = 0;
    GLuint texTelaFinal = 0;

    // --- Cenário ---
    GLuint texChao = 0;
    GLuint texParede = 0;
    GLuint texSangue = 0;
    GLuint texLava = 0;
    GLuint texChaoInterno = 0;
    GLuint texParedeInterna = 0;
    GLuint texTeto = 0;
    GLuint texSkydome = 0;

    // --- Efeitos de Tela (HUD Terror) ---
    GLuint texHealthOverlay = 0; // Flash de cura/transição
    GLuint texDamage = 0;        // Flash de dano (Boss te bateu)

    // --- Entidades (Bosses e HDs) ---
    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};

    // --- Shaders ---
    GLuint progSangue = 0;
    GLuint progLava = 0;
};

bool loadAssets(GameAssets &a);