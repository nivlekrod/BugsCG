#pragma once

bool isWalkable(float x, float z);
void updateEntities(float dt);

const float ENEMY_SPEED = 2.5f;

enum EnemyState
{
    STATE_IDLE,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_DEAD 
};

struct Enemy
{
    int type;         
    float x, z;       
    float hp;         
    EnemyState state; 
    
    float startX, startZ;
    float respawnTimer;
    
    // Animação e Patrulha
    int animFrame;
    float animTimer;  // Usaremos este timer para decidir quanto tempo ele anda numa direção

    // --- ADICIONE ESTAS LINHAS ---
    float dirX;       // Direção X da patrulha
    float dirZ;       // Direção Z da patrulha
    // ----------------------------

    float attackCooldown; 
    float hurtTimer;      
};

// ADICIONE ESTES BLOCOS:
enum ItemType
{
    ITEM_HEALTH,
    ITEM_AMMO,
    ITEM_AMMO_BOX,
    ITEM_HD  // Caso você use HDs no chão no seu jogo
};

struct Item
{
    float x, z;
    ItemType type;
    bool active; // Se false, já foi pego

    float respawnTimer;
};