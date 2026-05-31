#include "raylib.h"
#include "Player.h"
#include "Enemy.h"
#include "Level.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include "CastleLevel.h"

const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int LEVEL_WIDTH = 60 * 64;

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "Chimera Girl - Spirit Hunters");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);

    // Загрузка звуков
    Sound coinSound = LoadSound("assets/sound/coin_sound.mp3");
    Sound attackSound = LoadSound("assets/sound/attack_s.mp3");
    Sound punchSound = LoadSound("assets/sound/punch_s.mp3");
    Music bgm = LoadMusicStream("assets/sound/ghost_music.mp3");
    SetMusicVolume(bgm, 0.05f);   // — 5% громкости
    PlayMusicStream(bgm);
        
    Player player;
    player.pos = {200, 600};
    
    // Один указатель:
    Level* currentLevel = nullptr;
    Level forestLevel;
    CastleLevel castleLevel;

    // Загрузка первого уровня
    currentLevel = &forestLevel;
    currentLevel->LoadTextures();
    currentLevel->GenerateLevel();
    
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    
    Camera2D camera = {0};
    camera.target = {player.pos.x, player.pos.y - 100};
    camera.offset = {SCREEN_W/2.0f, SCREEN_H/2.0f};
    camera.rotation = 0;
    camera.zoom = 2.0f;
    
    int waveNumber = 0;
    float waveTimer = 3.0f;
    bool gameOver = false;
    bool showGirlSelect = true;
    int selectedGirl = 0;
    
    // Главный цикл
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // ЗВУК: 
        UpdateMusicStream(bgm);
        
        // ====== ЭКРАН ВЫБОРА ДЕВОЧКИ ======
        if (showGirlSelect) {
            if (IsKeyPressed(KEY_ONE)) { 
                selectedGirl = 0; 
                player.LoadSprites(GirlType::GIRL_1);  // ДОБАВИТЬ
                showGirlSelect = false; 
            }
            if (IsKeyPressed(KEY_TWO)) { 
                selectedGirl = 1; 
                player.LoadSprites(GirlType::GIRL_2);  // ДОБАВИТЬ
                showGirlSelect = false; 
            }
            if (IsKeyPressed(KEY_THREE)) { 
                selectedGirl = 2; 
                player.LoadSprites(GirlType::GIRL_3);  // ДОБАВИТЬ
                showGirlSelect = false; 
            }
            
            BeginDrawing();
            ClearBackground({10, 10, 30, 255});
            DrawText("CHOOSE YOUR HUNTER", SCREEN_W/2 - 200, 100, 36, WHITE);
            DrawText("1. Girl 1 (Magic/Book)", SCREEN_W/2 - 150, 250, 24, {255, 200, 200, 255});
            DrawText("2. Girl 2 (Protection)", SCREEN_W/2 - 150, 320, 24, {200, 255, 200, 255});
            DrawText("3. Girl 3 (Attack?)", SCREEN_W/2 - 150, 390, 24, {200, 200, 255, 255});
            DrawText("Press 1, 2, or 3", SCREEN_W/2 - 100, 500, 20, GRAY);
            EndDrawing();
            continue;
        }
        
        if (!gameOver) {
            // ====== ВВОД ======
            Vector2 moveInput = {0, 0};
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveInput.x = -1;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveInput.x = 1;
            
            bool jumpPressed = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE);
            bool attackPressed = IsKeyPressed(KEY_J) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            
            // Трансформации (с проверкой энергии)
            if (IsKeyPressed(KEY_ONE)) player.TransformTo(PlayerForm::HUMAN);
            if (IsKeyPressed(KEY_TWO) && player.energy >= FORM_DATA[1].energyCost) player.TransformTo(PlayerForm::RABBIT);
            if (IsKeyPressed(KEY_THREE) && player.energy >= FORM_DATA[2].energyCost) player.TransformTo(PlayerForm::WOLF);
            if (IsKeyPressed(KEY_FOUR) && player.energy >= FORM_DATA[3].energyCost) player.TransformTo(PlayerForm::SNAKE);
            
            // ====== ОБНОВЛЕНИЕ ======
            player.velocity.x = moveInput.x * player.speed;
            if (moveInput.x != 0) player.facingDirection = (moveInput.x > 0) ? 1 : -1;
            
            // Прыжок
            if (jumpPressed && player.onGround) {
                player.velocity.y = player.jumpForce;
                player.onGround = false;
                // ЗВУК: PlaySound(jumpSound);
            } else if (jumpPressed && player.canDoubleJump && !player.hasDoubleJumped) {
                player.velocity.y = player.jumpForce * 0.8f;
                player.hasDoubleJumped = true;
                // ЗВУК: PlaySound(doubleJumpSound);
            }
            
            player.Update(dt);
            
            // Коллизия игрока
            Rectangle playerRect = player.GetRect();
            currentLevel->CheckCollision(playerRect, player.pos, player.velocity, player.onGround, player.wasOnGround);
            if (player.onGround) player.hasDoubleJumped = false;
            
            // Границы
            if (player.pos.x < 10) player.pos.x = 10;
            if (player.pos.x > LEVEL_WIDTH) player.pos.x = LEVEL_WIDTH;
            if (player.pos.y > SCREEN_H + 200) {
                player.TakeDamage(999);
                gameOver = true;
            }
            
        // Атака игрока
        if (attackPressed && player.attackCooldown <= 0) {
            player.Attack();
            PlaySound(attackSound);
            
            // Проверяем попадание по орбам Onre
            for (auto& e : enemies) {
                if (!e.alive || e.type != EnemyType::ONRE || e.orbCount <= 0) continue;
                
                // Проверяем столкновение атаки с орбами
                float attackX = player.pos.x + player.facingDirection * (player.width/2 + player.attackRange/2);
                float attackY = player.pos.y - player.height/2;
                
                for (int i = e.orbCount - 1; i >= 0; i--) {
                    float angle = e.orbAngle + (i * 2 * PI / e.orbCount);
                    float ox = e.pos.x + cosf(angle) * e.orbRadius;
                    float oy = e.pos.y + sinf(angle) * e.orbRadius;
                    
                    if (CheckCollisionCircles({attackX, attackY}, player.attackRange, {ox, oy}, 10)) {
                        e.orbCount--;
                        
                        // Если все орбы сбиты — враг уязвим и начинается перезарядка
                        if (e.orbCount <= 0) {
                            e.invulnerable = false;
                            e.recharging = true;
                            e.rechargeTimer = 4.5f;
                        }
                        break;  // Одна атака = один орб
                    }
                }
            }
            
            // Melee атака по врагам (только если они не неуязвимы)
            for (auto& e : enemies) {
                if (!e.alive || e.invulnerable) continue;  // ← ПРОПУСКАЕМ НЕУЯЗВИМЫХ
                
                float dist = sqrtf(powf(e.pos.x - player.pos.x, 2) + powf(e.pos.y - player.pos.y, 2));
                if (dist < player.attackRange) {
                    e.hp -= player.attackDamage;
                    e.isAttacking = false;
                    PlaySound(punchSound);
                    if (e.hp <= 0) {
                        e.alive = false;
                        player.AddEnergy(15);
                    }
                }
            }
        
                
        if (player.isRangedAttack()) {
            Projectile p;
            
            // Начальная позиция: перед игроком + немного вверх (на уровне рук)
            p.pos = {
                player.pos.x + player.facingDirection * (player.width/2 + 10),  // Перед игроком
                player.pos.y - player.height/2  // На уровне середины тела (рук)
            };
            
            // Направление: вперёд и немного вверх (книга летит по дуге)
            p.dir = {
                (float)player.facingDirection,  // Горизонтально в сторону взгляда
                -0.3f                            // Немного вверх в начале
            };
            
            p.speed = 350.0f;       // Скорость полёта
            p.radius = 20.0f;       // Размер хитбокса
            p.damage = player.attackDamage + 5;  // Книга бьёт сильнее melee
            p.fromPlayer = true;
            
            // Спрайт книги
            if (player.bookProjectile.loaded) {
                p.tex = player.bookProjectile.spritesheet;
            }
            
            projectiles.push_back(p);
        } else {
                    // Другие девочки: melee-атака
                    for (auto& e : enemies) {
                        float dist = sqrtf(powf(e.pos.x - player.pos.x, 2) + powf(e.pos.y - player.pos.y, 2));
                        if (dist < player.attackRange && e.alive) {
                            e.hp -= player.attackDamage;
                            e.isAttacking = false;
                            PlaySound(punchSound);
                            if (e.hp <= 0) {
                                e.alive = false;
                                player.AddEnergy(15);
                            }
                        }
                    }
                }
            }
            
            // Сбор пикапов
            for (auto& ep : currentLevel->energyPickups) {
                if (ep.collected) continue;
                if (CheckCollisionCircles(player.pos, 30, ep.pos, ep.radius)) {
                    
                    switch (ep.type) {
                        case PickupType::COIN:
                        case PickupType::DIAMOND:
                        case PickupType::STAR:
                            player.AddEnergy(ep.amount);
                            PlaySound(coinSound);  // ← ВОТ ЗДЕСЬ
                            break;
                        case PickupType::LIFE:
                            player.hp += ep.healAmount;
                            if (player.hp > player.maxHp) player.hp = player.maxHp;
                            PlaySound(coinSound);  // ← ИЛИ ЗДЕСЬ (звук лечения)
                            break;
                    }
                    ep.collected = true;
                }
            }

            // Проверка контрольной точки (руины)
            for (auto& d : currentLevel->decorations) {
                if (d.texture.id > 0 && d.isCheckpoint) {
                    Rectangle checkpointRect = {
                        d.pos.x,
                        d.pos.y,
                        d.texture.width * d.scale,
                        d.texture.height * d.scale
                    };
                    if (CheckCollisionRecs(player.GetRect(), checkpointRect)) {
                        // Достигли контрольной точки!
                        gameOver = true;  // Или можно показать сообщение
                        // ЗВУК: PlaySound(checkpointSound);
                    }
                }
            }
            
            // Очистка собранного
            currentLevel->energyPickups.erase(
                std::remove_if(currentLevel->energyPickups.begin(), currentLevel->energyPickups.end(),
                    [](const EnergyPickup& ep) { return ep.collected; }),
                currentLevel->energyPickups.end()
            );
            
            // Враги
            for (auto& e : enemies) {
                if (!e.alive) continue;
                e.Update(dt, player.pos);
                
                // Коллизия врагов 
                // (им не нужен wasOnGround, передаём их же onGround)
                Rectangle enemyRect = e.GetRect();
                currentLevel->CheckCollision(enemyRect, e.pos, e.velocity, e.onGround, e.onGround);
                
                // Melee атака (Gotoku, Yurei)
                if (e.type == EnemyType::GOTOKU || e.type == EnemyType::YUREI) {
                    if (e.CanAttack(player.pos) && e.attackCooldown <= 0) {
                        player.TakeDamage(e.damage);
                        e.attackCooldown = 1.2f;
                        e.isAttacking = true;
                        // ЗВУК: PlaySound(enemyAttackSound);
                    }
                }
                
                // Ranged атака (Onre, Yurei)
                if ((e.type == EnemyType::ONRE || e.type == EnemyType::YUREI) && e.shootCooldown <= 0) {
                    float dx = player.pos.x - e.pos.x;
                    float dy = player.pos.y - e.pos.y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    
                    if (dist < 400) {
                        Projectile p;
                        p.pos = e.pos;
                        p.dir = {dx/dist, dy/dist};
                        p.damage = e.damage;
                        p.fromPlayer = false;
                        projectiles.push_back(p);
                        e.shootCooldown = 2.0f;
                        // ЗВУК: PlaySound(enemyShootSound);
                    }
                }
                
                if (e.isAttacking && e.attackCooldown <= 0.5f) e.isAttacking = false;
            }
            
            // Очистка мёртвых врагов
            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(),
                    [](const Enemy& e) { return !e.alive; }),
                enemies.end()
            );
            
            // ====== ОБНОВЛЕНИЕ ПУЛЬ ======
            for (auto& p : projectiles) {
                if (!p.active) continue;
                
                // Движение пули
                p.pos.x += p.dir.x * p.speed * dt;
                p.pos.y += p.dir.y * p.speed * dt;
                
                // Гравитация для снарядов игрока (книга падает по дуге)
                if (p.fromPlayer) {
                    // Сначала проверяем орбы Onre
                    for (auto& e : enemies) {
                        if (!e.alive || e.type != EnemyType::ONRE || e.orbCount <= 0) continue;
                        
                        for (int i = e.orbCount - 1; i >= 0; i--) {
                            float angle = e.orbAngle + (i * 2 * PI / e.orbCount);
                            float ox = e.pos.x + cosf(angle) * e.orbRadius;
                            float oy = e.pos.y + sinf(angle) * e.orbRadius;
                            
                            if (CheckCollisionCircles(p.pos, p.radius, {ox, oy}, 10)) {
                                e.orbCount--;
                                p.active = false;
                                if (e.orbCount <= 0) {
                                    e.invulnerable = false;
                                    e.recharging = true;
                                    e.rechargeTimer = 2.5f;
                                }
                                break;
                            }
                        }
                        if (!p.active) break;
                    }
                    
                    // Потом проверяем врагов (только уязвимых)
                    if (p.active) {
                        for (auto& e : enemies) {
                            if (!e.alive || e.invulnerable) continue;
                            if (CheckCollisionCircles(p.pos, p.radius, {e.pos.x, e.pos.y - e.height/2}, 25)) {
                                e.hp -= p.damage;
                                p.active = false;
                                PlaySound(punchSound);
                                if (e.hp <= 0) { e.alive = false; player.AddEnergy(15); }
                                break;
                            }
                        }
                    }
                    p.dir.y += 2.3f * dt;  // Постепенно опускается вниз
                    
                    // Нормализуем направление чтобы скорость оставалась постоянной
                    float len = sqrtf(p.dir.x * p.dir.x + p.dir.y * p.dir.y);
                    if (len > 0) {
                        p.dir.x /= len;
                        p.dir.y /= len;
                    }
                }
                
                // Выход за границы
                if (p.pos.x < 0 || p.pos.x > LEVEL_WIDTH || p.pos.y < 0 || p.pos.y > SCREEN_H) {
                    p.active = false;
                    continue;
                }
                
                // Попадание снаряда игрока во врагов
                if (p.fromPlayer) {
                    for (auto& e : enemies) {
                        if (!e.alive) continue;
                        if (CheckCollisionCircles(p.pos, p.radius, {e.pos.x, e.pos.y - e.height/2}, 25)) {
                            e.hp -= p.damage;
                            p.active = false;
                            PlaySound(punchSound);
                            if (e.hp <= 0) {
                                e.alive = false;
                                player.AddEnergy(15);
                            }
                            break;
                        }
                    }
                }
                
                // Попадание вражеского снаряда в игрока
                if (!p.fromPlayer && p.active) {
                    if (CheckCollisionCircles(p.pos, p.radius, player.pos, 25)) {
                        player.TakeDamage(p.damage);
                        p.active = false;
                    }
                }
            }

            // Очистка неактивных пуль
            projectiles.erase(
                std::remove_if(projectiles.begin(), projectiles.end(),
                    [](const Projectile& p) { return !p.active; }),
                projectiles.end()
            );
            
            // Волны — враги выходят группами по 2-3
            waveTimer -= dt;
            if (waveTimer <= 0) {
                // Если все враги из текущей волны убиты — начинаем новую волну
                if (enemies.empty()) {
                    waveNumber++;
                    // Босс каждые 5 волн
                    if (waveNumber % 5 == 0) {
                        Enemy e;
                        e.pos = {
                            player.pos.x + (GetRandomValue(0, 1) ? 450.0f : -450.0f),
                            600.0f
                        };
                        e.type = EnemyType::YUREI;
                        e.hp = 15 + waveNumber * 5;
                        e.maxHp = e.hp;
                        e.width = 60;
                        e.height = 72;
                        e.damage = 20 + waveNumber;
                        e.speed = 80 + waveNumber * 10;
                        e.LoadSprites();
                        enemies.push_back(e);
                        waveTimer = 2.0f;
                    } else {
                        // Обычная волна: спавним 2-3 врагов сразу
                        int spawnCount = 2 + GetRandomValue(0, 1);  // 2 или 3 врага
                        
                        for (int j = 0; j < spawnCount; j++) {
                            Enemy e;
                            
                            // Разные стороны спавна для разнообразия
                            float spawnX = player.pos.x + (GetRandomValue(0, 1) ? 400.0f + j * 100 : -400.0f - j * 100);
                            if (spawnX < 50) spawnX = 50;
                            if (spawnX > LEVEL_WIDTH - 50) spawnX = LEVEL_WIDTH - 50;
                            
                            e.pos = {spawnX, 600.0f};
                            
                            // Распределение типов
                            int r = GetRandomValue(0, 100);
                            if (waveNumber < 3) {
                                e.type = EnemyType::GOTOKU;
                            } else if (waveNumber < 6) {
                                e.type = (r < 70) ? EnemyType::GOTOKU : EnemyType::ONRE;
                            } else {
                                if (r < 50) e.type = EnemyType::GOTOKU;
                                else if (r < 85) e.type = EnemyType::ONRE;
                                else e.type = EnemyType::YUREI;
                            }
                            
                            // Статы
                            e.hp = 2 + waveNumber;
                            e.maxHp = e.hp;
                            e.speed = 100 + waveNumber * 10;
                            e.damage = 8 + waveNumber * 2;
                            e.width = 80.0f;
                            e.height = 96.0f;
                            e.attackRange = 70.0f;
                            
                            // Типовые особенности
                            if (e.type == EnemyType::ONRE) {
                                e.hp = 1 + waveNumber;
                                e.maxHp = e.hp;
                                e.damage = 10 + waveNumber * 2;
                                e.width = 70.0f;
                                e.height = 84.0f;
                                e.attackRange = 65.0f;
                                e.orbCount = 6;
                                e.invulnerable = true;
                                e.orbRadius = 60.0f;
                            }
                            if (e.type == EnemyType::YUREI) {
                                e.hp = 5 + waveNumber * 2;
                                e.maxHp = e.hp;
                                e.width = 110.0f;
                                e.height = 132.0f;
                                e.damage = 15 + waveNumber * 2;
                                e.speed = 90 + waveNumber * 8;
                                e.attackRange = 85.0f;
                            }
                            
                            e.LoadSprites();
                            enemies.push_back(e);
                        }
                        waveTimer = 3.0f;  // Пауза между волнами
                    }
                }
            }
            
            if (player.hp <= 0) gameOver = true;
            
            // Камера
            camera.target.x += (player.pos.x - camera.target.x) * 0.1f;
            camera.target.y += (player.pos.y - 100 - camera.target.y) * 0.1f;
            if (camera.target.x < SCREEN_W/2/camera.zoom) camera.target.x = SCREEN_W/2/camera.zoom;
            if (camera.target.x > LEVEL_WIDTH - SCREEN_W/2/camera.zoom) camera.target.x = LEVEL_WIDTH - SCREEN_W/2/camera.zoom;
        }
        
        // ====== РЕНДЕР ======
        BeginDrawing();
        ClearBackground({15, 15, 30, 255});
        
        BeginMode2D(camera);
        currentLevel->DrawBackground(camera.target.x);  // Фон с параллаксом
        currentLevel->Draw();
        currentLevel->DrawDecorations();
        currentLevel->DrawEnergyPickups();
        
        for (auto& e : enemies) e.Draw();
        
        for (auto& p : projectiles) {
            if (!p.active) continue;
            
            if (p.fromPlayer && p.tex.id > 0) {
                float frameW = (float)p.tex.height;
                int totalFrames = p.tex.width / (int)frameW;
                int frame = ((int)(GetTime() * 14)) % totalFrames;
                
                Rectangle src = {(float)frame * frameW, 0, frameW, (float)p.tex.height};
                Rectangle dest = {p.pos.x - p.radius, p.pos.y - p.radius, p.radius * 2, p.radius * 2};
                
                // Книга всегда смотрит в сторону полёта
                if (p.dir.x < 0) {
                    dest.width = -dest.width;
                    dest.x += p.radius * 2;
                }
                
                DrawTexturePro(p.tex, src, dest, {0, 0}, 0, WHITE);
            } else {
                Color c = p.fromPlayer ? YELLOW : Color{255, 100, 100, 255};
                DrawCircleV(p.pos, p.radius, c);
            }
        }
        
        player.Draw();
        EndMode2D();

        // ====== FPS СЧЁТЧИК (полупрозрачный) ======
        int fps = GetFPS();
        Color fpsColor;
        if (fps >= 55) {
            fpsColor = {0, 255, 0, 180};  // Зелёный полупрозрачный
        } else if (fps >= 45) {
            fpsColor = {255, 255, 0, 180};  // Жёлтый
        } else {
            fpsColor = {255, 0, 0, 180};  // Красный
        }
        DrawText(TextFormat("FPS: %d", fps), SCREEN_W - 120, 10, 20, fpsColor);

        
        // ====== UI ======
        // Панель игрока
        DrawRectangle(10, 10, 240, 145, {0, 0, 0, 180});
        DrawRectangle(15, 15, 230, 135, {30, 30, 50, 200});
        
        DrawText(TextFormat("HP: %d/%d", player.hp, player.maxHp), 25, 20, 16, RED);
        
        // Энергия
        DrawText("ENERGY", 25, 42, 14, {0, 200, 255, 255});
        DrawRectangle(25, 58, 200, 12, DARKGRAY);
        DrawRectangle(25, 58, 200 * ((float)player.energy/player.maxEnergy), 12, {0, 200, 255, 255});
        DrawText(TextFormat("%d/%d", player.energy, player.maxEnergy), 230, 56, 12, WHITE);
        
        // Форма
        DrawText(TextFormat("Form: %s", FORM_DATA[(int)player.currentForm].name), 25, 78, 16, YELLOW);
        
        // Волна
        DrawText(TextFormat("Wave: %d", waveNumber), 25, 100, 18, ORANGE);
        DrawText(TextFormat("Enemies: %d", (int)enemies.size()), 25, 120, 14, RED);
        
        // Подсказки управления
        DrawText("1:Human 2:Rabbit(20) 3:Wolf(50) 4:Snake(100)", 10, SCREEN_H - 60, 14, GRAY);
        DrawText("WASD:Move  Space:Jump  J:Attack", 10, SCREEN_H - 40, 14, GRAY);
        
        // Game Over / Checkpoint Menu
        if (gameOver) {
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 200});
            
            bool reachedCheckpoint = false;
            for (auto& d : currentLevel->decorations) {
                if (d.isCheckpoint) {
                    Rectangle cr = {d.pos.x, d.pos.y, d.texture.width * d.scale, d.texture.height * d.scale};
                    if (CheckCollisionRecs(player.GetRect(), cr)) {
                        reachedCheckpoint = true;
                    }
                }
            }
            
            if (reachedCheckpoint) {
                // Меню после прохождения уровня
                DrawText("LEVEL COMPLETE!", SCREEN_W/2 - 180, SCREEN_H/2 - 80, 36, GREEN);
                DrawText("You found the ancient ruins", SCREEN_W/2 - 180, SCREEN_H/2 - 30, 20, WHITE);
                
                // Кнопки выбора
                DrawText("N - Next Level (Castle)", SCREEN_W/2 - 150, SCREEN_H/2 + 40, 22, YELLOW);
                DrawText("R - Restart This Level", SCREEN_W/2 - 150, SCREEN_H/2 + 70, 22, {200, 200, 200, 255});
                DrawText("M - Main Menu", SCREEN_W/2 - 150, SCREEN_H/2 + 100, 22, {200, 200, 200, 255});
                
                if (IsKeyPressed(KEY_N)) {
                    player.UnloadSprites();
                    player = Player();
                    player.pos = {200, 600};
                    player.LoadSprites((GirlType)selectedGirl);
                    enemies.clear();
                    projectiles.clear();
                    
                    // Выгружаем старый уровень
                    currentLevel->UnloadTextures();
                    
                    // Переключаем указатель на замок
                    currentLevel = &castleLevel;
                    
                    // Загружаем замок
                    currentLevel->LoadTextures();
                    currentLevel->GenerateLevel();
                    
                    waveNumber = 0;
                    waveTimer = 3.0f;
                    gameOver = false;
                }
                
                if (IsKeyPressed(KEY_R)) {
                    // Рестарт этого же уровня
                    player.UnloadSprites();
                    player = Player();
                    player.pos = {200, 600};
                    player.LoadSprites((GirlType)selectedGirl);
                    enemies.clear();
                    projectiles.clear();
                    currentLevel->UnloadTextures();
                    currentLevel->LoadTextures();
                    currentLevel->GenerateLevel();
                    waveNumber = 0;
                    waveTimer = 3.0f;
                    gameOver = false;
                }
                
                if (IsKeyPressed(KEY_M)) {
                    player.UnloadSprites();
                    player = Player();
                    enemies.clear();
                    projectiles.clear();
                    
                    currentLevel->UnloadTextures();
                    currentLevel = &forestLevel;
                    currentLevel->LoadTextures();
                    currentLevel->GenerateLevel();
                    
                    waveNumber = 0;
                    waveTimer = 3.0f;
                    gameOver = false;
                    showGirlSelect = true;
                }
                
            } else {
                // Обычное Game Over (смерть)
                DrawText("GAME OVER", SCREEN_W/2 - 130, SCREEN_H/2 - 30, 40, RED);
                DrawText("Press R to restart", SCREEN_W/2 - 100, SCREEN_H/2 + 20, 20, WHITE);
                
                if (IsKeyPressed(KEY_R)) {
                    player.UnloadSprites();
                    player = Player();
                    player.pos = {200, 600};
                    player.LoadSprites((GirlType)selectedGirl);
                    enemies.clear();
                    projectiles.clear();
                    currentLevel->UnloadTextures();
                    currentLevel->LoadTextures();
                    currentLevel->GenerateLevel();
                    waveNumber = 0;
                    waveTimer = 3.0f;
                    gameOver = false;
                }
            }
        }
                
        EndDrawing();
    }
    
    // Очистка
    currentLevel->UnloadTextures();
    for (auto& e : enemies) e.UnloadSprites();
    // Очистка звуков
    UnloadSound(coinSound);
    UnloadSound(attackSound);
    UnloadSound(punchSound);
    UnloadMusicStream(bgm);
    CloseAudioDevice(); 
    
    CloseWindow();
    return 0;
}