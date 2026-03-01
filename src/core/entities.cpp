#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include <cmath>
#include <cstdlib> 
#include <cstdio>  

// --- VARIÁVEIS EXTERNAS DO DEVOUR ---
extern int componentesCarregados;
extern int componentesQueimados;
extern int faseAtual;

// MANTIDO: Função de colisão original do seu motor
bool isWalkable(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];
    if (c == '1' || c == '2') return false; 

    return true;
}

void updateEntities(float dt)
{
    auto& g = gameContext();
    auto& lvl = gameLevel();
    auto& audio = gameAudio();

    // --- LÓGICA DE SPAWN DE NOVOS INIMIGOS COM O TEMPO ---
    static float spawnNewEnemyTimer = 45.0f;
    spawnNewEnemyTimer -= dt;
    if (spawnNewEnemyTimer <= 0.0f)
    {
        // O tempo diminui conforme mais HDs são queimados (mínimo de 10s)
        spawnNewEnemyTimer = 45.0f - (componentesQueimados * 3.0f);
        if (spawnNewEnemyTimer < 10.0f) spawnNewEnemyTimer = 10.0f;

        float spawnX = 0;
        float spawnZ = 0;
        bool found = false;

        // Tenta achar um local válido no mapa que não esteja muito perto do jogador
        for (int tries = 0; tries < 50; tries++)
        {
            if (lvl.map.data().empty()) break;
            
            int rx = std::rand() % lvl.map.data()[0].size();
            int rz = std::rand() % lvl.map.data().size();

            float wx, wz;
            lvl.metrics.tileCenter(rx, rz, wx, wz);
            if (isWalkable(wx, wz))
            {
                float dx = wx - camX;
                float dz = wz - camZ;
                if (std::sqrt(dx * dx + dz * dz) > 10.0f) // Mais de 10 blocos de distância
                {
                    spawnX = wx;
                    spawnZ = wz;
                    found = true;
                    break;
                }
            }
        }

        static int enemiesSpawnedThisPhase = 0;
        if (found)
        {
            Enemy newEn;
            if (faseAtual == 1) newEn.type = 2;
            else if (faseAtual == 2) newEn.type = 1;
            else newEn.type = 0;

            newEn.x = spawnX;
            newEn.z = spawnZ;
            newEn.startX = spawnX;
            newEn.startZ = spawnZ;
            newEn.hp = 100.0f;
            newEn.state = STATE_CHASE;
            newEn.attackCooldown = 0.0f;
            newEn.hurtTimer = 0.0f;
            newEn.respawnTimer = 0.0f;
            
            lvl.enemies.push_back(newEn);
            printf("\n>>> NOVO INIMIGO SPAWNOU! A tensao aumenta...\n");

            enemiesSpawnedThisPhase++;
            if (enemiesSpawnedThisPhase >= 2)
            {
                enemiesSpawnedThisPhase = 0;
                
                // Procurar local aleatório para a caixa de munição
                float ammoSpawnX = spawnX;
                float ammoSpawnZ = spawnZ;
                
                for (int tries = 0; tries < 50; tries++)
                {
                    if (lvl.map.data().empty()) break;
                    
                    int rx = std::rand() % lvl.map.data()[0].size();
                    int rz = std::rand() % lvl.map.data().size();

                    float wx, wz;
                    lvl.metrics.tileCenter(rx, rz, wx, wz);
                    if (isWalkable(wx, wz))
                    {
                        ammoSpawnX = wx;
                        ammoSpawnZ = wz;
                        break;
                    }
                }

                Item ammoDrop;
                ammoDrop.type = ITEM_AMMO;
                ammoDrop.x = ammoSpawnX;
                ammoDrop.z = ammoSpawnZ;
                ammoDrop.active = true;
                ammoDrop.respawnTimer = 0.0f;
                lvl.items.push_back(ammoDrop);

                g.player.showAmmoDropWarning = true;
                g.player.ammoDropWarningTimer = 3.0f; // Exibe o aviso por 3 segundos

                printf("\n>>> MUNICÃO SURGIU EM UM LOCAL ALEATÓRIO DO MAPA!\n");
            }
        }
    }

    // --- LÓGICA DE COLETA DE ITENS ---
    for (auto& item : lvl.items)
    {
        if (!item.active) continue;
        
        float dx = camX - item.x;
        float dz = camZ - item.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        
        if (dist < 1.0f) 
        {
            if (item.type == ITEM_AMMO)
            {
                g.player.reserveAmmo += 12; // Dá 12 tiros reservas
                item.active = false;
                printf("\n>>> MUNICÃO COLETADA! (+12 tiros)\n");
            }
        }
    }

    // Apenas passamos pelas entidades (Bosses e HDs)
    for (auto& en : lvl.enemies)
    {
        // --- LÓGICA DE RESPAWN PARA INIMIGOS MORTOS ---
        if (en.state == STATE_DEAD) 
        {
            // Apenas inimigos (0, 1, 2) dão respawn, HDs (4) não!
            if (en.type == 0 || en.type == 1 || en.type == 2)
            {
                if (en.respawnTimer > 0.0f)
                {
                    en.respawnTimer -= dt;
                    if (en.respawnTimer <= 0.0f)
                    {
                        en.state = STATE_CHASE;
                        en.hp = 100.0f; // Recupera vida
                        en.x = en.startX;
                        en.z = en.startZ;
                        printf("\n>>> UM INIMIGO RENASCEU!\n");
                    }
                }
            }
            continue;
        }

        if (en.hurtTimer > 0.0f) en.hurtTimer -= dt;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        
        // =============================================================
        // 1. BOSSES (Tipo 0 = Júlio, Tipo 1 = Thiago, Tipo 2 = Marco)
        // =============================================================
        if (en.type == 0 || en.type == 1 || en.type == 2) 
        {
            // --- CÁLCULO DE DIFICULDADE ---
            // A velocidade aumenta conforme os HDs são queimados
            float baseSpeed = ENEMY_SPEED * 0.7f;
            float speedBoost = 1.0f + (componentesQueimados * 0.20f); 
            float moveStep = baseSpeed * speedBoost * dt;

            // Limite de velocidade (Cap) para não quebrar o jogo
            float maxSpeedLimit = ENEMY_SPEED * 3.0f;
            if (moveStep > maxSpeedLimit * dt) moveStep = maxSpeedLimit * dt;

            if (dist > 0.8f) 
            {
                en.state = STATE_CHASE;
                float dirX = dx / dist;
                float dirZ = dz / dist;

                // Move em X
                float nextX = en.x + dirX * moveStep;
                if (isWalkable(nextX, en.z)) en.x = nextX;

                // Move em Z
                float nextZ = en.z + dirZ * moveStep;
                if (isWalkable(en.x, nextZ)) en.z = nextZ;
            }
            else 
            {
                // Ataque dos Bosses
                en.state = STATE_ATTACK;
                en.attackCooldown -= dt;
                
                if (en.attackCooldown <= 0.0f)
                {
                    g.player.health -= 35; // Dano massivo!
                    en.attackCooldown = 0.8f; 
                    g.player.damageAlpha = 1.0f; 
                    audioPlayHurt(audio);
                }
            }
        }
        // =============================================================
        // 2. COLETÁVEL (Tipo 4 = HD)
        // =============================================================
        else if (en.type == 4)
        {
            if (dist < 1.2f && componentesCarregados == 0)
            {
                en.state = STATE_DEAD; // O HD some do mapa
                componentesCarregados = 1;
                printf("\n>>> HD RECOLHIDO! Corra para o Altar (Bloco 9)!\n");
            }
        }
    }
}