#include "Enemy.h"
#include <cmath>
#include <string>

void Enemy::LoadSprites() {
    std::string basePath = "assets/Enemies/";
    std::string folder;
    
    switch (type) {
        case EnemyType::GOTOKU: folder = "Gotoku"; break;
        case EnemyType::ONRE:   folder = "Onre";   break;
        case EnemyType::YUREI:  folder = "Yurei";  break;
    }
    
    std::string path = basePath + folder + "/";
    
    // Загружаем основные спрайты если они существуют
    if (FileExists((path + "Idle.png").c_str()))    texIdle = LoadTexture((path + "Idle.png").c_str());
    if (FileExists((path + "Walk.png").c_str()))    texWalk = LoadTexture((path + "Walk.png").c_str());
    if (FileExists((path + "Attack_1.png").c_str())) texAttack = LoadTexture((path + "Attack_1.png").c_str());
    if (FileExists((path + "Hurt.png").c_str()))    texHurt = LoadTexture((path + "Hurt.png").c_str());
    if (FileExists((path + "Dead.png").c_str()))    texDead = LoadTexture((path + "Dead.png").c_str());
}

void Enemy::UnloadSprites() {
    if (texIdle.id > 0) UnloadTexture(texIdle);
    if (texWalk.id > 0) UnloadTexture(texWalk);
    if (texAttack.id > 0) UnloadTexture(texAttack);
    if (texHurt.id > 0) UnloadTexture(texHurt);
    if (texDead.id > 0) UnloadTexture(texDead);
}

void Enemy::Update(float dt, Vector2 playerPos) {
    if (!alive) return;
    
    // Гравитация (Onre может летать — нет гравитации)
    if (type != EnemyType::ONRE) {
        velocity.y += 1200.0f * dt;
    } else {
        // Onre летает — медленно следует за игроком по Y
        float targetY = playerPos.y - 100;
        velocity.y = (targetY - pos.y) * 2.0f;
    }
    
    float dir = (playerPos.x > pos.x) ? 1.0f : -1.0f;
    facingDirection = (int)dir;
    
    // Поведение в зависимости от типа
    switch (type) {
        case EnemyType::GOTOKU: {
            // Melee — бежит вплотную
            velocity.x = dir * speed;
            break;
        }
        case EnemyType::ONRE: {
            // Ranged — держит дистанцию, летает
            float dist = fabsf(playerPos.x - pos.x);
            if (dist > 300.0f) velocity.x = dir * speed;
            else if (dist < 150.0f) velocity.x = -dir * speed * 0.5f;
            else velocity.x = 0;
            break;
        }
        case EnemyType::YUREI: {
            // Boss — агрессивный, и вблизи и издалека
            float dist = fabsf(playerPos.x - pos.x);
            if (dist > 100.0f) velocity.x = dir * speed * 1.3f;
            else velocity.x = 0;
            break;
        }
    }
    
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    
    // Кулдауны
    if (attackCooldown > 0) attackCooldown -= dt;
    if (shootCooldown > 0) shootCooldown -= dt;
}

void Enemy::Draw() const {
    if (!alive) return;
    
    // Выбираем текстуру
    Texture2D currentTex = texIdle;
    if (isAttacking && texAttack.id > 0) currentTex = texAttack;
    else if (fabsf(velocity.x) > 10 && texWalk.id > 0) currentTex = texWalk;
    
    if (currentTex.id > 0) {
        // ====== НАРЕЗКА SPRITESHEET ======
        // Все спрайты квадратные, высота = размер кадра
        float frameSize = (float)currentTex.height;  // 128 для твоих спрайтов
        int totalFrames = (int)(currentTex.width / frameSize);
        
        // Анимируем: меняем кадр по времени
        static float animTimer = 0;  // общий таймер для всех врагов
        // (в идеале у каждого врага свой таймер, но для простоты пока так)
        
        int currentFrame = 0;
        if (totalFrames > 1) {
            // Простая анимация на основе позиции врага (чтобы не синхронно)
            currentFrame = ((int)(pos.x * 10 + GetTime() * 8)) % totalFrames;
        }
        
        // Вырезаем один кадр из spritesheet
        Rectangle src = {
            currentFrame * frameSize,  // X внутри текстуры
            0,                          // Y (всегда 0 для горизонтального листа)
            frameSize,                  // Ширина одного кадра
            frameSize                   // Высота одного кадра
        };
        
        // Куда рисуем на экране (с отражением по горизонтали)
        Rectangle dest = {
            pos.x - width/2,
            pos.y - height,
            width,
            height
        };
        
        // Отражение если враг смотрит влево
        if (facingDirection < 0) {
            dest.width = -width;  // Отрицательная ширина = отражение
            dest.x = pos.x + width/2;  // Корректируем позицию
        }
        
        DrawTexturePro(currentTex, src, dest, {0, 0}, 0, WHITE);
        
    } else {
        // Запасная геометрия (если спрайты не загрузились)
        Color color;
        switch (type) {
            case EnemyType::GOTOKU: color = {200, 50, 50, 255}; break;
            case EnemyType::ONRE:   color = {100, 100, 200, 255}; break;
            case EnemyType::YUREI:  color = {150, 0, 200, 255}; break;
        }
        DrawRectangle(pos.x - width/2, pos.y - height, width, height, color);
        DrawRectangle(pos.x - width/3, pos.y - 5, width/1.5f, 8, {color.r, color.g, color.b, 100});
    }
    
    // HP bar
    float barW = width;
    DrawRectangle(pos.x - barW/2, pos.y - height - 12, barW, 4, DARKGRAY);
    float hpPercent = (float)hp / maxHp;
    Color hpColor = hpPercent > 0.5f ? GREEN : (hpPercent > 0.25f ? YELLOW : RED);
    DrawRectangle(pos.x - barW/2, pos.y - height - 12, barW * hpPercent, 4, hpColor);
}

Rectangle Enemy::GetRect() const {
    return {pos.x - width/2.5f, pos.y - height + height/4, width/1.25f, height * 0.75f};
}

bool Enemy::CanAttack(Vector2 playerPos) const {
    float dist = sqrtf(powf(playerPos.x - pos.x, 2) + powf(playerPos.y - pos.y, 2));
    return dist < attackRange;
}