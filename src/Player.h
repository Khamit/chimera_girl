#pragma once
#include "raylib.h"
#include <vector>
#include <string>

enum class PlayerForm {
    HUMAN = 0, RABBIT = 1, WOLF = 2, SNAKE = 3, DRAGON = 4, FORM_COUNT = 5
};

enum class GirlType {
    GIRL_1 = 0, GIRL_2 = 1, GIRL_3 = 2
};

struct Animation {
    Texture2D spritesheet;
    int frameCount = 0;
    float frameWidth = 0;
    int currentFrame = 0;
    float frameTime = 0.0f;
    float frameSpeed = 0.12f;
    bool loop = true;
    bool loaded = false;
    
    void Update(float dt);
    void Draw(Vector2 pos, float width, float height, int facingDir, Color tint = WHITE) const;
    void Load(const char* path);
    void Unload();
};

struct Player {
    Vector2 pos = {200, 600};
    Vector2 velocity = {0, 0};
    float speed = 120.0f;
    float jumpForce = -600.0f;
    float width = 48.0f;
    float height = 64.0f;
    bool wasOnGround = false;
    
    int hp = 100;
    int maxHp = 100;
    int energy = 0;
    int maxEnergy = 100;
    
    GirlType girlType = GirlType::GIRL_1;
    PlayerForm currentForm = PlayerForm::HUMAN;
    int facingDirection = 1;
    
    bool onGround = false;
    bool canDoubleJump = false;
    bool hasDoubleJumped = false;
    bool isAttacking = false;
    float attackCooldown = 0.0f;
    float attackRange = 60.0f;
    int attackDamage = 15;
    float attackAnimTimer = 0.0f;
    
    // Анимации игрока
    Animation animIdle;
    Animation animWalk;
    Animation animAttack;       // Атака игрока (Attack.png)
    Animation bookProjectile;   // Снаряд-книга (Book.png) — только Girl_1
    Animation animProtection;
    Animation animDialogue;
    
    bool isRangedAttack() const;
    void TakeDamage(int damage);
    void TransformTo(PlayerForm form);
    void Attack();
    void AddEnergy(int amount);
    Rectangle GetRect() const;
    void Update(float dt);
    void Draw() const;
    void LoadSprites(GirlType girl);
    void UnloadSprites();
};

struct FormStats {
    float speed;
    float jumpForce;
    int maxHp;
    int attackDamage;
    float attackRange;
    Color color;
    const char* name;
    bool canDoubleJump;
    int energyCost;
};

extern FormStats FORM_DATA[];