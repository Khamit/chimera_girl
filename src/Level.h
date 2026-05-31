#pragma once
#include "raylib.h"
#include <vector>
#include <string>

struct Platform {
    Rectangle rect;
    Texture2D texture;
    bool useTexture = false;
    bool isRamp = false;
    bool rampRight = true;
};

enum class PickupType {
    COIN = 0, DIAMOND = 1, STAR = 2, LIFE = 3
};

struct EnergyPickup {
    Vector2 pos;
    PickupType type = PickupType::COIN;
    int amount = 10;
    int healAmount = 20;
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
    
    // Текстуры (будут использоваться в наследниках)
    Texture2D texSky;
    Texture2D texMoon;
    Texture2D texCloudsBottom;
    Texture2D texCloudsTop;
    Texture2D texGround;
    std::vector<Texture2D> groundTiles;
    std::vector<Texture2D> platformTiles;
    
    float bgOffset = 0;
    
    virtual ~Level() {}
    virtual void LoadTextures();
    virtual void UnloadTextures();
    virtual void GenerateLevel();
    virtual void Draw() const;
    virtual void DrawBackground(float cameraX) const;
    virtual void DrawDecorations() const;
    virtual void DrawEnergyPickups() const;
    virtual bool CheckCollision(Rectangle rect, Vector2& pos, Vector2& velocity, bool& onGround, bool wasOnGround) const;
};