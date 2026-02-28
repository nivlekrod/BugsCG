#include "level/maploader.h"
#include <fstream>
#include <iostream>

bool MapLoader::load(const char* filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERRO CRITICO: Nao foi possivel abrir o arquivo " << filename << std::endl;
        return false;
    }

    std::string line;
    mapData.clear();
    height = 0;
    width = 0;

    while (std::getline(file, line)) {

        // remove \r se o arquivo vier do Windows
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // ignora linhas vazias ou comentários
        if (line.empty() || line[0] == '#')
            continue;

        // detecta spawn '9'
   // detecta spawn e objetos especiais
        for (int x = 0; x < (int)line.size(); x++) {
            // Se achar 'P', define como spawn do jogador e vira chão '3'
            if (line[x] == 'P' || line[x] == 'S') {
                playerStartX = (float)x;
                playerStartZ = (float)height;
                line[x] = '3'; // Vira chão de interior para você poder andar
            }
            // Se achar '9', NÃO APAGA. Deixa ele lá para o input.cpp encontrar
            else if (line[x] == '9') {
                // Apenas deixa o '9' no mapa, não faz nada com o spawn
            }
        }

        mapData.push_back(line);
        if ((int)line.size() > width) width = (int)line.size();
        height++;
    }

    file.close();
    return (height > 0 && width > 0);
}
