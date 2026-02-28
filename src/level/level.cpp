#include "level/level.h"
#include "core/config.h" 
#include <cstdio>

extern int faseAtual; // Puxa a informação da fase atual definida no game.cpp

// Configurações básicas para spawn
static const float ENEMY_START_HP = 100.0f;

bool loadLevel(Level &lvl, const char *mapPath, float tileSize)
{
    // 1. Carrega o mapa de texto (paredes, chão)
    if (!lvl.map.load(mapPath))
    {
        std::printf("ERRO: nao foi possivel carregar o mapa: %s\n", mapPath);
        return false;
    }

    lvl.metrics = LevelMetrics::fromMap(lvl.map, tileSize);
    
    // Limpa entidades antigas se houver
    lvl.enemies.clear();

    // 2. Escaneia o mapa procurando Bosses e o HD
    const auto& data = lvl.map.data();
    int H = lvl.map.getHeight();

    for(int z = 0; z < H; z++)
    {
        const std::string& row = data[z];
        for(int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            float wx, wz;
            lvl.metrics.tileCenter(x, z, wx, wz);

            // --- LÓGICA DE SPAWN (Ajustada para progressão de fases) ---
            int enemyType = -1; 

            // Se encontrar qualquer marcador de Boss no .txt
            if (c == 'J' || c == 'T' || c == 'M') 
            {
                // Força o boss específico da fase atual
                if (faseAtual == 1)      enemyType = 2; // Fase 1: Marco Leal (M)
                else if (faseAtual == 2) enemyType = 1; // Fase 2: Thiago (T)
                else                     enemyType = 0; // Fase 3+: Júlio (J)
            }
            else if (c == 'G') enemyType = 3; // Monstro extra
            else if (c == 'H') enemyType = 4; // Item Coletável: HD

            if (enemyType != -1) 
            {
                Enemy e;
                e.type = enemyType; 

                e.x = wx;
                e.z = wz;

                // Salva a posição inicial e zera timers
                e.startX = wx; 
                e.startZ = wz;
                e.respawnTimer = 0.0f;

                e.hp = ENEMY_START_HP; 
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f; 

                lvl.enemies.push_back(e);
            }
        }
    }

    return true;
}

void applySpawn(const Level &lvl, float &camX, float &camZ)
{
    lvl.metrics.spawnPos(lvl.map, camX, camZ);
}