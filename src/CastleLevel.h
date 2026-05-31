#pragma once
#include "Level.h"  // Наследуем от Level!

class CastleLevel : public Level {
public:
    // Текстуры замка
    Texture2D texBg;
    Texture2D texMountains;
    Texture2D texFloor;
    Texture2D texColumns;
    Texture2D texCandeliar;
    Texture2D texDragon;
    Texture2D texWallWindows;
    
    void LoadTextures() override;
    void UnloadTextures() override;
    void GenerateLevel() override;
    void Draw() const override;
    void DrawBackground(float cameraX) const override;
    void DrawDecorations() const override;
    void DrawEnergyPickups() const override;
    bool CheckCollision(Rectangle rect, Vector2& pos, Vector2& velocity, bool& onGround, bool wasOnGround) const override;
};