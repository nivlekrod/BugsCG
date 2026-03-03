#include "graphics/particles.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>
#include <cstdlib>
#include <cmath>

struct BloodParticle
{
    float x, y, z;
    float vx, vy, vz;
    float life;
    float size;
};

static std::vector<BloodParticle> gParticles;
static const int MAX_PARTICLES = 500;

void spawnBloodParticles(float x, float y, float z)
{
    int count = 8 + (std::rand() % 6);
    for (int i = 0; i < count && (int)gParticles.size() < MAX_PARTICLES; i++)
    {
        BloodParticle p;
        p.x = x + ((std::rand() % 20) - 10) / 40.0f;
        p.y = y;
        p.z = z + ((std::rand() % 20) - 10) / 40.0f;

        p.vx = ((std::rand() % 200) - 100) / 100.0f * 1.5f;
        p.vy = ((std::rand() % 150)) / 100.0f * 2.0f;
        p.vz = ((std::rand() % 200) - 100) / 100.0f * 1.5f;

        p.life = 0.4f + (std::rand() % 40) / 100.0f;
        p.size = 0.04f + (std::rand() % 8) / 100.0f;

        gParticles.push_back(p);
    }
}

void updateParticles(float dt)
{
    for (int i = (int)gParticles.size() - 1; i >= 0; i--)
    {
        auto &p = gParticles[i];
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
        p.vy -= 6.0f * dt;
        p.life -= dt;

        if (p.life <= 0.0f || p.y < 0.0f)
        {
            gParticles[i] = gParticles.back();
            gParticles.pop_back();
        }
    }
}

void renderParticles(float camX, float camY, float camZ)
{
    if (gParticles.empty())
        return;

    (void)camY;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (auto &p : gParticles)
    {
        float alpha = p.life / 0.8f;
        if (alpha > 1.0f)
            alpha = 1.0f;

        float r = 0.5f + (std::rand() % 20) / 100.0f;
        glColor4f(r, 0.0f, 0.0f, alpha);

        float s = p.size;

        float dx = camX - p.x;
        float dz = camZ - p.z;
        float angle = std::atan2(dx, dz) * 180.0f / 3.14159f;

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
        glVertex3f(-s, -s, 0);
        glVertex3f(s, -s, 0);
        glVertex3f(s, s, 0);
        glVertex3f(-s, s, 0);
        glEnd();

        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

void clearParticles()
{
    gParticles.clear();
}
