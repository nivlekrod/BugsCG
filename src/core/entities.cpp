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

    // Apenas passamos pelas entidades (Bosses e HDs)
    for (auto& en : lvl.enemies)
    {
        if (en.state == STATE_DEAD) continue;

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