#pragma once
#include "raylib.h"
#include <vector>
#include <string>

struct Platform {
    Rectangle rect;
    Texture2D texture;
    bool useTexture = false;
    bool isRamp = false;       // Это склон?
    bool rampRight = true;     // true = подъём вправо (Ground_01), false = подъём влево (Ground_03)
};

enum class PickupType {
    COIN = 0,    // Энергия
    DIAMOND = 1, // Много энергии
    STAR = 2,    // Очень много энергии
    LIFE = 3     // Восстановление HP
};

struct EnergyPickup {
    Vector2 pos;
    PickupType type = PickupType::COIN;
    int amount = 10;        // Для энергии
    int healAmount = 20;    // Для лечения
    float radius = 12.0f;
    bool collected = false;
    Texture2D texture;
};

struct Decoration {
    Vector2 pos;
    Texture2D texture;
    float scale = 1.0f;
    bool isCheckpoint = false;
};

class Level {
public:
    std::vector<Platform> platforms;
    std::vector<EnergyPickup> energyPickups;
    std::vector<Decoration> decorations;
    
    // Текстуры для уровня
    Texture2D texSky;           // 1.png — фон неба
    Texture2D texMoon;          // 2.png — луна
    Texture2D texCloudsBottom;  // 3.png — нижние облака
    Texture2D texCloudsTop;     // 4.png — верхние облака
    Texture2D texGround;
    std::vector<Texture2D> groundTiles;
    std::vector<Texture2D> platformTiles;
    
    float bgOffset = 0;  // Смещение для параллакс-эффекта
    
    void LoadTextures();
    void UnloadTextures();
    void GenerateLevel();
    void Draw() const;
    void DrawBackground(float cameraX) const;  // Новый метод для фона
    void DrawDecorations() const;
    void DrawEnergyPickups() const;
    bool CheckCollision(Rectangle rect, Vector2& pos, Vector2& velocity, bool& onGround, bool wasOnGround) const;
};