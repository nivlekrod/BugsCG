#include <GL/glew.h> 
#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "core/game_enums.h"
#include "core/game_state.h"
#include "core/game.h"
#include "level/level.h"
#include "core/camera.h"
#include "input/input.h"
#include "input/keystate.h"
#include "graphics/drawlevel.h"
#include "graphics/skybox.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "graphics/lighting.h"
#include "core/movement.h"
#include "core/player.h"
#include "core/entities.h"
#include "audio/audio_system.h"
#include "utils/assets.h"
#include "core/config.h"
#include "core/window.h"

// --- VARIÁVEIS DO DEVOUR ---
int componentesCarregados = 0;
int componentesQueimados = 0;
int faseAtual = 1;

static HudTextures gHudTex;
static GameContext g;
static GameAssets gAssets;
Level gLevel;
static AudioSystem gAudioSys;

GameContext &gameContext() { return g; }
AudioSystem &gameAudio() { return gAudioSys; }
Level &gameLevel() { return gLevel; }
GameState gameGetState() { return g.state; }
void gameSetState(GameState s) { g.state = s; }

void gameTogglePause()
{
    if (g.state == GameState::JOGANDO) g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO) g.state = GameState::JOGANDO;
}

bool gameInit(const char *mapPath)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 

    setupSunLightOnce();
    setupIndoorLightOnce();

    if (!loadAssets(gAssets)) return false;

    // Repassa assets para o renderizador
    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;
    g.r.texSkydome = gAssets.texSkydome;
    
    g.r.texMenuBG = gAssets.texMenuBG;
    g.r.texTelaWin = gAssets.texTelaWin;
    g.r.texTelaFinal = gAssets.texTelaFinal;
    
    // HUD de Terror (Overlay de dano e cura)
    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;
    gHudTex.texHD = gAssets.texEnemies[4];

    for (int i = 0; i < 5; i++) {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE)) return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);

    audioInit(gAudioSys, gLevel);

    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.player = PlayerState{};
    
    componentesCarregados = 0;
    componentesQueimados = 0;

    return true;
}

void gameReset()
{
    // Se a função foi chamada com 10 HDs queimados, é porque você GANHOU a fase!
    if (componentesQueimados >= 10) {
        if (faseAtual >= 3) faseAtual = 1; // Zerou o jogo? Recomeça a tortura
        else faseAtual++; // Sobe de fase!
    } else {
        // Se morreu no meio (Game Over) ou recomeçou do Menu, volta pra Fase 1
        faseAtual = 1; 
    }

    // --- 2. RESETA O JOGADOR ---
    g.player.health = 100;
    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;

    componentesCarregados = 0;
    componentesQueimados = 0;

    applySpawn(gLevel, camX, camZ); // Volta o player pro lugar de início

    // --- 3. RESETA O MAPA (Revive HDs e Troca o Boss) ---
    for (auto &en : gLevel.enemies) {
        // Devolve TODO MUNDO para a posição original que estava no mapa
        en.x = en.startX;
        en.z = en.startZ;
        en.state = STATE_IDLE; // Revive os HDs que tinham sumido
        en.attackCooldown = 0.0f;
        en.hurtTimer = 0.0f;

        // Se for um Boss (Tipos 0, 1 ou 2), muda a skin dele para a fase atual!
        if (en.type == 0 || en.type == 1 || en.type == 2) {
            if (faseAtual == 1) en.type = 2;      // Fase 1: Marco Leal
            else if (faseAtual == 2) en.type = 1; // Fase 2: Thiago
            else if (faseAtual >= 3) en.type = 0; // Fase 3: Júlio
        }
    }
}

void gameUpdate(float dt)
{
    g.time += dt;

    if (g.state != GameState::JOGANDO) return;

    atualizaMovimento();

    AudioListener L; L.pos = {camX, camY, camZ};
    float ry = yaw * 3.14159f / 180.0f; float rp = pitch * 3.14159f / 180.0f;
    L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    L.up = {0.0f, 1.0f, 0.0f}; L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    if (g.player.damageAlpha > 0.0f) {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f) g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f) {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f) g.player.healthAlpha = 0.0f;
    }

    updateEntities(dt);

    if (componentesQueimados >= 10) {
        if (faseAtual >= 3) {
            g.state = GameState::JOGO_ZERADO;
        } else {
            g.state = GameState::FASE_CONCLUIDA;
        }
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    }
    
    if (g.player.health <= 0) {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    }
}

void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glEnable(GL_TEXTURE_2D); glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);

    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    setSunDirectionEachFrame();
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);
    
    // CORRIGIDO: Agora chamamos sem o "gLevel.items"
    drawEntities(gLevel.enemies, camX, camZ, dirX, dirZ, g.r);
}

void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    HudState hs;
    hs.playerHealth = (int)g.player.health;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.componentesCarregados = componentesCarregados;

    if (g.state == GameState::MENU_INICIAL) {
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Sobreviver", g.r);
    }
    else if (g.state == GameState::GAME_OVER) {
        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "SISTEMA CORROMPIDO", "Pressione ENTER para Reiniciar", g.r);
    }
    else if (g.state == GameState::PAUSADO) {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, false, false, true, componentesQueimados);
        pauseMenuRender(janelaW, janelaH, g.time);
    }
    else if (g.state == GameState::FASE_CONCLUIDA) {
        drawWorld3D(); 
        GLuint fundoOriginal = g.r.texMenuBG; 
        g.r.texMenuBG = g.r.texTelaWin; 
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para ir para a Fase 2", g.r);
        g.r.texMenuBG = fundoOriginal; 
    }
    else if (g.state == GameState::JOGO_ZERADO) {
        GLuint fundoOriginal = g.r.texMenuBG; 
        g.r.texMenuBG = g.r.texTelaFinal; 
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER", g.r);
        g.r.texMenuBG = fundoOriginal; 
    }
    else { 
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, false, false, true, componentesQueimados);
        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }
    glutSwapBuffers();
}