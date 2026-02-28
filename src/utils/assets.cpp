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
    a.texSkydome = carregaTextura("assets/Va4wUMQ.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");

    // --- BOSS 1 ('J') - Júlio ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    // --- BOSS 2 ('T') - Thiago ---
    a.texEnemies[1] = carregaTextura("assets/enemies/enemy2.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyRage2.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyRageDamage2.png");

    // --- BOSS 3 ('M') - Marco Leal ---
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