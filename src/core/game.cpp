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
#include "graphics/altar.h"
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

// --- VARIÁVEIS DO BUGS ---
int componentesCarregados = 0;
int componentesQueimados = 0;
int faseAtual = 1;
bool doorActive = false;

// --- ESTADO DE INTERAÇÃO / CONTAGEM ---
bool g_pertoDoIncinerador = false;
int g_aliveEnemyCount = 0;
float g_enemySpawnNotifTimer = 0.0f;

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
    if (g.state == GameState::JOGANDO)
        g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO)
        g.state = GameState::JOGANDO;
}

bool gameInit(const char *mapPath)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    setupSunLightOnce();
    setupIndoorLightOnce();

    if (!loadAssets(gAssets))
        return false;

    // Repassa assets para o renderizador
    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;
    g.r.texLightOn = gAssets.texLightOn;
    g.r.texSkydome = gAssets.texSkydome;

    g.r.texMenuBG = gAssets.texMenuBG;
    g.r.texTelaWin = gAssets.texTelaWin;
    g.r.texTelaFinal = gAssets.texTelaFinal;

    // HUD de Terror (Overlay de dano e cura)
    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;
    gHudTex.texNotebook = gAssets.texEnemies[4];

    // Armas / FPS
    gHudTex.texGunDefault = gAssets.texGunDefault;
    gHudTex.texGunFire1 = gAssets.texGunFire1;
    gHudTex.texGunFire2 = gAssets.texGunFire2;
    gHudTex.texGunReload1 = gAssets.texGunReload1;
    gHudTex.texGunReload2 = gAssets.texGunReload2;
    gHudTex.texGunHUD = gAssets.texGunHUD;
    gHudTex.texHudFundo = gAssets.texHudFundo;
    g.r.texItemAmmo = gAssets.texItemAmmo;
    g.r.texPilar = gAssets.texPilar;
    g.r.texAltarChao = gAssets.texAltarChao;
    g.r.texPortal = gAssets.texPortal;
    g.r.progPortal = gAssets.progPortal;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;
    g.r.progFogo = gAssets.progFogo;

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    // Detecta a fase a partir do nome do mapa (ex: "maps/map2.txt" → fase 2)
    const char *p = mapPath;
    while (*p && !(*p >= '1' && *p <= '9')) p++;
    if (*p) faseAtual = *p - '0';

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;
    yaw = 180.0f; // Olhar para o sul (direção da lava)

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutSetCursor(GLUT_CURSOR_NONE);

    audioInit(gAudioSys, gLevel);

    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.player = PlayerState{};

    componentesCarregados = 0;
    componentesQueimados = 0;
    doorActive = false;

    applyPhaseTextures();

    // Inicializa sistema de partículas de fogo (para losangos do altar - fase 3)
    iniciaParticulasFogo(120); // 30 partículas por losango × 4 pilares

    return true;
}

void gameReset()
{
    // Se a função foi chamada com todos os Notebooks queimados, é porque você GANHOU a fase!
    if (componentesQueimados >= gLevel.totalNotebooks && gLevel.totalNotebooks > 0 && !doorActive)
    {
        if (faseAtual >= 3)
            faseAtual = 1; // Zerou o jogo? Recomeça a tortura
        else
            faseAtual++; // Sobe de fase!
    }
    else
    {
        // Se morreu no meio (Game Over) ou recomeçou do Menu, volta pra Fase 1
        faseAtual = 1;
    }

    // --- 2. RESETA O JOGADOR ---
    g.player.health = 100;
    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;
    g.player.currentAmmo = 12;
    g.player.reserveAmmo = 25;

    componentesCarregados = 0;
    componentesQueimados = 0;
    doorActive = false;

    applySpawn(gLevel, camX, camZ);// Volta o player pro lugar de início
    yaw = 180.0f;

    // --- 3. RESETA O MAPA (Revive Notebooks e Troca o Boss) ---
    gLevel.enemies.resize(gLevel.originalEnemyCount); // Remove os inimigos spawnados dinamicamente durante a partida
    gLevel.items.clear();                              // Remove itens (munição) que spawnaram durante a partida
    resetSpawnState();                                 // Zera os timers de spawn para não spawnar imediatamente ao reiniciar

    for (auto &en : gLevel.enemies)
    {
        // Devolve TODO MUNDO para a posição original que estava no mapa
        en.x = en.startX;
        en.z = en.startZ;
        en.hp = 100.0f;           // Reseta o HP (necessário para quem morreu antes do reset)
        en.state = STATE_IDLE;
        en.attackCooldown = 0.0f;
        en.hurtTimer = 0.0f;
        en.respawnTimer = 0.0f;   // Cancela qualquer contagem de respawn pendente

        // Se for um Boss (Tipos 0, 1 ou 2), muda a skin dele para a fase atual!
        if (en.type == 0 || en.type == 1 || en.type == 2)
        {
            if (faseAtual == 1)
                en.type = 2; // Fase 1: Mosca
            else if (faseAtual == 2)
                en.type = 1; // Fase 2: Grilo
            else if (faseAtual >= 3)
                en.type = 0; // Fase 3: Joaninha
        }
    }

    applyPhaseTextures();
}

void applyPhaseTextures()
{
    reloadPhaseTextures(gAssets, faseAtual);
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texChaoInterno   = gAssets.texChaoInterno;
    g.r.texTeto          = gAssets.texTeto;
    g.r.texLightOn       = gAssets.texLightOn;
    g.r.texSkydome       = gAssets.texSkydome;
    g.r.texPilar         = gAssets.texPilar;
    g.r.texAltarChao     = gAssets.texAltarChao;
}

void gameUpdate(float dt)
{
    g.time += dt;

    if (g.state != GameState::JOGANDO)
        return;

    atualizaMovimento();

    // Atualiza partículas de fogo do altar (fase 3)
    if (faseAtual == 3)
        atualizaParticulasFogo(dt);

    AudioListener L;
    L.pos = {camX, camY, camZ};
    float ry = yaw * 3.14159f / 180.0f;
    float rp = pitch * 3.14159f / 180.0f;
    L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    L.up = {0.0f, 1.0f, 0.0f};
    L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    if (g.player.damageAlpha > 0.0f)
    {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f)
            g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f)
    {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f)
            g.player.healthAlpha = 0.0f;
    }

    if (g.player.showAmmoDropWarning)
    {
        g.player.ammoDropWarningTimer -= dt;
        if (g.player.ammoDropWarningTimer <= 0.0f)
            g.player.showAmmoDropWarning = false;
    }

    updateEntities(dt);

    // --- Contagem de inimigos vivos ---
    g_aliveEnemyCount = 0;
    for (auto& en : gLevel.enemies)
        if ((en.type == 0 || en.type == 1 || en.type == 2) && en.state != STATE_DEAD)
            g_aliveEnemyCount++;

    if (g_enemySpawnNotifTimer > 0.0f)
        g_enemySpawnNotifTimer -= dt;

    // --- Proximidade ao incinerador (bloco '9') ---
    {
        float tile = gLevel.metrics.tile;
        float offX = gLevel.metrics.offsetX;
        float offZ = gLevel.metrics.offsetZ;
        int pX = (int)((camX - offX) / tile);
        int pZ = (int)((camZ - offZ) / tile);
        g_pertoDoIncinerador = false;
        for (int bz = pZ - 1; bz <= pZ + 1 && !g_pertoDoIncinerador; bz++)
            for (int bx = pX - 1; bx <= pX + 1 && !g_pertoDoIncinerador; bx++)
                if (bz >= 0 && bz < gLevel.map.getHeight() && bx >= 0 && bx < (int)gLevel.map.data()[bz].size())
                    if (gLevel.map.data()[bz][bx] == '9')
                        g_pertoDoIncinerador = true;
    }

    // --- Ativação do portal / purificação do núcleo ---
    if (componentesQueimados >= gLevel.totalNotebooks && gLevel.totalNotebooks > 0 && !doorActive)
    {
        doorActive = true;
        if (faseAtual >= 3)
            printf("\n>>> TODOS OS NOTEBOOKS PURIFICADOS! Elimine os inimigos e purifique o nucleo!\n");
        else
            printf("\n>>> TODOS OS COMPONENTES QUEIMADOS! Um portal apareceu na lava. Vá até ele e pressione E!\n");
    }

    if (g.player.health <= 0)
    {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    }

    updateWeaponAnim(dt);
}

void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    // Skydome (céu aberto) apenas na fase 3
    if (faseAtual == 3)
        drawSkydome(camX, camY, camZ, g.r);

    setSunDirectionEachFrame();
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);

    // CORRIGIDO: Agora chamamos com "gLevel.items"
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ, g.r, g.time);

    // Altar 3D (4 pilares + losangos + pirâmide + esfera) apenas na fase 3
    if (faseAtual == 3 && gLevel.hasLavaCenter)
    {
        drawAltar(gLevel.lavaCenterX, gLevel.lavaCenterZ, g.r, g.time);
    }
}

void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    HudState hs;
    hs.playerHealth = (int)g.player.health;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.componentesCarregados = componentesCarregados;
    // ADICIONE ESTAS DUAS LINHAS PARA A ARMA FUNCIONAR VISUALMENTE:
    hs.currentAmmo = g.player.currentAmmo;
    hs.reserveAmmo = g.player.reserveAmmo;
    hs.weaponState = g.weapon.state;
    hs.showAmmoDropWarning = g.player.showAmmoDropWarning;

    if (g.state == GameState::MENU_INICIAL)
    {
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Sobreviver", g.r);
    }
    else if (g.state == GameState::GAME_OVER)
    {
        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "SISTEMA CORROMPIDO", "Pressione ENTER para Reiniciar", g.r);
    }
    else if (g.state == GameState::PAUSADO)
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, false, false, true, componentesQueimados, gLevel.totalNotebooks);
        pauseMenuRender(janelaW, janelaH, g.time);
    }
    else if (g.state == GameState::FASE_CONCLUIDA)
    {
        drawWorld3D();
        GLuint fundoOriginal = g.r.texMenuBG;
        g.r.texMenuBG = g.r.texTelaWin;
        char subtitulo[64];
        sprintf(subtitulo, "Pressione ENTER para ir para a Fase %d", faseAtual + 1);
        menuRender(janelaW, janelaH, g.time, "", subtitulo, g.r);
        g.r.texMenuBG = fundoOriginal;
    }
    else if (g.state == GameState::JOGO_ZERADO)
    {
        GLuint fundoOriginal = g.r.texMenuBG;
        g.r.texMenuBG = g.r.texTelaFinal;
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER", g.r);
        g.r.texMenuBG = fundoOriginal;
    }
    else
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true, componentesQueimados, gLevel.totalNotebooks);
        drawEnemyCount(janelaW, janelaH, g_aliveEnemyCount, g_enemySpawnNotifTimer);

        // Prompts de interação perto do incinerador
        if (g_pertoDoIncinerador)
        {
            if (doorActive && g_aliveEnemyCount == 0)
            {
                if (faseAtual >= 3)
                    drawInteractionPrompt(janelaW, janelaH, "Pressione [E] para Purificar o Nucleo", 0.2f, 1.0f, 0.6f);
                else
                    drawInteractionPrompt(janelaW, janelaH, "Pressione [E] para Entrar no Portal", 0.2f, 1.0f, 0.6f);
            }
            else if (doorActive && g_aliveEnemyCount > 0)
            {
                char aviso[96];
                sprintf(aviso, "Elimine todos os inimigos! (%d restantes)", g_aliveEnemyCount);
                drawInteractionPrompt(janelaW, janelaH, aviso, 1.0f, 0.3f, 0.3f);
            }
            else if (componentesCarregados > 0)
            {
                drawInteractionPrompt(janelaW, janelaH, "Pressione [E] para Purificar Notebook", 0.9f, 0.9f, 0.2f);
            }
        }

        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }
    glutSwapBuffers();
}