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
    GLuint texLightOn = 0;
    GLuint texSkydome = 0;

    // --- Efeitos de Tela (HUD Terror) ---
    GLuint texHealthOverlay = 0; // Flash de cura/transição
    GLuint texDamage = 0;        // Flash de dano (Boss te bateu)

    // --- Entidades (Bosses e Notebooks) ---
    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};

    // VARIÁVEIS A ADICIONAR PARA A ARMA:
    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;
    GLuint texItemAmmo = 0;
    GLuint texPilar = 0;       // 565.png - pilares do altar
    GLuint texAltarChao = 0;   // 190.png - piso do altar

    GLuint texPortal = 0;      // portal.png - portal de transição

    // VARIÁVEIS PARA A BARRA INFERIOR (HUD TIPO DOOM)
    GLuint texGunHUD = 0;   // Ícone da arma na barra
    GLuint texHudFundo = 0; // Fundo da barra

    // --- Shaders ---
    GLuint progSangue = 0;
    GLuint progLava = 0;
    GLuint progFogo = 0;
    GLuint progPortal = 0;
};

bool loadAssets(GameAssets &a);
void reloadPhaseTextures(GameAssets &a, int fase);