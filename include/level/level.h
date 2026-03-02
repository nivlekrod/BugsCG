#pragma once
#include "level/maploader.h"
#include "level/levelmetrics.h"
#include "core/entities.h" 
#include <vector>          

struct Level
{
    MapLoader map;
    LevelMetrics metrics;
    std::vector<Enemy> enemies;
    int originalEnemyCount = 0; // Quantidade de inimigos carregados do mapa (exclui os spawnados dinamicamente)
    int totalHDs = 0; // Quantidade de HDs no mapa (necessários para vencer a fase)

    // ADICIONE ESTA LINHA:
    std::vector<Item> items;
};
bool loadLevel(Level &lvl, const char *mapPath, float tileSize);
void applySpawn(const Level &lvl, float &camX, float &camZ);
