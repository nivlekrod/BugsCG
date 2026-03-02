#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/game_state.h"
#include "graphics/ShaderObj.h"
#include "input/input.h"
#include "graphics/drawlevel.h"
#include "graphics/altar.h"
#include "level/levelmetrics.h"
#include "level/level.h"
#include "utils/utils.h"
#include <cstdio>

extern int faseAtual;
extern bool doorActive;

// =====================
// CONFIG / CONSTANTES
// =====================

// Config do grid
static const float TILE = 4.0f;      // tamanho do tile no mundo (ajuste)
static const float CEILING_H = 4.0f; // altura do teto
static const float WALL_H = 4.0f;    // altura da parede
static const float EPS_Y = 0.001f;   // evita z-fighting
static ShaderObj* shaderLanterna = nullptr;

// Dados de iluminação compartilhados entre drawLevel e drawEntities
static float sLavaCX = 0, sLavaCZ = 0;
static int sLavaCount = 0;
static const int MAX_LAMPS = 32;
static float sLampPX[MAX_LAMPS], sLampPZ[MAX_LAMPS];
static int sLampCount = 0;
static float sTime = 0;
static float sLavaCenterX = 0, sLavaCenterZ = 0;
static bool sHasLavaCenter = false;
static const MapLoader* sMapPtr = nullptr;
static LevelMetrics sMetrics;

// Cached uniform locations
static GLint locLampData[4] = {-1, -1, -1, -1};
static GLint locAltarLights[4] = {-1, -1, -1, -1};
static GLint locLavaCenter = -1;
static GLint locLavaFlicker = -1;
static GLint locIsSprite = -1;
static GLint locEntityWorldXZ = -1;
static bool sUniformsInit = false;

static void initUniformLocs()
{
    if (sUniformsInit || !shaderLanterna) return;
    char name[32];
    for (int i = 0; i < 4; i++) {
        snprintf(name, sizeof(name), "uLampData[%d]", i);
        locLampData[i] = glGetUniformLocation(shaderLanterna->ID, name);
        snprintf(name, sizeof(name), "uAltarLights[%d]", i);
        locAltarLights[i] = glGetUniformLocation(shaderLanterna->ID, name);
    }
    locLavaCenter = glGetUniformLocation(shaderLanterna->ID, "uLavaCenter");
    locLavaFlicker = glGetUniformLocation(shaderLanterna->ID, "uLavaFlicker");
    locIsSprite = glGetUniformLocation(shaderLanterna->ID, "uIsSprite");
    locEntityWorldXZ = glGetUniformLocation(shaderLanterna->ID, "uEntityWorldXZ");
    sUniformsInit = true;
}

// Verifica linha de visão entre dois pontos no grid (sem paredes bloqueando)
static bool hasLineOfSight(float x1, float z1, float x2, float z2)
{
    if (!sMapPtr) return true;
    const auto& data = sMapPtr->data();
    int H = sMapPtr->getHeight();
    float tile = sMetrics.tile;
    float offX = sMetrics.offsetX;
    float offZ = sMetrics.offsetZ;

    int tx1 = (int)((x1 - offX) / tile);
    int tz1 = (int)((z1 - offZ) / tile);
    int tx2 = (int)((x2 - offX) / tile);
    int tz2 = (int)((z2 - offZ) / tile);
    int stx = tx1, stz = tz1;

    int dx = tx2 > tx1 ? tx2 - tx1 : tx1 - tx2;
    int dz = tz2 > tz1 ? tz2 - tz1 : tz1 - tz2;
    int sx = tx1 < tx2 ? 1 : -1;
    int sz = tz1 < tz2 ? 1 : -1;
    int err = dx - dz;

    while (true) {
        if (!(tx1 == stx && tz1 == stz)) {
            if (tz1 >= 0 && tz1 < H && tx1 >= 0 && tx1 < (int)data[tz1].size()) {
                char c = data[tz1][tx1];
                if (c == '1' || c == '2') return false;
            }
        }
        if (tx1 == tx2 && tz1 == tz2) break;
        int e2 = 2 * err;
        if (e2 > -dz) { err -= dz; tx1 += sx; }
        if (e2 < dx) { err += dx; tz1 += sz; }
    }
    return true;
}

static const GLfloat kAmbientOutdoor[] = {0.02f, 0.02f, 0.02f, 1.0f}; // Breu
static const GLfloat kAmbientIndoor[] = {0.01f, 0.01f, 0.01f, 1.0f};  // Breu

// ======================
// CONFIG ÚNICA DO CULLING (XZ)
// ======================
static float gCullHFovDeg = 170.0f;     // FOV horizontal do culling (cenário + entidades)
static float gCullNearTiles = 2.0f;     // dentro disso não faz culling angular
static float gCullMaxDistTiles = 20.0f; // 0 = sem limite; em tiles

// Retorna TRUE se deve renderizar o objeto no plano XZ (distância + cone de FOV)
// - Usa as configs globais gCull*
// - Usa forward já normalizado (fwdx,fwdz) e flag hasFwd
static inline bool isVisibleXZ(float objX, float objZ,
                               float camX, float camZ,
                               bool hasFwd, float fwdx, float fwdz)
{
    float vx = objX - camX;
    float vz = objZ - camZ;
    float distSq = vx * vx + vz * vz;

    // 0) Distância máxima (se habilitada)
    if (gCullMaxDistTiles > 0.0f)
    {
        float maxDist = gCullMaxDistTiles * TILE;
        float maxDistSq = maxDist * maxDist;
        if (distSq > maxDistSq)
            return false;
    }

    // 1) Dentro do near: não faz culling angular
    float nearDist = gCullNearTiles * TILE;
    float nearDistSq = nearDist * nearDist;
    if (distSq <= nearDistSq)
        return true;

    // 2) Sem forward válido: não faz culling angular
    if (!hasFwd)
        return true;

    // 3) Cone por FOV horizontal
    float cosHalf = std::cos(deg2rad(gCullHFovDeg * 0.5f));

    float invDist = 1.0f / std::sqrt(distSq);
    float nvx = vx * invDist;
    float nvz = vz * invDist;

    float dot = clampf(nvx * fwdx + nvz * fwdz, -1.0f, 1.0f);
    return dot >= cosHalf;
}

static void bindTexture0(GLuint tex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
}

static float hash01(float x)
{
    float s = sinf(x * 12.9898f) * 43758.5453f;
    return s - floorf(s);
}

static float flickerFluorescente(float t)
{
    const float rate = 4.0f;
    float block = floorf(t * rate);
    float r = hash01(block);

    if (r < 0.22f)
    {
        float phase = t * rate - block;

        if (phase > 0.35f && phase < 0.55f)
            return 0.12f;

        if (r < 0.06f && phase > 0.65f && phase < 0.78f)
            return 0.40f;
    }

    return 0.96f + 0.04f * sinf(t * 5.0f);
}

// Seta visibilidade das luminárias para uma posição (com LOS check)
static void setLampVisForPos(float px, float pz, float time, const float* lpx, const float* lpz, int lc)
{
    if (!shaderLanterna) return;
    shaderLanterna->use();
    for (int li = 0; li < 4; li++) {
        if (li < lc) {
            float fl = flickerFluorescente(time + lpx[li] * 3.7f + lpz[li] * 2.3f);
            bool vis = fl > 0.5f && hasLineOfSight(lpx[li], lpz[li], px, pz);
            glUniform3f(locLampData[li], lpx[li], lpz[li], vis ? fl : 0.0f);
        } else {
            glUniform3f(locLampData[li], 0.0f, 0.0f, 0.0f);
        }
    }
}

static void setIndoorLampAt(float x, float z, float intensity)
{
    GLfloat pos[] = {x, CEILING_H - 0.05f, z, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    GLfloat diff[] = {
        1.20f * intensity,
        1.22f * intensity,
        1.28f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff);

    GLfloat amb[] = {
        1.10f * intensity,
        1.10f * intensity,
        1.12f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
}

static void beginIndoor(float wx, float wz, float time)
{
    glDisable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientIndoor);

    glEnable(GL_LIGHT1);

    float f = flickerFluorescente(time);
    float intensity = 1.2f * f;

    setIndoorLampAt(wx, wz, intensity);
}

static void endIndoor()
{
    glDisable(GL_LIGHT1);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
    glEnable(GL_LIGHT0);
}

static void desenhaQuadTeto(float x, float z, float tile, float tilesUV)
{
    float half = tile * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, 0.0f);
    glVertex3f(x + half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, tilesUV);
    glVertex3f(x + half, CEILING_H, z + half);
    glTexCoord2f(0.0f, tilesUV);
    glVertex3f(x - half, CEILING_H, z + half);
    glEnd();
}

static void desenhaQuadChao(float x, float z, float tile, float tilesUV)
{
    float half = tile * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, EPS_Y, z + half);
    glTexCoord2f(tilesUV, 0.0f);
    glVertex3f(x + half, EPS_Y, z + half);
    glTexCoord2f(tilesUV, tilesUV);
    glVertex3f(x + half, EPS_Y, z - half);
    glTexCoord2f(0.0f, tilesUV);
    glVertex3f(x - half, EPS_Y, z - half);
    glEnd();
}

static void desenhaTileChao(float x, float z, GLuint texChaoX, bool temTeto)
{
    if (shaderLanterna) {
        shaderLanterna->use();
        shaderLanterna->setInt("uTexture", 0);
        shaderLanterna->setInt("uFlashlightOn", flashlightOn ? 1 : 0);
    } else {
        glUseProgram(0);
    }
    glColor3f(1, 1, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texChaoX);

    desenhaQuadChao(x, z, TILE, 2.0f);

    if (temTeto)
    {
        glBindTexture(GL_TEXTURE_2D, texChaoX);
        desenhaQuadTeto(x, z, TILE, 2.0f);
    }
}

// --- Desenha parede FACE POR FACE ---
static void desenhaParedePorFace(float x, float z, GLuint texParedeX, int f)
{
    float half = TILE * 0.5f;

    // --- CORREÇÃO: Liga a lanterna na parede em vez de glUseProgram(0) ---
    if (shaderLanterna) {
        shaderLanterna->use();
        shaderLanterna->setInt("uTexture", 0);
        shaderLanterna->setInt("uFlashlightOn", flashlightOn ? 1 : 0);
    } else {
        glUseProgram(0);
    }

    glColor3f(1, 1, 1);
    
    glActiveTexture(GL_TEXTURE0); // Garante que a textura certa vai pro shader
    glBindTexture(GL_TEXTURE_2D, texParedeX);

    float tilesX = 1.0f;
    float tilesY = 2.0f;

    glBegin(GL_QUADS);

    switch (f)
    {
    case 0: // z+ (Frente)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        break;

    case 1: // z- (Trás)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        break;

    case 2: // x+ (Direita)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        break;

    case 3: // x- (Esquerda)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        break;
    }
    glEnd();
}

// Wrapper para desenhar o cubo todo (parede outdoor)
static void desenhaParedeCuboCompleto(float x, float z, GLuint texParedeX)
{
    desenhaParedePorFace(x, z, texParedeX, 0);
    desenhaParedePorFace(x, z, texParedeX, 1);
    desenhaParedePorFace(x, z, texParedeX, 2);
    desenhaParedePorFace(x, z, texParedeX, 3);

    float half = TILE * 0.5f;
    glBindTexture(GL_TEXTURE_2D, texParedeX);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, WALL_H, z + half);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + half, WALL_H, z + half);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + half, WALL_H, z - half);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - half, WALL_H, z - half);
    glEnd();
}

static void desenhaTileLava(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progLava);

    GLint locTime = glGetUniformLocation(r.progLava, "uTime");
    GLint locStr = glGetUniformLocation(r.progLava, "uStrength");
    GLint locScr = glGetUniformLocation(r.progLava, "uScroll");
    GLint locHeat = glGetUniformLocation(r.progLava, "uHeat");
    GLint locTex = glGetUniformLocation(r.progLava, "uTexture");
    GLint locCenter = glGetUniformLocation(r.progLava, "uLavaCenterPos");
    GLint locRadius = glGetUniformLocation(r.progLava, "uLavaRadius");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locScr, 0.1f, 0.0f);
    glUniform1f(locHeat, 0.6f);
    glUniform2f(locCenter, sLavaCX, sLavaCZ);
    glUniform1f(locRadius, TILE * 1.4f);

    bindTexture0(r.texLava);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    desenhaQuadChao(x, z, TILE, 2.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glUseProgram(0);
}

static void desenhaTileSangue(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progSangue);

    GLint locTime = glGetUniformLocation(r.progSangue, "uTime");
    GLint locStr = glGetUniformLocation(r.progSangue, "uStrength");
    GLint locSpd = glGetUniformLocation(r.progSangue, "uSpeed");
    GLint locTex = glGetUniformLocation(r.progSangue, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locSpd, 2.0f, 1.3f);

    bindTexture0(r.texSangue);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
}

// --- Checa vizinhos ---
static char getTileAt(const MapLoader &map, int tx, int tz)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    if (tz < 0 || tz >= H)
        return '0';
    if (tx < 0 || tx >= (int)data[tz].size())
        return '0';

    return data[tz][tx];
}

static void drawFace(float wx, float wz, int face, char neighbor, GLuint texParedeInternaX, float time)
{
    bool outside = (neighbor == '0' || neighbor == 'L' || neighbor == 'B');

    if (outside)
    {
        glDisable(GL_LIGHT1);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
        glEnable(GL_LIGHT0);

        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
    }
    else if (neighbor != '2')
    {
        beginIndoor(wx, wz, time);
        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
        endIndoor();
    }
}

void drawLevel(const MapLoader &map, float px, float pz, float dx, float dz, const RenderAssets &r, float time)
{
    // CARREGA A LANTERNA NA PRIMEIRA VEZ
    if (!shaderLanterna) {
        shaderLanterna = new ShaderObj("shaders/flashlight.vert", "shaders/flashlight.frag");
    }
    const auto &data = map.data();
    const int H = map.getHeight();

    LevelMetrics m = LevelMetrics::fromMap(map, TILE);
    sMapPtr = &map;
    sMetrics = m;

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    // --- Lava glow: computar centro da lava para iluminação via shader ---
    float lavaCX = 0, lavaCZ = 0;
    int lavaCount = 0;
    // --- Lamp positions: coletar para iluminação omnidirecional ---
    float lampPX[MAX_LAMPS], lampPZ[MAX_LAMPS];
    int lampCount = 0;
    for (int zz = 0; zz < H; zz++) {
        for (int xx = 0; xx < (int)data[zz].size(); xx++) {
            char cc = data[zz][xx];
            if (cc == 'L' || cc == '9') {
                float lwx, lwz;
                m.tileCenter(xx, zz, lwx, lwz);
                lavaCX += lwx;
                lavaCZ += lwz;
                lavaCount++;
            }
            if (cc == 'F' && lampCount < MAX_LAMPS) {
                m.tileCenter(xx, zz, lampPX[lampCount], lampPZ[lampCount]);
                lampCount++;
            }
        }
    }
    if (lavaCount > 0) {
        lavaCX /= lavaCount;
        lavaCZ /= lavaCount;
    }
    // Armazena para uso em drawEntities
    sLavaCX = lavaCX; sLavaCZ = lavaCZ; sLavaCount = lavaCount;
    sLavaCenterX = lavaCX; sLavaCenterZ = lavaCZ; sHasLavaCenter = (lavaCount > 0);
    for (int i = 0; i < lampCount; i++) { sLampPX[i] = lampPX[i]; sLampPZ[i] = lampPZ[i]; }
    sLampCount = lampCount;
    sTime = time;

    // Uniforms de lava e lâmpadas (set once per frame)
    initUniformLocs();
    if (shaderLanterna) {
        shaderLanterna->use();
        glUniform1i(locIsSprite, 0); // tiles usam WorldPos
        if (lavaCount > 0) {
            glUniform2f(locLavaCenter, lavaCX, lavaCZ);
            float lFlk = 0.9f + 0.1f * sinf(time * 3.0f);
            glUniform1f(locLavaFlicker, lFlk);
        } else {
            glUniform1f(locLavaFlicker, 0.0f);
        }

        // Iluminação dos losangos do altar (apenas fase 3)
        if (faseAtual == 3 && lavaCount > 0) {
            float pillarX[4], pillarZ[4];
            getAltarPillarPositions(lavaCX, lavaCZ, pillarX, pillarZ);
            float pulse = 1.6f + 0.3f * sinf(time * 3.5f);
            for (int i = 0; i < 4; i++)
                glUniform3f(locAltarLights[i], pillarX[i], pillarZ[i], pulse);
        } else {
            for (int i = 0; i < 4; i++)
                glUniform3f(locAltarLights[i], 0.0f, 0.0f, 0.0f);
        }
    }

    // --- Detecta se o mapa é "ao ar livre" (sem tiles '3' ou 'F', ignorando o spawn do P) ---
    int spawnTX = (int)map.getPlayerStartX();
    int spawnTZ = (int)map.getPlayerStartZ();
    bool outdoorMap = true;
    for (int zz = 0; zz < H && outdoorMap; zz++)
        for (int xx = 0; xx < (int)data[zz].size() && outdoorMap; xx++)
            if ((xx != spawnTX || zz != spawnTZ) && (data[zz][xx] == '3' || data[zz][xx] == 'F'))
                outdoorMap = false;

    for (int z = 0; z < H; z++)
    {
        for (int x = 0; x < (int)data[z].size(); x++)
        {
            float wx, wz;
            m.tileCenter(x, z, wx, wz);

            // CULLING ÚNICO (cenário)
            if (!isVisibleXZ(wx, wz, px, pz, hasFwd, fwdx, fwdz))
                continue;

            char c = data[z][x];

            // Per-tile: set lamp visibility (LOS check)
            setLampVisForPos(wx, wz, time, lampPX, lampPZ, lampCount);

            bool isEntity = (c == 'J' || c == 'T' || c == 'M' || c == 'K' ||
                             c == 'G' || c == 'N' || c == 'A' || c == 'E' ||
                             c == 'I');

            if (isEntity)
            {
                char viz1 = getTileAt(map, x + 1, z);
                char viz2 = getTileAt(map, x - 1, z);
                char viz3 = getTileAt(map, x, z + 1);
                char viz4 = getTileAt(map, x, z - 1);

                bool isIndoor = !outdoorMap &&
                                (viz1 == '3' || viz1 == '2' || viz1 == 'F' ||
                                 viz2 == '3' || viz2 == '2' || viz2 == 'F' ||
                                 viz3 == '3' || viz3 == '2' || viz3 == 'F' ||
                                 viz4 == '3' || viz4 == '2' || viz4 == 'F');

                if (isIndoor)
                {
                    beginIndoor(wx, wz, time);
                    desenhaTileChao(wx, wz, r.texChaoInterno, true);
                    endIndoor();
                }
                else
                {
                    desenhaTileChao(wx, wz, r.texChao, false);
                }
            }
            else if (c == '0')
            {
                desenhaTileChao(wx, wz, r.texChao, false);
            }
            else if (c == '3')
            {
                if (!outdoorMap) {
                    beginIndoor(wx, wz, time);
                    desenhaTileChao(wx, wz, r.texChaoInterno, true);
                    endIndoor();
                } else {
                    desenhaTileChao(wx, wz, r.texChao, false);
                }
            }
            else if (c == '1')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texParede);
            }
            else if (c == '2')
            {
                int nbs[4][2] = {{x, z+1}, {x, z-1}, {x+1, z}, {x-1, z}};
                char vizChars[4];
                vizChars[0] = getTileAt(map, x, z + 1);
                vizChars[1] = getTileAt(map, x, z - 1);
                vizChars[2] = getTileAt(map, x + 1, z);
                vizChars[3] = getTileAt(map, x - 1, z);

                for (int fi = 0; fi < 4; fi++) {
                    if (vizChars[fi] == '2') continue;
                    float nwx, nwz;
                    m.tileCenter(nbs[fi][0], nbs[fi][1], nwx, nwz);
                    setLampVisForPos(nwx, nwz, time, lampPX, lampPZ, lampCount);
                    drawFace(wx, wz, fi, vizChars[fi], r.texParedeInterna, time);
                }
            }
            else if (c == 'F')
            {
                // Luminária piscando no teto (tile, não entidade)
                float f = flickerFluorescente(time + wx * 3.7f + wz * 2.3f);

                desenhaTileChao(wx, wz, r.texChaoInterno, true);

                // Sprite da luminária no teto (081_on quando acesa, 081 quando apagada)
                if (shaderLanterna) {
                    shaderLanterna->use();
                    shaderLanterna->setInt("uTexture", 0);
                    shaderLanterna->setInt("uFlashlightOn", flashlightOn ? 1 : 0);
                } else {
                    glUseProgram(0);
                }

                glActiveTexture(GL_TEXTURE0);
                GLuint lightTex = (f > 0.5f) ? r.texLightOn : r.texTeto;
                glBindTexture(GL_TEXTURE_2D, lightTex);
                glColor3f(1, 1, 1);

                float half = 1.0f;
                glBegin(GL_QUADS);
                glNormal3f(0.0f, -1.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(wx - half, CEILING_H - 0.01f, wz - half);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(wx + half, CEILING_H - 0.01f, wz - half);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(wx + half, CEILING_H - 0.01f, wz + half);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(wx - half, CEILING_H - 0.01f, wz + half);
                glEnd();

                glColor3f(1, 1, 1);
            }
            else if (c == 'L')
            {
                if (!outdoorMap) {
                    beginIndoor(wx, wz, time);
                    desenhaTileChao(wx, wz, r.texChaoInterno, true);
                    endIndoor();
                } else {
                    desenhaTileChao(wx, wz, r.texChao, false);
                }
                desenhaTileLava(wx, wz, r, time);
            }
            else if (c == 'B')
            {
                desenhaTileSangue(wx, wz, r, time);
            }
            else if (c == '9')
            {
                if (!outdoorMap) {
                    beginIndoor(wx, wz, time);
                    desenhaTileChao(wx, wz, r.texChaoInterno, true);
                    endIndoor();
                } else {
                    desenhaTileChao(wx, wz, r.texChao, false);
                }
                desenhaTileLava(wx, wz, r, time);
            }
        }
    }
}

static void drawSprite(float x, float z, float w, float h, GLuint tex, float camX, float camZ)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    float ddx = camX - x;
    float ddz = camZ - z;
    float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;

    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    float hw = w * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(hw, h, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-hw, h, 0.0f);
    glEnd();

    glPopMatrix();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

// Calcula e aplica uniforms de iluminação (luminárias) para uma posição de sprite
static void setLightingUniformsAt(float wx, float wz)
{
    if (!shaderLanterna) return;
    shaderLanterna->use();
    glUniform1i(locIsSprite, 1);
    glUniform2f(locEntityWorldXZ, wx, wz);
    setLampVisForPos(wx, wz, sTime, sLampPX, sLampPZ, sLampCount);
}

// Desenha inimigos e itens
void drawEntities(const std::vector<Enemy> &enemies, const std::vector<Item> &items, float camX, float camZ, float dx, float dz, const RenderAssets &r, float time)
{
    // AQUI ESTAVA O BUG: Removi o glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    // --- CORREÇÃO: Aplica a lanterna nos HDs e Monstros ---
    if (shaderLanterna) {
        shaderLanterna->use();
        shaderLanterna->setInt("uTexture", 0);
        shaderLanterna->setInt("uFlashlightOn", flashlightOn ? 1 : 0);
    }

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    // --- INIMIGOS E HDs ---
    for (const auto &en : enemies)
    {
        if (en.state == STATE_DEAD)
            continue;

        if (!isVisibleXZ(en.x, en.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        int t = (en.type < 0 || en.type > 4) ? 0 : en.type;

        // --- LUMINÁRIA PISCANDO (type 5) ---
        if (en.type == 5)
        {
            float flicker = flickerFluorescente(time + en.x * 3.7f + en.z * 2.3f);
            float half = 1.0f;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.1f);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r.texTeto);
            glColor3f(flicker, flicker, flicker);

            // Quad no teto voltado para baixo
            glBegin(GL_QUADS);
            glNormal3f(0.0f, -1.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(en.x - half, CEILING_H - 0.01f, en.z - half);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(en.x + half, CEILING_H - 0.01f, en.z - half);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(en.x + half, CEILING_H - 0.01f, en.z + half);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(en.x - half, CEILING_H - 0.01f, en.z + half);
            glEnd();

            glColor3f(1, 1, 1);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            continue;
        }

        GLuint currentTex;
        if (en.hurtTimer > 0.0f)
            currentTex = r.texEnemiesDamage[t];
        else if (en.state == STATE_CHASE || en.state == STATE_ATTACK)
            currentTex = r.texEnemiesRage[t];
        else
            currentTex = r.texEnemies[t];

        // --- CONTROLE DE TAMANHO ---
        float spriteW = 2.5f; 
        float spriteH = 2.5f; 

        if (en.type == 4) {   // Se for o notebook
            spriteH = 0.5f;   
            spriteW = 0.75f;  // 1536/1024 = 1.5 → 0.5 * 1.5
        }

        // --- Segurança para o shader ler a textura certa ---
        setLightingUniformsAt(en.x, en.z);
        glActiveTexture(GL_TEXTURE0); 
        drawSprite(en.x, en.z, spriteW, spriteH, currentTex, camX, camZ);
    }

    // --- ITENS (MUNIÇÃO) ---
    for (const auto &item : items)
    {
        if (!item.active)
            continue;

        if (!isVisibleXZ(item.x, item.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        if (item.type == ITEM_AMMO && r.texItemAmmo != 0)
        {
            setLightingUniformsAt(item.x, item.z);
            // Desenha um pequeno cubo 3D (caixa) em vez de um sprite
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r.texItemAmmo);
            glColor3f(1.0f, 1.0f, 1.0f);

            glPushMatrix();
            glTranslatef(item.x, 0.25f, item.z); // Eleva um pouco para não ficar enterrado no chão
            
            float size = 0.25f; // Metade do lado da caixa

            glBegin(GL_QUADS);
            // Frente
            glNormal3f(0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, size);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, size);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, size);
            // Tras
            glNormal3f(0.0f, 0.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, -size);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, size, -size);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(size, size, -size);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size, -size);
            // Topo
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, -size);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, size, size);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(size, size, size);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, -size);
            // Base
            glNormal3f(0.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, -size, -size);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(size, -size, -size);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size, size);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, size);
            // Direita
            glNormal3f(1.0f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, -size);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, -size);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(size, size, size);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size, size);
            // Esquerda
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, -size);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, size);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, size, size);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, -size);
            glEnd();

            glPopMatrix();
        }
    }

    // --- PORTAL DE TRANSIÇÃO (aparece na lava quando todos os notebooks foram queimados) ---
    if (doorActive && sHasLavaCenter && r.texPortal != 0 && r.progPortal != 0)
    {
        glUseProgram(r.progPortal);
        glUniform1i(glGetUniformLocation(r.progPortal, "uTexture"), 0);
        glUniform1f(glGetUniformLocation(r.progPortal, "uTime"), time);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r.texPortal);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 1);

        float portalW = 3.0f, portalH = 3.0f;
        float hw = portalW * 0.5f;

        glPushMatrix();
        glTranslatef(sLavaCenterX, 0.0f, sLavaCenterZ);

        float ddx = camX - sLavaCenterX;
        float ddz = camZ - sLavaCenterZ;
        float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(-hw, 0, 0);
        glTexCoord2f(1, 1); glVertex3f( hw, 0, 0);
        glTexCoord2f(1, 0); glVertex3f( hw, portalH, 0);
        glTexCoord2f(0, 0); glVertex3f(-hw, portalH, 0);
        glEnd();

        glPopMatrix();
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glUseProgram(0);
    }

    // Desliga o shader e restaura estados no final
    if (shaderLanterna) {
        shaderLanterna->use();
        glUniform1i(locIsSprite, 0);
    }
    glUseProgram(0);
    glDisable(GL_ALPHA_TEST);
}