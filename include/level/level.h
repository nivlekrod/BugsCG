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
    int totalNotebooks = 0; // Quantidade de notebooks no mapa (necessários para vencer a fase)

    // ADICIONE ESTA LINHA:
    std::vector<Item> items;

    // Centro da lava ('9') em coordenadas de mundo (para altar e colisão)
    float lavaCenterX = 0, lavaCenterZ = 0;
    bool hasLavaCenter = false;
};
bool loadLevel(Level &lvl, const char *mapPath, float tileSize);
void applySpawn(const Level &lvl, float &camX, float &camZ);
