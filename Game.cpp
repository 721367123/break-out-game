#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

// 构造函数
Game::Game() : score(0), lives(3), currentLevel(1), 
               gameRunning(true), gamePaused(false),
               screenWidth(800), screenHeight(600),
               saveFilePath("savegame.json") {
    
    InitWindow(screenWidth, screenHeight, "Arkanoid Game");
    SetTargetFPS(60);
    
    paddle.Init();
    ball.SetPosition(screenWidth / 2, screenHeight - 50);
    ball.SetSpeed(0, -4.0f);
    
    // 检查是否有存档
    std::ifstream checkFile(saveFilePath);
    bool hasSave = checkFile.good();
    checkFile.close();
    
    if (hasSave && LoadGame()) {
        // 有存档，询问是否继续
        bool continueGame = false;
        
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);
            
            DrawText("SAVE FILE FOUND!", screenWidth / 2 - 100, screenHeight / 2 - 80, 25, YELLOW);
            DrawText(TextFormat("Level: %d   Score: %d   Lives: %d", currentLevel, score, lives), 
                     screenWidth / 2 - 200, screenHeight / 2 - 30, 20, WHITE);
            DrawText("[Y] Continue", screenWidth / 2 - 60, screenHeight / 2 + 30, 20, GREEN);
            DrawText("[N] New Game", screenWidth / 2 - 60, screenHeight / 2 + 60, 20, RED);
            
            EndDrawing();
            
            if (IsKeyPressed(KEY_Y)) {
                continueGame = true;
                break;
            }
            if (IsKeyPressed(KEY_N)) {
                continueGame = false;
                break;
            }
        }
        
        if (continueGame) {
            // 继续游戏，LoadGame() 已经加载了关卡
            std::cout << "Continue game - Level " << currentLevel << std::endl;
        } else {
            // 新游戏
            score = 0;
            lives = 3;
            currentLevel = 1;
            std::remove(saveFilePath.c_str());
            LoadLevel(currentLevel);
            std::cout << "Start new game" << std::endl;
        }
    } else {
        // 没有存档，直接开始新游戏
        LoadLevel(currentLevel);
        std::cout << "No save file, start new game" << std::endl;
    }
}

// 析构函数
Game::~Game() {
    CloseWindow();
}
// 默认关卡（兜底方案）
bool Game::LoadDefaultLevel() {
    bricks.clear();
    int brickWidth = 60;
    int brickHeight = 25;
    int rows = 5;
    int cols = 10;
    int startX = (screenWidth - cols * brickWidth) / 2;
    int startY = 50;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            Brick brick(startX + col * brickWidth, startY + row * brickHeight, 
                       brickWidth, brickHeight, BLUE, 1);
            bricks.push_back(brick);
        }
    }
    return true;
}

// 从JSON加载关卡
bool Game::LoadLevelFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开关卡文件: " << filename << std::endl;
        return false;
    }
    
    try {
        json levelData;
        file >> levelData;
        bricks.clear();
        
        int brickWidth = levelData.value("brick_width", 50);
        int brickHeight = levelData.value("brick_height", 20);
        int startX = levelData.value("start_x", 50);
        int startY = levelData.value("start_y", 50);
        
        auto& layout = levelData["layout"];
        int rows = layout.size();
        int cols = layout[0].size();
        
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int brickType = layout[row][col];
                if (brickType > 0) {
                    Color color;
                    int hp = 1;
                    switch(brickType) {
                        case 1: color = RED; hp = 1; break;
                        case 2: color = ORANGE; hp = 2; break;
                        case 3: color = YELLOW; hp = 3; break;
                        case 4: color = GREEN; hp = 1; break;
                        default: color = BLUE; hp = 1;
                    }
                    Brick brick(startX + col * brickWidth, 
                               startY + row * brickHeight, 
                               brickWidth, brickHeight, color, hp);
                    bricks.push_back(brick);
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return false;
    }
}
// 加载指定关卡
bool Game::LoadLevel(int level) {
    std::string filename = "level_" + std::to_string(level) + ".json";
    
    if (LoadLevelFromJSON(filename)) {
        currentLevel = level;
        ball.SetPosition(screenWidth / 2, screenHeight - 50);
        ball.SetSpeed(0, -4.0f);
        paddle.SetPosition(screenWidth / 2 - 50, screenHeight - 30);
        return true;
    }
    
    std::cerr << "使用默认砖块配置" << std::endl;
    return LoadDefaultLevel();
}

// 保存游戏
void Game::SaveGame() {
    json saveData;
    saveData["score"] = score;
    saveData["lives"] = lives;
    saveData["current_level"] = currentLevel;
    saveData["version"] = 1;
    saveData["remaining_bricks"] = bricks.size();
    
    std::ofstream file(saveFilePath);
    if (file.is_open()) {
        file << saveData.dump(4);
        std::cout << "游戏已保存！" << std::endl;
    } else {
        std::cerr << "保存失败！" << std::endl;
    }
}
// 加载存档
bool Game::LoadGame() {
    std::ifstream file(saveFilePath);
    if (!file.is_open()) {
        return false;
    }
    
    try {
        json saveData;
        file >> saveData;
        
        int version = saveData.value("version", 0);
        if (version < 1) {
            std::cout << "检测到旧版存档，尝试迁移..." << std::endl;
        }
        
        score = saveData.value("score", 0);
        lives = saveData.value("lives", 3);
        int savedLevel = saveData.value("current_level", 1);
        
        std::cout << "发现存档：第" << savedLevel << "关，分数：" << score 
                  << "，生命：" << lives << std::endl;
        
        currentLevel = savedLevel;
        return LoadLevel(currentLevel);
        
    } catch (const std::exception& e) {
        std::cerr << "加载存档失败: " << e.what() << std::endl;
        return false;
    }
}

// 检查关卡完成
void Game::CheckLevelCompletion() {
    // 检查是否还有活跃的砖块
    for (const auto& brick : bricks) {
        if (brick.IsActive()) {
            return;  // 还有砖块，不进入下一关
        }
    }
    // 没有活跃砖块了，进入下一关
    bricks.clear();  // 清空数组
    NextLevel();
}
// 下一关
void Game::NextLevel() {
    if (currentLevel < 3) {
        currentLevel++;
        LoadLevel(currentLevel);
        score += 500;
        std::cout << "进入第 " << currentLevel << " 关！" << std::endl;
    } else {
        std::cout << "恭喜通关！游戏胜利！" << std::endl;
        gameRunning = false;
    }
}

// 主更新循环
// 主更新循环
void Game::Update() {
    if (IsKeyPressed(KEY_R)) {
    // 重置游戏
    score = 0;
    lives = 3;
    currentLevel = 1;
    LoadLevel(currentLevel);
    ball.SetPosition(screenWidth / 2, screenHeight - 50);
    ball.SetSpeed(0, -4.0f);
    paddle.SetPosition(screenWidth / 2 - 50, screenHeight - 30);
    gameRunning = true;
    
    // 删除存档文件
    std::remove(saveFilePath.c_str());
}
    if (!gameRunning || gamePaused) return;
    
    // ========== 1. 更新挡板位置（响应键盘） ==========
    paddle.Update();
    
    // ========== 2. 更新球的位置 ==========
    ball.Update();
    
    // ========== 3. 球与挡板碰撞 ==========
    Rectangle paddleRect = paddle.GetRect();
    Vector2 ballPos = ball.GetPosition();
    float ballRadius = ball.GetRadius();
    
    if (CheckCollisionCircleRec(ballPos, ballRadius, paddleRect)) {
        // 改变球的方向
        Vector2 ballSpeed = ball.GetSpeed();
        ball.SetSpeed(ballSpeed.x, -ballSpeed.y);
        
        // 根据击中挡板的位置改变水平方向
        float hitPos = (ballPos.x - paddleRect.x) / paddleRect.width;
        float newSpeedX = (hitPos - 0.5f) * 8.0f;
        ball.SetSpeed(newSpeedX, -ballSpeed.y);
    }
    
    // ========== 4. 球与砖块碰撞 ==========
    for (int i = 0; i < bricks.size(); i++) {
    if (bricks[i].IsActive() && 
        CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), bricks[i].GetRect())) {
        
        bricks[i].Hit();
        Vector2 ballSpeed = ball.GetSpeed();
        ball.SetSpeed(ballSpeed.x, -ballSpeed.y);
        score += 10;
        
        // 如果砖块被摧毁，从vector中删除
        if (!bricks[i].IsActive()) {
            bricks.erase(bricks.begin() + i);
            i--;  // 调整索引，因为删除了一个元素
        }
        
        break;  // 一次只处理一个碰撞
    }
}
    
    // ========== 5. 检查球是否出界（掉到底部） ==========
    if (ballPos.y + ballRadius >= screenHeight) {
        lives--;
        if (lives <= 0) {
            gameRunning = false;
        } else {
            // 重置球的位置和速度
            ball.SetPosition(screenWidth / 2, screenHeight - 50);
            ball.SetSpeed(0, -4.0f);
            // 重置挡板位置
            paddle.SetPosition(screenWidth / 2 - 50, screenHeight - 30);
        }
    }
    
    // ========== 6. 保存快捷键 ==========
    if (IsKeyPressed(KEY_S)) {
        SaveGame();
    }
    
    // ========== 7. 检查关卡完成 ==========
    CheckLevelCompletion();
}

// 绘图函数
void Game::Draw() {
    // 绘制挡板
    paddle.Draw();
    
    // 绘制球
    ball.Draw();
    
    // 绘制所有砖块
    for (auto& brick : bricks) {
        brick.Draw();
    }
    
    // 绘制分数
    DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
    
    // 绘制生命值
    DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, WHITE);
    
    // 绘制当前关卡
    DrawText(TextFormat("Level: %d", currentLevel), 10, 70, 20, WHITE);
    
    // 如果游戏暂停，显示暂停提示
    if (gamePaused) {
        DrawText("PAUSED", screenWidth / 2 - 50, screenHeight / 2, 30, YELLOW);
    }
    
    // 如果游戏结束，显示结束信息
    if (!gameRunning && lives <= 0) {
        DrawText("GAME OVER", screenWidth / 2 - 80, screenHeight / 2, 40, RED);
        DrawText("Press R to restart", screenWidth / 2 - 100, screenHeight / 2 + 50, 20, WHITE);
    }
    
    // 如果通关，显示胜利信息
    if (!gameRunning && bricks.empty() && currentLevel >= 3) {
        DrawText("YOU WIN!", screenWidth / 2 - 60, screenHeight / 2, 40, GREEN);
    }
    
    // 操作提示
    DrawText("S:Save  L:Load  R:Reset", screenWidth - 180, screenHeight - 20, 15, GRAY);
}