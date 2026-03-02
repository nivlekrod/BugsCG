#pragma once
#include "core/game_enums.h"
// #include "core/game_state.h"
#include <GL/glew.h>

// Adicione a estrutura da animação da arma:
struct WeaponAnim
{
    WeaponState state = WeaponState::W_IDLE;
    float timer = 0.0f;
};

struct PlayerState
{
    int health = 100;
    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    // VARIÁVEIS A ADICIONAR PARA O FPS (Munição):
    int currentAmmo = 12;
    int reserveAmmo = 25;

    // Variáveis para aviso na tela de drop
    bool showAmmoDropWarning = false;
    float ammoDropWarningTimer = 0.0f;
};



struct RenderAssets
{
    GLuint texChao = 0, texParede = 0, texSangue = 0, texLava = 0;
    GLuint texChaoInterno = 0, texParedeInterna = 0, texTeto = 0, texLightOn = 0, texSkydome = 0, texMenuBG = 0, texTelaWin = 0, texTelaFinal = 0;
    GLuint texItemAmmo = 0;
    GLuint texPilar = 0;       // 565.png - textura dos pilares do altar
    GLuint texAltarChao = 0;   // 190.png - textura do piso do altar

    GLuint texEnemies[5] = {0};
    GLuint texEnemiesRage[5] = {0};
    GLuint texEnemiesDamage[5] = {0};



    GLuint progSangue = 0;
    GLuint progLava = 0;
    GLuint progFogo = 0;
};

struct GameContext
{
    GameState state = GameState::MENU_INICIAL;
    PlayerState player;
    // VARIÁVEL A ADICIONAR (Máquina de estado da arma):
    WeaponAnim weapon;

    float time = 0.0f;

    RenderAssets r;
};
