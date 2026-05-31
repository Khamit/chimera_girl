#pragma once
#include "raylib.h"

// Типы призраков из твоих ассетов
enum class EnemyType {
    GOTOKU = 0,    // Ближний бой (Attack_1,2,3 — melee)
    ONRE = 1,      // Дальний бой / летающий (Flight, Attack — ranged)
    YUREI = 2      // Босс (Attack_1-4, Charge — оба типа + спец-атаки)
};

struct Enemy {
    Vector2 pos;
    Vector2 velocity = {0, 0};
    float speed = 120.0f;
    float width = 80.0f;   // Было 40
    float height = 96.0f;  // Было 48
    
    int hp = 3;
    int maxHp = 3;
    int damage = 10;
    
    EnemyType type = EnemyType::GOTOKU;
    float attackRange = 50.0f;
    float attackCooldown = 0.0f;
    float shootCooldown = 0.0f;
    bool onGround = false;
    bool alive = true;
    bool isAttacking = false;
    int facingDirection = 1;

    // === НОВЫЕ ПОЛЯ ДЛЯ ONRE (орбитальные снаряды) ===
    int orbCount = 6;           // Количество орбов вокруг Onre
    float orbAngle = 0.0f;      // Базовый угол вращения
    float orbRadius = 60.0f;    // Радиус орбиты
    float orbSpeed = 3.0f;      // Скорость вращения
    bool invulnerable = false;  // Неуязвимость пока есть орбы
    float rechargeTimer = 0.0f; // Таймер перезарядки
    bool recharging = false;    // Флаг перезарядки
    
    // Анимации
    Texture2D texIdle;
    Texture2D texWalk;
    Texture2D texAttack;
    Texture2D texHurt;
    Texture2D texDead;
    
    void LoadSprites();  // Загружает спрайты из assets/Enemies/
    void Update(float dt, Vector2 playerPos);
    void Draw() const;
    Rectangle GetRect() const;
    bool CanAttack(Vector2 playerPos) const;
    void UnloadSprites();
};

struct Projectile {
    Vector2 pos;
    Vector2 dir;
    float speed = 120.0f;
    float radius = 6.0f;
    int damage = 15;
    bool active = true;
    bool fromPlayer = false;
    Texture2D tex;  // Спрайт пули (можно Coin_01 или Star)
};