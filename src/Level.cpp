#include "Level.h"
#include <algorithm>
#include <string>
#include <cstdio>
#include <cmath>

const int SCREEN_W = 1280;
const int SCREEN_H = 720;

void Level::LoadTextures() {
    if (FileExists("assets/background/night/1.png"))
        texSky = LoadTexture("assets/background/night/1.png");
    
    if (FileExists("assets/background/night/2.png"))
        texMoon = LoadTexture("assets/background/night/2.png");
    
    if (FileExists("assets/background/night/3.png"))
        texCloudsBottom = LoadTexture("assets/background/night/3.png");
    
    if (FileExists("assets/background/night/4.png"))
        texCloudsTop = LoadTexture("assets/background/night/4.png");
    
    for (int i = 1; i <= 13; i++) {
        char pathBuffer[256];
        if (i < 10) {
            snprintf(pathBuffer, sizeof(pathBuffer), 
                     "assets/platform_ground/Platformer/Ground_0%d.png", i);
        } else {
            snprintf(pathBuffer, sizeof(pathBuffer), 
                     "assets/platform_ground/Platformer/Ground_%d.png", i);
        }
        if (FileExists(pathBuffer)) {
            groundTiles.push_back(LoadTexture(pathBuffer));
        }
    }
}

void Level::UnloadTextures() {
    if (texSky.id > 0) UnloadTexture(texSky);
    if (texMoon.id > 0) UnloadTexture(texMoon);
    if (texCloudsBottom.id > 0) UnloadTexture(texCloudsBottom);
    if (texCloudsTop.id > 0) UnloadTexture(texCloudsTop);
    for (auto& t : groundTiles) UnloadTexture(t);
    for (auto& t : platformTiles) UnloadTexture(t);
}

void Level::GenerateLevel() {
    platforms.clear();
    energyPickups.clear();
    decorations.clear();
    
    int groundY = 680;
    int tileSize = 64;
    int levelLength = 60;
    
    int IDX_FLAT = 1;
    int IDX_EDGE_LEFT = 3;
    int IDX_EDGE_RIGHT = 7;
    int IDX_PLAT_LEFT = 9;
    int IDX_PLAT_MID = 10;
    int IDX_PLAT_RIGHT = 11;
    
    // ====== 1. ЗЕМЛЯ (сплошная) ======
    for (int x = 0; x < levelLength; x++) {
        Platform p;
        p.rect = {(float)(x * tileSize), (float)groundY, (float)tileSize, (float)tileSize};
        p.isRamp = false;
        
        int tileIdx = IDX_FLAT;
        if (x == 0) tileIdx = IDX_EDGE_LEFT;
        else if (x == levelLength - 1) tileIdx = IDX_EDGE_RIGHT;
        
        if (tileIdx < (int)groundTiles.size()) {
            p.texture = groundTiles[tileIdx];
            p.useTexture = true;
        }
        platforms.push_back(p);
    }
    
    // ====== 2. ВЕРХНИЕ ПЛОЩАДКИ (ровные, без рамп) ======
    struct UpperPlat {
        int startX;
        int length;
        int heightOffset;  // В тайлах над землёй
    };
    
    UpperPlat upperPlats[] = {
        {8, 8, 1},    // Площадка на высоте 1 тайл
        {32, 10, 1},  // Площадка на высоте 1 тайл
    };
    int numUpper = sizeof(upperPlats) / sizeof(upperPlats[0]);
    
    for (int s = 0; s < numUpper; s++) {
        UpperPlat& up = upperPlats[s];
        float y = groundY - up.heightOffset * tileSize;
        
        for (int i = 0; i < up.length; i++) {
            Platform p;
            p.rect = {(float)((up.startX + i) * tileSize), y, (float)tileSize, (float)tileSize};
            p.isRamp = false;
            
            if (IDX_FLAT < (int)groundTiles.size()) {
                p.texture = groundTiles[IDX_FLAT];
                p.useTexture = true;
            }
            platforms.push_back(p);
        }
    }
    
    // ====== 3. ПАРЯЩИЕ ПЛАТФОРМЫ ======
    struct FloatingPlat { float x, y; int tilesWide; };
    FloatingPlat floatingPlats[] = {
        {590, 620, 3}, {900, 460, 4}, {1300, 500, 3}, {1700, 400, 5},
        {2100, 480, 4}, {2500, 380, 3}, {2800, 450, 4}, {3100, 520, 3},
    };
    int numFloating = sizeof(floatingPlats) / sizeof(floatingPlats[0]);
    
    for (int i = 0; i < numFloating; i++) {
        for (int t = 0; t < floatingPlats[i].tilesWide; t++) {
            Platform p;
            p.rect = {floatingPlats[i].x + t * tileSize, floatingPlats[i].y, (float)tileSize, 20};
            p.isRamp = false;
            int tileIdx = (t == 0) ? IDX_PLAT_LEFT : ((t == floatingPlats[i].tilesWide - 1) ? IDX_PLAT_RIGHT : IDX_PLAT_MID);
            if (tileIdx < (int)groundTiles.size()) { p.texture = groundTiles[tileIdx]; p.useTexture = true; }
            platforms.push_back(p);
        }
    }
    
    // ====== 4. ПИКАПЫ ======
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
    
    for (int i = 0; i < 15; i++) {
        EnergyPickup ep;
        ep.pos = {(float)GetRandomValue(150, levelLength * tileSize - 150), (float)(groundY - GetRandomValue(50, 150))};
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
    
    // ====== 5. ЧЕКПОИНТ ======
    Decoration checkpoint;
    checkpoint.isCheckpoint = true;
    checkpoint.scale = 2.0f;
    checkpoint.pos = {(float)((levelLength - 2) * tileSize), (float)(groundY - 256 * checkpoint.scale)};
    if (FileExists("assets/platform_ground/Environment/Decor_Ruins_01.png")) {
        checkpoint.texture = LoadTexture("assets/platform_ground/Environment/Decor_Ruins_01.png");
    }
    decorations.push_back(checkpoint);
}

void Level::Draw() const {
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

void Level::DrawBackground(float cameraX) const {
    // ВАЖНО: в режиме камеры все координаты МИРОВЫЕ
    // cameraX — это центр камеры в мировых координатах
    
    float viewLeft = cameraX - SCREEN_W / 2.0f;  // Левый край видимой области
    float viewTop = 0;  // Верх экрана в мировых координатах
    
    // 1. Небо — заполняет всё видимое пространство
    if (texSky.id > 0) {
        float parallaxX = viewLeft * 0.05f;
        
        // Рисуем небо несколько раз чтобы покрыть весь экран
        int numTiles = (SCREEN_W / 300) + 2;  // 300 — примерная ширина после растяжения
        for (int i = 0; i < numTiles; i++) {
            Rectangle dest = {
                viewLeft + i * 600 - fmodf(parallaxX, 600),
                viewTop,
                600,
                (float)SCREEN_H
            };
            DrawTexturePro(texSky,
                {0, 0, (float)texSky.width, (float)texSky.height},
                dest, {0, 0}, 0, WHITE);
        }
    }
    
    // 2. Луна — фиксирована в правой части неба
    if (texMoon.id > 0) {
        float moonX = viewLeft + SCREEN_W - 250;
        float moonY = viewTop + 50;
        DrawTexturePro(texMoon,
            {0, 0, (float)texMoon.width, (float)texMoon.height},
            {moonX, moonY, 120, 120},
            {0, 0}, 0, WHITE);
    }
    
    // 3. Верхние облака (4.png) — медленный параллакс
    if (texCloudsTop.id > 0) {
        float parallaxX = viewLeft * 0.1f;
        float cloudY = viewTop + 20;
        int numTiles = (SCREEN_W / texCloudsTop.width) + 2;
        
        for (int i = 0; i < numTiles; i++) {
            float x = viewLeft + i * texCloudsTop.width - fmodf(parallaxX, texCloudsTop.width);
            DrawTexturePro(texCloudsTop,
                {0, 0, (float)texCloudsTop.width, (float)texCloudsTop.height},
                {x, cloudY, (float)texCloudsTop.width, 150},
                {0, 0}, 0, WHITE);
        }
    }
    
    // 4. Нижние облака (3.png) — быстрый параллакс (ближе к зрителю)
    if (texCloudsBottom.id > 0) {
        float parallaxX = viewLeft * 0.2f;
        float cloudY = viewTop + SCREEN_H - 250;
        int numTiles = (SCREEN_W / texCloudsBottom.width) + 2;
        
        for (int i = 0; i < numTiles; i++) {
            float x = viewLeft + i * texCloudsBottom.width - fmodf(parallaxX, texCloudsBottom.width);
            DrawTexturePro(texCloudsBottom,
                {0, 0, (float)texCloudsBottom.width, (float)texCloudsBottom.height},
                {x, cloudY, (float)texCloudsBottom.width, 150},
                {0, 0}, 0, WHITE);
        }
    }
}

void Level::DrawDecorations() const {
    for (size_t i = 0; i < decorations.size(); i++) {
        const Decoration& d = decorations[i];
        if (d.texture.id > 0) {
            Rectangle src = {0, 0, (float)d.texture.width, (float)d.texture.height};
            Rectangle dest = {
                d.pos.x,
                d.pos.y,
                d.texture.width * d.scale,
                d.texture.height * d.scale
            };
            DrawTexturePro(d.texture, src, dest, {0, 0}, 0, WHITE);
            
            // Для чекпоинта рисуем подсказку
            if (d.isCheckpoint) {
                DrawText("CHECKPOINT",
                    d.pos.x + (d.texture.width * d.scale) / 2 - 40,
                    d.pos.y - 30,
                    16, YELLOW);
            }
        }
    }
}

void Level::DrawEnergyPickups() const {
    for (size_t i = 0; i < energyPickups.size(); i++) {
        const EnergyPickup& ep = energyPickups[i];
        if (ep.collected) continue;
        
        if (ep.texture.id > 0) {
            DrawTexturePro(ep.texture,
                {0, 0, (float)ep.texture.width, (float)ep.texture.height},
                {ep.pos.x - ep.radius, ep.pos.y - ep.radius, ep.radius * 2, ep.radius * 2},
                {0, 0}, 0, WHITE);
            
            // Подсказка что даёт предмет
            const char* label = "";
            Color labelColor = WHITE;
            switch (ep.type) {
                case PickupType::COIN:   label = TextFormat("+%dE", ep.amount); labelColor = YELLOW; break;
                case PickupType::DIAMOND: label = TextFormat("+%dE", ep.amount); labelColor = {0, 255, 255, 255}; break;
                case PickupType::STAR:   label = TextFormat("+%dE", ep.amount); labelColor = GOLD; break;
                case PickupType::LIFE:   label = TextFormat("+%dHP", ep.healAmount); labelColor = RED; break;
            }
            DrawText(label, ep.pos.x - 15, ep.pos.y - 25, 12, labelColor);
        } else {
            DrawCircleV(ep.pos, ep.radius, {0, 200, 255, 255});
        }
    }
}

bool Level::CheckCollision(Rectangle rect, Vector2& pos, Vector2& velocity, bool& onGround, bool wasOnGround) const {
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