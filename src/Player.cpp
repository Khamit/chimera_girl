#include "Player.h"
#include <cmath>
#include <string>

FormStats FORM_DATA[] = {
    {300.0f, -600.0f, 100, 15, 60.0f, {255, 200, 200, 255}, "Schoolgirl", false, 0},
    {500.0f, -650.0f, 80,  20, 45.0f, {255, 220, 240, 255}, "Rabbit", true, 20},
    {380.0f, -550.0f, 110, 35, 55.0f, {180, 180, 200, 255}, "Wolf", false, 50},
    {200.0f, -450.0f, 160, 50, 75.0f, {100, 200, 100, 255}, "Snake", false, 100},
    {350.0f, -700.0f, 200, 60, 80.0f, {255, 100, 50, 255}, "Dragon", true, 150}
};

void Animation::Update(float dt) {
    if (!loaded || frameCount <= 0) return;
    frameTime += dt;
    if (frameTime >= frameSpeed) {
        frameTime = 0;
        currentFrame++;
        if (currentFrame >= frameCount) {
            currentFrame = loop ? 0 : frameCount - 1;
        }
    }
}

void Animation::Draw(Vector2 pos, float width, float height, int facingDir, Color tint) const {
    if (!loaded || frameCount <= 0) return;
    
    Rectangle src = {
        currentFrame * frameWidth,
        0,
        frameWidth,
        (float)spritesheet.height
    };
    
    // Начинаем с обычного прямоугольника
    Rectangle dest = {
        pos.x - width/2,
        pos.y - height,
        width,
        height
    };
    
    // ОТРАЖЕНИЕ ПО ГОРИЗОНТАЛИ через отрицательную ширину
    if (facingDir < 0) {
        dest.width = -width;          // Отрицательная ширина = зеркальное отражение
        dest.x = pos.x + width/2;     // Компенсируем смещение
    }
    
    DrawTexturePro(spritesheet, src, dest, {0, 0}, 0, tint);
}

void Animation::Load(const char* path) {
    if (FileExists(path)) {
        spritesheet = LoadTexture(path);
        loaded = true;
        frameWidth = (float)spritesheet.height;
        frameCount = spritesheet.width / (int)frameWidth;
        currentFrame = 0;
        frameTime = 0;
    }
}

void Animation::Unload() {
    if (loaded) {
        UnloadTexture(spritesheet);
        loaded = false;
        frameCount = 0;
    }
}

void Player::LoadSprites(GirlType girl) {
    girlType = girl;
    
    std::string girlFolder;
    switch (girl) {
        case GirlType::GIRL_1: girlFolder = "Girl_1"; break;
        case GirlType::GIRL_2: girlFolder = "Girl_2"; break;
        case GirlType::GIRL_3: girlFolder = "Girl_3"; break;
    }
    
    std::string basePath = "assets/Player/" + girlFolder + "/";
    
    UnloadSprites();
    
    animIdle.Load((basePath + "Idle.png").c_str());
    animIdle.frameSpeed = 0.12f;
    
    animWalk.Load((basePath + "Walk.png").c_str());
    animWalk.frameSpeed = 0.08f;
    
    // АНИМАЦИЯ АТАКИ ИГРОКА (все девочки)
    animAttack.Load((basePath + "Attack.png").c_str());
    animAttack.frameSpeed = 0.06f;
    animAttack.loop = false;
    
    // СНАРЯД-КНИГА (только Girl_1) — 10 кадров
    if (girl == GirlType::GIRL_1) {
        bookProjectile.Load((basePath + "Book.png").c_str());
        bookProjectile.frameSpeed = 0.06f;  // 10 кадров × 0.06 = 0.6 сек анимации
        bookProjectile.loop = true;         // Зацикливаем пока летит
    }
    
    animProtection.Load((basePath + "Protection.png").c_str());
    animProtection.frameSpeed = 0.15f;
    
    animDialogue.Load((basePath + "Dialogue.png").c_str());
    animDialogue.frameSpeed = 0.12f;
}

void Player::UnloadSprites() {
    animIdle.Unload();
    animWalk.Unload();
    animAttack.Unload();
    bookProjectile.Unload();
    animProtection.Unload();
    animDialogue.Unload();
}

void Player::TakeDamage(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Player::TransformTo(PlayerForm form) {
    if (form == currentForm) return;
    
    FormStats& stats = FORM_DATA[(int)form];
    if (form != PlayerForm::HUMAN && energy < stats.energyCost) return;
    if (form != PlayerForm::HUMAN) energy -= stats.energyCost;
    
    currentForm = form;
    speed = stats.speed;
    jumpForce = stats.jumpForce;
    maxHp = stats.maxHp;
    hp = maxHp;
    attackDamage = stats.attackDamage;
    attackRange = stats.attackRange;
    canDoubleJump = stats.canDoubleJump;
    hasDoubleJumped = false;
}

void Player::Attack() {
    if (attackCooldown > 0) return;
    
    isAttacking = true;
    attackAnimTimer = 0.4f;
    attackCooldown = 0.5f;
    
    // Сбрасываем анимацию АТАКИ ИГРОКА
    animAttack.currentFrame = 0;
    animAttack.frameTime = 0;
}

bool Player::isRangedAttack() const {
    return girlType == GirlType::GIRL_1 && bookProjectile.loaded;
}

void Player::AddEnergy(int amount) {
    energy += amount;
    if (energy > maxEnergy) energy = maxEnergy;
}

Rectangle Player::GetRect() const {
    return {pos.x - width/3, pos.y - height + 4, width/1.5f, height - 4};
}

void Player::Update(float dt) {
    wasOnGround = onGround;
    
    // ЕДИНАЯ гравитация
    velocity.y += 1200.0f * dt;
    
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    
    // Трение
    float friction = onGround ? 0.8f : 0.95f;
    velocity.x *= friction;
    
    if (attackCooldown > 0) attackCooldown -= dt;
    
    if (isAttacking) {
        attackAnimTimer -= dt;
        if (attackAnimTimer <= 0) isAttacking = false;
    }
    
    if (isAttacking) {
        animAttack.Update(dt);
    } else if (fabsf(velocity.x) > 10 && onGround) {
        animWalk.Update(dt);
    } else {
        animIdle.Update(dt);
    }
}

void Player::Draw() const {
    FormStats& stats = FORM_DATA[(int)currentForm];
    
    const Animation* currentAnim = &animIdle;
    if (isAttacking && animAttack.loaded) currentAnim = &animAttack;
    else if (fabsf(velocity.x) > 10 && onGround && animWalk.loaded) currentAnim = &animWalk;
    
    // === СИНЕЕ СВЕЧЕНИЕ ДЛЯ КРОЛИКА ===
    if (currentForm == PlayerForm::RABBIT) {
        Color glowColor = {50, 100, 255, 60};  // Синий полупрозрачный
        float glowRadius = width * 0.9f;
        float glowY = pos.y - height/2;
        
        // Внешнее свечение
        DrawCircleGradient(pos.x, glowY, glowRadius + 8, 
                          {50, 100, 255, 40}, {50, 100, 255, 0});
        // Внутреннее свечение
        DrawCircleGradient(pos.x, glowY, glowRadius + 4,
                          {100, 150, 255, 30}, {50, 100, 255, 0});
        // Лёгкое мерцание
        float pulse = 1.0f + sinf(GetTime() * 8.0f) * 0.15f;
        DrawEllipse(pos.x, glowY, glowRadius * pulse, glowRadius * 0.6f * pulse, 
                    {80, 140, 255, 20});
    }
    
    if (currentAnim->loaded && currentAnim->frameCount > 0) {
        // Вырезаем кадр из spritesheet
        Rectangle src = {
            (float)(currentAnim->currentFrame) * currentAnim->frameWidth,
            0,
            currentAnim->frameWidth,
            (float)currentAnim->spritesheet.height
        };
        
        // Позиция на экране
        Rectangle dest = {
            pos.x - width/2.0f,
            pos.y - height,
            width,
            height
        };
        
        // ОТРАЖЕНИЕ ЧЕРЕЗ SRC (работает на всех GPU)
        if (facingDirection < 0) {
            src.width = -src.width;  // Отрицательная ширина src = зеркальное отражение
        }
        
        DrawTexturePro(currentAnim->spritesheet, src, dest, {0, 0}, 0, WHITE);
    } else {
        Color c = stats.color;
        DrawRectangle(pos.x - width/2, pos.y - height, width, height, c);
    }
    
    DrawText(stats.name, pos.x - 25, pos.y - height - 25, 14, WHITE);
    
    float barW = width;
    DrawRectangle(pos.x - barW/2, pos.y - height - 15, barW, 5, DARKGRAY);
    DrawRectangle(pos.x - barW/2, pos.y - height - 15, 
                  barW * ((float)energy / maxEnergy), 5, {0, 200, 255, 255});
}