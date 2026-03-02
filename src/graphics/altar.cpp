// Altar do Mapa 3 - Adaptado de draw-deprecated.cpp
// 4 pilares com losangos girantes incandescentes + pirâmide de 3 degraus + esfera flutuante

#include "graphics/altar.h"
#include "core/game_state.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const int NUM_PILARES = 4;
static const float RAIO = 14.0f;

// ============================================
// HELPER: posições dos pilares no mundo
// ============================================
void getAltarPillarPositions(float centerX, float centerZ, float outX[4], float outZ[4])
{
    float ang0 = -(float)M_PI / 2.0f;
    float passo = 2.0f * (float)M_PI / NUM_PILARES;
    for (int i = 0; i < NUM_PILARES; i++)
    {
        float ang = ang0 + passo * i;
        outX[i] = centerX + RAIO * cosf(ang);
        outZ[i] = centerZ + RAIO * sinf(ang);
    }
}

// ============================================
// Losango verde incandescente (diamante 3D)
// ============================================
static void desenhaLosango(float altura)
{
    float h = altura / 2.0f;
    float s = altura / 3.0f;

    float claro[3] = {0.3f, 1.0f, 0.3f};
    float escuro[3] = {0.0f, 0.6f, 0.0f};

    glBegin(GL_TRIANGLES);
    glColor3fv(claro);
    glVertex3f(0.0f, h, 0.0f);
    glVertex3f(-s, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, s);

    glColor3fv(escuro);
    glVertex3f(0.0f, h, 0.0f);
    glVertex3f(0.0f, 0.0f, s);
    glVertex3f(s, 0.0f, 0.0f);

    glColor3fv(claro);
    glVertex3f(0.0f, h, 0.0f);
    glVertex3f(s, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -s);

    glColor3fv(escuro);
    glVertex3f(0.0f, h, 0.0f);
    glVertex3f(0.0f, 0.0f, -s);
    glVertex3f(-s, 0.0f, 0.0f);

    glColor3fv(claro);
    glVertex3f(0.0f, -h, 0.0f);
    glVertex3f(0.0f, 0.0f, s);
    glVertex3f(-s, 0.0f, 0.0f);

    glColor3fv(escuro);
    glVertex3f(0.0f, -h, 0.0f);
    glVertex3f(s, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, s);

    glColor3fv(claro);
    glVertex3f(0.0f, -h, 0.0f);
    glVertex3f(0.0f, 0.0f, -s);
    glVertex3f(s, 0.0f, 0.0f);

    glColor3fv(escuro);
    glVertex3f(0.0f, -h, 0.0f);
    glVertex3f(-s, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -s);
    glEnd();
}

// Cubo unitário texturizado (-0.5 a 0.5) — todas 6 faces
// tilesX = largura (X), tilesY = altura (Y), tilesZ = profundidade (Z)
static void desenhaCuboTexturizado(float half, float tilesX, float tilesY, float tilesZ)
{
    glBegin(GL_QUADS);
    // Frente (+Z) — largura × altura, V invertido
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, tilesY); glVertex3f(-half, -half, half);
    glTexCoord2f(tilesX, tilesY); glVertex3f(half, -half, half);
    glTexCoord2f(tilesX, 0.0f); glVertex3f(half, half, half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-half, half, half);

    // Trás (-Z) — largura × altura, V invertido
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, tilesY); glVertex3f(half, -half, -half);
    glTexCoord2f(tilesX, tilesY); glVertex3f(-half, -half, -half);
    glTexCoord2f(tilesX, 0.0f); glVertex3f(-half, half, -half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(half, half, -half);

    // Direita (+X) — profundidade × altura, V invertido
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, tilesY); glVertex3f(half, -half, half);
    glTexCoord2f(tilesZ, tilesY); glVertex3f(half, -half, -half);
    glTexCoord2f(tilesZ, 0.0f); glVertex3f(half, half, -half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(half, half, half);

    // Esquerda (-X) — profundidade × altura, V invertido
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, tilesY); glVertex3f(-half, -half, -half);
    glTexCoord2f(tilesZ, tilesY); glVertex3f(-half, -half, half);
    glTexCoord2f(tilesZ, 0.0f); glVertex3f(-half, half, half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-half, half, -half);

    // Topo (+Y) — largura × profundidade
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-half, half, half);
    glTexCoord2f(tilesX, 0.0f); glVertex3f(half, half, half);
    glTexCoord2f(tilesX, tilesZ); glVertex3f(half, half, -half);
    glTexCoord2f(0.0f, tilesZ); glVertex3f(-half, half, -half);

    // Fundo (-Y) — largura × profundidade
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-half, -half, -half);
    glTexCoord2f(tilesX, 0.0f); glVertex3f(half, -half, -half);
    glTexCoord2f(tilesX, tilesZ); glVertex3f(half, -half, half);
    glTexCoord2f(0.0f, tilesZ); glVertex3f(-half, -half, half);
    glEnd();
}

extern float yaw; // definido em camera.cpp

struct Particula
{
    float x, y, z;
    float vx, vy, vz;
    float vida;
    float vidaMax;
    float tamanho;
};

static std::vector<Particula> particulas;
static bool iniciou = false;

static float frand(float a, float b)
{
    return a + (rand() / (float)RAND_MAX) * (b - a);
}

static void respawn(Particula &p)
{
    // Amostra dentro de um losango (octaedro) de altura ~1.5 usado na cena
    const float h = 0.75f;   // metade da altura
    const float base = 0.5f; // raio na base

    p.y = frand(-h, h);
    float crossScale = 1.0f - fabsf(p.y) / h;      // afunila até as pontas
    float r = crossScale * base * frand(0.0f, 1.0f); // raio aleatório no nível atual
    float ang = frand(0.0f, 2.0f * (float)M_PI);
    p.x = cosf(ang) * r;
    p.z = sinf(ang) * r;

    p.vx = frand(-0.25f, 0.25f);
    p.vz = frand(-0.25f, 0.25f);
    p.vy = frand(2.0f, 4.5f);

    p.vidaMax = frand(0.7f, 1.2f);
    p.vida = p.vidaMax;
    p.tamanho = frand(0.3f, 0.55f);
}

void iniciaParticulasFogo(int maxParticulas)
{
    if (iniciou)
        return;

    srand((unsigned)::time(NULL));
    particulas.resize(maxParticulas);
    for (size_t i = 0; i < particulas.size(); i++)
    {
        respawn(particulas[i]);
        particulas[i].vida *= frand(0.0f, 1.0f); // fase inicial aleatória
    }
    iniciou = true;
}

void atualizaParticulasFogo(float dt)
{
    if (!iniciou)
        return;

    for (size_t i = 0; i < particulas.size(); i++)
    {
        Particula &p = particulas[i];
        p.vida -= dt;
        if (p.vida <= 0.0f)
        {
            respawn(p);
            continue;
        }

        // leve turbulência horizontal
        float wiggle = sinf(p.x * 10.0f + p.z * 10.0f + p.vida * 15.0f);
        p.vx += wiggle * 0.02f;
        p.vz += wiggle * 0.02f;

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;

        // sobe mais rápido que espalha
        p.vy += 1.2f * dt;
        p.vx *= 0.98f;
        p.vz *= 0.98f;
    }
}

// Desenha as partículas usando o shader de fogo verde
static void desenhaParticulasFogoShader(const RenderAssets &r, float gameTime)
{
    if (!iniciou || particulas.empty())
        return;

    // Ativa o fire shader
    glUseProgram(r.progFogo);

    GLint locTime = glGetUniformLocation(r.progFogo, "uTime");
    GLint locStr  = glGetUniformLocation(r.progFogo, "uStrength");
    GLint locScrl = glGetUniformLocation(r.progFogo, "uScroll");
    GLint locInt  = glGetUniformLocation(r.progFogo, "uIntensity");
    GLint locTex  = glGetUniformLocation(r.progFogo, "uTexture");

    glUniform1f(locTime, gameTime);
    glUniform1f(locStr, 1.2f);
    glUniform2f(locScrl, 0.0f, -0.3f);
    glUniform1f(locInt, 1.5f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.texLava); // textura de lava como base para distorção
    glUniform1i(locTex, 0);

    // Billboarding simples: roda no Y para encarar a câmera
    glPushMatrix();
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    for (size_t i = 0; i < particulas.size(); i++)
    {
        const Particula &p = particulas[i];
        float t = p.vida / p.vidaMax; // 1 = recém-nascido, 0 = morrendo

        float size = p.tamanho * (0.5f + 0.5f * t);
        float half = size * 0.5f;

        // cor verde (modulada pelo shader)
        float gr = 0.2f;
        float gg = 0.6f + 0.4f * t;
        float gb = 0.1f;
        float ga = t * 0.8f;

        glColor4f(gr, gg, gb, ga);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(p.x - half, p.y - half, p.z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(p.x + half, p.y - half, p.z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(p.x + half, p.y + half, p.z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(p.x - half, p.y + half, p.z);
    }
    glEnd();

    glPopMatrix();
    glUseProgram(0);
}

void drawAltar(float centerX, float centerZ, const RenderAssets &r, float time)
{
    static bool debugOnce = false;
    if (!debugOnce) {
        std::printf("[ALTAR DEBUG] texPilar=%u texAltarChao=%u texLava=%u\n",
                    r.texPilar, r.texAltarChao, r.texLava);
        debugOnce = true;
    }

    float anguloPiramide = time * 30.0f;
    float anguloEsfera = time * 15.0f;

    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT | GL_POINT_BIT);
    glPushMatrix();
    glTranslatef(centerX, 0.0f, centerZ);

    glUseProgram(0);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_NORMALIZE);
    glDisable(GL_CULL_FACE);

    // Aumenta ambient do altar para que faces na sombra não fiquem escuras
    GLfloat altarAmbient[] = {0.45f, 0.35f, 0.30f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, altarAmbient);

    // Garante texture matrix = identidade
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    // ============================================
    // 4 PILARES COM LOSANGOS INCANDESCENTES
    // ============================================
    float alturaTorre = 3.0f;
    float w = 0.8f;
    float ang0 = -((float)M_PI) / 2.0f;
    float passo = 2.0f * (float)M_PI / NUM_PILARES;
    float losangoY = alturaTorre + 1.2f;

    for (int i = 0; i < NUM_PILARES; i++)
    {
        float ang = ang0 + passo * i;
        float x = RAIO * cosf(ang);
        float z = RAIO * sinf(ang);

        glPushMatrix();
        glTranslatef(x, 0.0f, z);

        // Pilar texturizado (565.png)
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r.texPilar);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(0.0f, alturaTorre / 2.0f, 0.0f);
        glScalef(w, alturaTorre, w);
        desenhaCuboTexturizado(0.5f, 1.0f, 2.0f, 1.0f);
        glPopMatrix();

        // --- Losango incandescente ---
        glPushMatrix();
        glDisable(GL_TEXTURE_2D);
        glTranslatef(0.0f, losangoY, 0.0f);
        glRotatef(anguloPiramide, 0.0f, 1.0f, 0.0f);

        // Material emissivo verde (brilho próprio)
        GLfloat emissive[] = {0.5f, 1.0f, 0.3f, 1.0f};
        GLfloat noEmit[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
        desenhaLosango(1.8f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmit);

        // Halo de brilho (losango maior, blending aditivo)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        float pulse = 0.25f + 0.1f * sinf(time * 4.0f + (float)i * 1.5f);
        glColor4f(0.15f, 0.9f, 0.1f, pulse);
        desenhaLosango(2.6f);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glPopMatrix();

        // --- Partículas de fogo verde (shader de fogo, quads billboard) ---
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glTranslatef(0.0f, losangoY, 0.0f);
        desenhaParticulasFogoShader(r, time);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glPopMatrix();

        // Restaura estado para fixed-function texturizado
        glUseProgram(0);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_COLOR_MATERIAL);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glColor3f(1.0f, 1.0f, 1.0f);

        glPopMatrix();
    }

    // ============================================
    // PIRÂMIDE DE 3 DEGRAUS (190.png)
    // ============================================
    float alturaDegrau = 0.6f;
    float tamanhoBase = 5.0f;
    float reducao = 0.65f;

    glUseProgram(0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.texAltarChao);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(0.0f, alturaDegrau / 2.0f, 0.0f);
    glScalef(tamanhoBase, alturaDegrau, tamanhoBase);
    desenhaCuboTexturizado(0.5f, tamanhoBase, alturaDegrau, tamanhoBase);
    glPopMatrix();

    float larg2 = tamanhoBase * reducao;
    glPushMatrix();
    glTranslatef(0.0f, alturaDegrau + alturaDegrau / 2.0f, 0.0f);
    glScalef(larg2, alturaDegrau, larg2);
    desenhaCuboTexturizado(0.5f, larg2, alturaDegrau, larg2);
    glPopMatrix();

    float larg3 = tamanhoBase * reducao * reducao;
    glPushMatrix();
    glTranslatef(0.0f, 2 * alturaDegrau + alturaDegrau / 2.0f, 0.0f);
    glScalef(larg3, alturaDegrau, larg3);
    desenhaCuboTexturizado(0.5f, larg3, alturaDegrau, larg3);
    glPopMatrix();

    // ============================================
    // ESFERA FLUTUANTE (shader de sangue)
    // ============================================
    float topoDegrausY = 3.0f * alturaDegrau;
    float raioEsfera = 2.0f;

    glUseProgram(r.progSangue);

    GLint locTime = glGetUniformLocation(r.progSangue, "uTime");
    GLint locStr = glGetUniformLocation(r.progSangue, "uStrength");
    GLint locSpd = glGetUniformLocation(r.progSangue, "uSpeed");
    GLint locTex = glGetUniformLocation(r.progSangue, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locSpd, 3.0f, 1.7f);

    glPushMatrix();
    glTranslatef(0.0f, topoDegrausY + raioEsfera + 0.3f, 0.0f);
    glRotatef(anguloEsfera, 1.0f, 1.0f, 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.texSangue);
    glUniform1i(locTex, 0);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glScalef(1.5f, 1.5f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    static GLUquadric *quad = nullptr;
    if (!quad)
    {
        quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GL_SMOOTH);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    gluSphere(quad, raioEsfera, 40, 40);

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopMatrix();

    glUseProgram(0);

    glPopMatrix();
    glPopAttrib();
}

// ============================================
// COLISÃO: verifica se (px,pz) intercepta a pirâmide ou os pilares
// ============================================
bool altarBlocksMovement(float centerX, float centerZ, float px, float pz, float playerRadius)
{
    float pyramidHalf = 5.0f / 2.0f;
    float minX = centerX - pyramidHalf;
    float maxX = centerX + pyramidHalf;
    float minZ = centerZ - pyramidHalf;
    float maxZ = centerZ + pyramidHalf;

    float closestX = (px < minX) ? minX : (px > maxX ? maxX : px);
    float closestZ = (pz < minZ) ? minZ : (pz > maxZ ? maxZ : pz);
    float dx = px - closestX;
    float dz = pz - closestZ;
    if (dx * dx + dz * dz < playerRadius * playerRadius)
        return true;

    float pillarHalf = 0.8f / 2.0f;
    float totalR = pillarHalf + playerRadius;
    float totalRSq = totalR * totalR;

    float ang0 = -(float)M_PI / 2.0f;
    float passoA = 2.0f * (float)M_PI / NUM_PILARES;

    for (int i = 0; i < NUM_PILARES; i++)
    {
        float ang = ang0 + passoA * i;
        float pillarX = centerX + RAIO * cosf(ang);
        float pillarZ = centerZ + RAIO * sinf(ang);
        float ddx = px - pillarX;
        float ddz = pz - pillarZ;
        if (ddx * ddx + ddz * ddz < totalRSq)
            return true;
    }

    return false;
}
