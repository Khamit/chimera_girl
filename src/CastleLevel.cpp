#include "CastleLevel.h"
#include <cstdio>
#include <cmath>

const int SCREEN_W = 1280;
const int SCREEN_H = 720;

void CastleLevel::LoadTextures() {
    if (FileExists("assets/castle/interior/bg.png"))
        texBg = LoadTexture("assets/castle/interior/bg.png");
    
    if (FileExists("assets/castle/interior/mountaims.png"))
        texMountains = LoadTexture("assets/castle/interior/mountaims.png");
    
    if (FileExists("assets/castle/interior/floor.png"))
        texFloor = LoadTexture("assets/castle/interior/floor.png");
    
    if (FileExists("assets/castle/interior/columns&falgs.png"))
        texColumns = LoadTexture("assets/castle/interior/columns&falgs.png");
    
    if (FileExists("assets/castle/interior/candeliar.png"))
        texCandeliar = LoadTexture("assets/castle/interior/candeliar.png");
    
    if (FileExists("assets/castle/interior/dragon.png"))
        texDragon = LoadTexture("assets/castle/interior/dragon.png");
    
    if (FileExists("assets/castle/interior/wall@windows.png"))
        texWallWindows = LoadTexture("assets/castle/interior/wall@windows.png");
}

void CastleLevel::UnloadTextures() {
    if (texBg.id > 0) UnloadTexture(texBg);
    if (texMountains.id > 0) UnloadTexture(texMountains);
    if (texFloor.id > 0) UnloadTexture(texFloor);
    if (texColumns.id > 0) UnloadTexture(texColumns);
    if (texCandeliar.id > 0) UnloadTexture(texCandeliar);
    if (texDragon.id > 0) UnloadTexture(texDragon);
    if (texWallWindows.id > 0) UnloadTexture(texWallWindows);
}

void CastleLevel::GenerateLevel() {
    platforms.clear();
    energyPickups.clear();
    decorations.clear();
    
    int groundY = 680;
    int tileSize = 64;
    int levelLength = 60;
    
    // ====== 1. ПОЛ (floor.png тайлингом) ======
    int floorTileW = 128;  // Предполагаемый размер тайла пола
    for (int x = 0; x < levelLength * tileSize; x += floorTileW) {
        Platform p;
        p.rect = {(float)x, (float)groundY, (float)floorTileW, (float)tileSize};
        p.isRamp = false;
        p.texture = texFloor;
        p.useTexture = true;
        platforms.push_back(p);
    }
    
    // ====== 2. ВЕРХНИЕ ПЛАТФОРМЫ ======
    struct UpperPlat { int startX; int length; int heightOffset; };
    UpperPlat upperPlats[] = {
        {8, 6, 2},     // Высокая площадка
        {20, 5, 1},    // Средняя площадка
        {30, 7, 2},    // Ещё одна высокая
        {42, 6, 1},    // Средняя
    };
    int numUpper = sizeof(upperPlats) / sizeof(upperPlats[0]);
    
    for (int s = 0; s < numUpper; s++) {
        UpperPlat& up = upperPlats[s];
        float y = groundY - up.heightOffset * tileSize;
        
        for (int i = 0; i < up.length; i++) {
            Platform p;
            p.rect = {(float)((up.startX + i) * tileSize), y, (float)tileSize, 20};
            p.isRamp = false;
            p.texture = texFloor;
            p.useTexture = true;
            platforms.push_back(p);
        }
    }
    
    // ====== 3. ПАРЯЩИЕ ПЛАТФОРМЫ ======
    struct FloatingPlat { float x, y; int tilesWide; };
    FloatingPlat floatingPlats[] = {
        {400, 520, 3}, {900, 440, 4}, {1400, 500, 3}, {1900, 400, 5},
        {2400, 460, 4}, {2900, 380, 3}, {3300, 440, 4},
    };
    int numFloating = sizeof(floatingPlats) / sizeof(floatingPlats[0]);
    
    for (int i = 0; i < numFloating; i++) {
        for (int t = 0; t < floatingPlats[i].tilesWide; t++) {
            Platform p;
            p.rect = {floatingPlats[i].x + t * tileSize, floatingPlats[i].y, (float)tileSize, 16};
            p.isRamp = false;
            p.texture = texFloor;
            p.useTexture = true;
            platforms.push_back(p);
        }
    }
    
    // ====== 4. ДЕКОРАЦИИ ЗАМКА ======
    // Колонны с флагами — каждые 10 тайлов
    if (texColumns.id > 0) {
        for (int x = 2; x < levelLength; x += 10) {
            Decoration d;
            d.pos = {(float)(x * tileSize), (float)(groundY - texColumns.height * 1.5f)};
            d.texture = texColumns;
            d.scale = 1.5f;
            decorations.push_back(d);
        }
    }
    
    // Люстры — сверху
    if (texCandeliar.id > 0) {
        for (int x = 5; x < levelLength; x += 12) {
            Decoration d;
            d.pos = {(float)(x * tileSize), 20.0f};
            d.texture = texCandeliar;
            d.scale = 1.2f;
            decorations.push_back(d);
        }
    }
    
    // Стены с окнами — на заднем плане
    if (texWallWindows.id > 0) {
        for (int x = 0; x < levelLength; x += 8) {
            Decoration d;
            d.pos = {(float)(x * tileSize), (float)(groundY - texWallWindows.height - 100)};
            d.texture = texWallWindows;
            d.scale = 1.0f;
            decorations.push_back(d);
        }
    }
    
    // ====== 5. ПИКАПЫ ======
    struct PickupInfo {
        const char* path;
        PickupType type;
        int amount;
        int healAmount;
    };
    PickupInfo pickups[] = {
        {"assets/platform_ground/Collectable Object/Coin_01.png", PickupType::COIN, 8, 0},
        {"assets/platform_ground/Collectable Object/Diamond.png", PickupType::DIAMOND, 18, 0},
        {"assets/platform_ground/Collectable Object/Star.png", PickupType::STAR, 28, 0},
        {"assets/platform_ground/Collectable Object/Life.png", PickupType::LIFE, 0, 25},
    };
    int numPickupTypes = sizeof(pickups) / sizeof(pickups[0]);
    
    for (int i = 0; i < 20; i++) {
        EnergyPickup ep;
        ep.pos = {(float)GetRandomValue(150, levelLength * tileSize - 150), (float)(groundY - GetRandomValue(40, 130))};
        int r = GetRandomValue(0, numPickupTypes - 1);
        ep.type = pickups[r].type;
        ep.amount = pickups[r].amount + GetRandomValue(-3, 3);
        ep.healAmount = pickups[r].healAmount;
        if (FileExists(pickups[r].path)) { ep.texture = LoadTexture(pickups[r].path); }
        energyPickups.push_back(ep);
    }
    
    for (int i = 0; i < 10; i++) {
        int platIdx = GetRandomValue(0, numFloating - 1);
        EnergyPickup ep;
        ep.pos = {floatingPlats[platIdx].x + GetRandomValue(20, (int)(floatingPlats[platIdx].tilesWide * tileSize - 40)), floatingPlats[platIdx].y - 30};
        int r = GetRandomValue(0, numPickupTypes - 1);
        ep.type = pickups[r].type;
        ep.amount = pickups[r].amount + GetRandomValue(-5, 5);
        ep.healAmount = pickups[r].healAmount;
        if (FileExists(pickups[r].path)) { ep.texture = LoadTexture(pickups[r].path); }
        energyPickups.push_back(ep);
    }
    
    // ====== 6. ДРАКОН (ЧЕКПОИНТ) ======
    if (texDragon.id > 0) {
        Decoration dragon;
        dragon.isCheckpoint = true;
        dragon.texture = texDragon;
        dragon.scale = 2.5f;
        dragon.pos = {(float)((levelLength - 3) * tileSize), (float)(groundY - texDragon.height * dragon.scale)};
        decorations.push_back(dragon);
    }
}

void CastleLevel::Draw() const {
    for (size_t i = 0; i < platforms.size(); i++) {
        const Platform& p = platforms[i];
        if (p.useTexture && p.texture.id > 0) {
            DrawTexturePro(p.texture,
                {0, 0, (float)p.texture.width, (float)p.texture.height},
                p.rect, {0, 0}, 0, WHITE);
        } else {
            DrawRectangleRec(p.rect, {100, 100, 100, 255});
        }
    }
}

void CastleLevel::DrawBackground(float cameraX) const {
    float viewLeft = cameraX - SCREEN_W / 2.0f;
    
    // 1. Задний план (bg.png) — заполняет экран
    if (texBg.id > 0) {
        for (int i = 0; i < 3; i++) {
            DrawTexturePro(texBg,
                {0, 0, (float)texBg.width, (float)texBg.height},
                {viewLeft + i * texBg.width * 1.5f - fmodf(viewLeft * 0.02f, texBg.width * 1.5f), -50,
                 texBg.width * 1.5f, (float)SCREEN_H + 50},
                {0, 0}, 0, WHITE);
        }
    }
    
    // 2. Горы (mountaims.png) — медленный параллакс
    if (texMountains.id > 0) {
        for (int i = 0; i < 3; i++) {
            DrawTexturePro(texMountains,
                {0, 0, (float)texMountains.width, (float)texMountains.height},
                {viewLeft + i * texBg.width * 1.5f - fmodf(viewLeft * 0.02f, texBg.width * 1.5f), -50.0f,
                    texBg.width * 1.5f, (float)SCREEN_H + 50.0f},
                {0, 0}, 0, WHITE);
        }
    }
}

void CastleLevel::DrawDecorations() const {
    for (size_t i = 0; i < decorations.size(); i++) {
        const Decoration& d = decorations[i];
        if (d.texture.id > 0) {
            Rectangle src = {0, 0, (float)d.texture.width, (float)d.texture.height};
            Rectangle dest = {d.pos.x, d.pos.y, d.texture.width * d.scale, d.texture.height * d.scale};
            DrawTexturePro(d.texture, src, dest, {0, 0}, 0, WHITE);
            
            if (d.isCheckpoint) {
                DrawText("BOSS", d.pos.x + (d.texture.width * d.scale) / 2 - 25, d.pos.y - 25, 18, RED);
            }
        }
    }
}

void CastleLevel::DrawEnergyPickups() const {
    for (size_t i = 0; i < energyPickups.size(); i++) {
        const EnergyPickup& ep = energyPickups[i];
        if (ep.collected) continue;
        
        if (ep.texture.id > 0) {
            DrawTexturePro(ep.texture,
                {0, 0, (float)ep.texture.width, (float)ep.texture.height},
                {ep.pos.x - ep.radius, ep.pos.y - ep.radius, ep.radius * 2, ep.radius * 2},
                {0, 0}, 0, WHITE);
        } else {
            DrawCircleV(ep.pos, ep.radius, {0, 200, 255, 255});
        }
    }
}

bool CastleLevel::CheckCollision(Rectangle rect, Vector2& pos, Vector2& velocity, bool& onGround, bool wasOnGround) const {
    onGround = false;
    
    for (size_t i = 0; i < platforms.size(); i++) {
        const Platform& plat = platforms[i];
        const Rectangle& r = plat.rect;
        
        if (!CheckCollisionRecs(rect, r)) continue;
        
        float overlapLeft = (rect.x + rect.width) - r.x;
        float overlapRight = (r.x + r.width) - rect.x;
        float overlapTop = (rect.y + rect.height) - r.y;
        float overlapBottom = (r.y + r.height) - rect.y;
        
        float minOverlapX = fminf(overlapLeft, overlapRight);
        float minOverlapY = fminf(overlapTop, overlapBottom);
        
        if (minOverlapX < minOverlapY) {
            if (overlapLeft < overlapRight) {
                pos.x = r.x - rect.width/2;
            } else {
                pos.x = r.x + r.width + rect.width/2;
            }
            velocity.x = 0;
        } else {
            if (overlapTop < overlapBottom) {
                pos.y = r.y;
                velocity.y = 0;
                onGround = true;
            } else {
                pos.y = r.y + r.height + rect.height;
                velocity.y = 0;
            }
        }
    }
    return onGround;
}