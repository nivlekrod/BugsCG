#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    // --- TELAS ---
    a.texMenuBG = carregaTextura("assets/menu_bg.png");
    a.texTelaWin = carregaTextura("assets/telaWin.png");
    a.texTelaFinal = carregaTextura("assets/gamezerado.png");

    // --- CENÁRIO E SHADERS ---
    a.texChao = carregaTextura("assets/181.png");
    a.texParede = carregaTextura("assets/091.png");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texChaoInterno = carregaTextura("assets/100.png");
    a.texParedeInterna = carregaTextura("assets/060.png");
    a.texTeto = carregaTextura("assets/081.png");
    a.texLightOn = carregaTextura("assets/081_on.png");
    a.texSkydome = carregaTextura("assets/Va4wUMQ.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");
    a.progFogo = criaShader("shaders/fire.vert", "shaders/fire.frag");

    // --- BOSS 1 ('J') - Joaninha ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    // --- BOSS 2 ('G') - Grilo ---
    a.texEnemies[1] = carregaTextura("assets/enemies/enemy2.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyRage2.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyRageDamage2.png");

    // --- BOSS 3 ('M') - Mosca ---
    a.texEnemies[2] = carregaTextura("assets/enemies/enemy3.png");
    a.texEnemiesRage[2] = carregaTextura("assets/enemies/enemyRage3.png");
    a.texEnemiesDamage[2] = carregaTextura("assets/enemies/enemyRageDamage3.png");

    // --- INIMIGO EXTRA ('K') ---
    a.texEnemies[3] = carregaTextura("assets/enemies/enemy4.png");
    a.texEnemiesRage[3] = carregaTextura("assets/enemies/enemyRage4.png");
    a.texEnemiesDamage[3] = carregaTextura("assets/enemies/enemyRageDamage4.png");

    // --- COLETÁVEL: HD ('H') ---
    a.texEnemies[4] = carregaTextura("assets/hd.png");
    a.texEnemiesRage[4] = a.texEnemies[4];   // HD não tem raiva
    a.texEnemiesDamage[4] = a.texEnemies[4]; // HD não toma dano

    // --- OVERLAYS DE TELA (Efeitos) ---
    a.texHealthOverlay = carregaTextura("assets/heal.png");
    a.texDamage = carregaTextura("assets/damage.png");

    // Substitua "hudTex" pelo nome da variável que o seu código usa para os Assets (ex: assets.hudTex)
    a.texGunDefault = carregaTextura("assets/gun_default.png");
    a.texGunFire1 = carregaTextura("assets/gun_fire1.png");
    a.texGunFire2 = carregaTextura("assets/gun_fire2.png");
    a.texGunReload1 = carregaTextura("assets/gun_reload1.png");
    a.texGunReload2 = carregaTextura("assets/gun_reload2.png");
    a.texGunHUD = carregaTextura("assets/Shotgun.png");
    a.texItemAmmo = carregaTextura("assets/066.png"); // Textura de caixa de madeira
    a.texPilar = carregaTextura("assets/565.png");     // Pilares do altar
    a.texAltarChao = carregaTextura("assets/190.png"); // Piso do altar (degraus)


    // --- VERIFICAÇÃO DE ERROS ---
    // (Removidos os checks de armas e itens de chão que não usamos mais)
    if (!a.texChao || !a.texParede || !a.texSangue || !a.texLava || !a.progSangue ||
        !a.texHealthOverlay || !a.texDamage || !a.texMenuBG || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2])
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }

    return true;
}

void reloadPhaseTextures(GameAssets &a, int fase)
{
    std::printf("[DEBUG] reloadPhaseTextures chamada com fase=%d\n", fase);
    if (fase == 2)
    {
        a.texParedeInterna = carregaTextura("assets/backrooms-wall-diffuse.png");
        a.texChaoInterno   = carregaTextura("assets/backrooms-carpet-diffuse.png");
        a.texTeto          = carregaTextura("assets/backrooms-ceiling-tile-diffuse.png");
        a.texLightOn       = carregaTextura("assets/backrooms-ceiling-light-diffuse.png");
        a.texSkydome       = carregaTextura("assets/Va4wUMQ.png");
        std::printf("[DEBUG] Backrooms textures: wall=%u floor=%u ceil=%u light=%u\n",
            a.texParedeInterna, a.texChaoInterno, a.texTeto, a.texLightOn);
    }
    else if (fase == 3)
    {
        a.texParedeInterna = carregaTextura("assets/060.png");
        a.texChaoInterno   = carregaTextura("assets/100.png");
        a.texTeto          = carregaTextura("assets/081.png");
        a.texLightOn       = carregaTextura("assets/081_on.png");
        a.texSkydome       = carregaTextura("assets/Y1m2NAI.png");
        a.texPilar         = carregaTextura("assets/565.png");
        a.texAltarChao     = carregaTextura("assets/190.png");
    }
    else
    {
        a.texParedeInterna = carregaTextura("assets/060.png");
        a.texChaoInterno   = carregaTextura("assets/100.png");
        a.texTeto          = carregaTextura("assets/081.png");
        a.texLightOn       = carregaTextura("assets/081_on.png");
        a.texSkydome       = carregaTextura("assets/Va4wUMQ.png");
    }
}