#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>
#include <cstdlib>
#include <cmath>

int main() 
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "打砖块游戏 - 生命值+计时");
    SetTargetFPS(60);

    // 创建小球
    Ball ball({400, 500}, 8);
    
    // 创建蓝色长方形（paddle）
    Paddle paddle(350, 550, 100, 20);

    // 创建5行不同颜色的砖块
    std::vector<Brick> bricks;
    float brickWidth = 70;
    float brickHeight = 25;
    float startX = 45;
    float startY = 50;
    float spacing = 5;
    
    Color colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE};
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 8; col++) {
            bricks.emplace_back(
                startX + col * (brickWidth + spacing),
                startY + row * (brickHeight + spacing),
                brickWidth, 
                brickHeight,
                colors[row]
            );
        }
    }

    // 游戏变量
    int score = 0;
    int bricksRemaining = bricks.size();
    int lives = 3;
    float gameTime = 0.0f;
    bool gameRunning = true;
    bool gameWin = false;
    bool ballLaunched = false;
    bool gamePaused = false;

    while (!WindowShouldClose() && gameRunning) {
        // 计时器更新
        if (ballLaunched && !gamePaused && !gameWin) {
            gameTime += GetFrameTime();
        }
        
        // 控制小球发射
        if (!ballLaunched && !gamePaused) {
            ball.SetPosition({paddle.GetX() + paddle.GetWidth() / 2, 
                              paddle.GetY() - ball.GetRadius()});
            
            if (IsKeyPressed(KEY_SPACE)) {
                ballLaunched = true;
                ball.RandomizeDirection();
                Vector2 speed = ball.GetPosition();
                if (speed.y > 0) speed.y = -speed.y;
                ball.SetSpeed(speed);
            }
        } else if (!gamePaused) {
            ball.Move();
        }
        
        // 更新 paddle
        paddle.Update();
        
        // 碰撞检测
        if (ballLaunched && !gamePaused) {
            // 边界碰撞
            if (ball.GetPosition().x - ball.GetRadius() <= 0 || 
                ball.GetPosition().x + ball.GetRadius() >= screenWidth) {
                ball.BounceX();
            }
            
            if (ball.GetPosition().y - ball.GetRadius() <= 0) {
                ball.BounceY();
            }
            
            // 底部碰撞 - 失去一条命
            if (ball.GetPosition().y + ball.GetRadius() >= screenHeight) {
                lives--;
                
                if (lives > 0) {
                    ballLaunched = false;
                    ball.SetPosition({400, 500});
                    ball.RandomizeDirection();
                    
                    gamePaused = true;
                    int pauseFrames = 0;
                    while (pauseFrames < 60 && !WindowShouldClose()) {
                        BeginDrawing();
                        ClearBackground(BLACK);
                        
                        ball.Draw();
                        paddle.Draw();
                        for (auto& brick : bricks) brick.Draw();
                        
                        DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
                        DrawText(TextFormat("Bricks Left: %d", bricksRemaining), 10, 35, 20, WHITE);
                        DrawText(TextFormat("Lives: %d", lives), 10, 60, 20, RED);
                        DrawText(TextFormat("Time: %.1f s", gameTime), 10, 85, 20, WHITE);
                        
                        DrawText("LOST LIFE!", screenWidth/2 - 60, screenHeight/2, 30, RED);
                        DrawText("Press SPACE to continue", screenWidth/2 - 110, screenHeight/2 + 50, 20, YELLOW);
                        
                        EndDrawing();
                        pauseFrames++;
                        
                        if (IsKeyPressed(KEY_SPACE)) {
                            gamePaused = false;
                            break;
                        }
                    }
                    gamePaused = false;
                } else {
                    gameRunning = false;
                    gameWin = false;
                }
            }
              
    // 与 paddle 碰撞
if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), 
                             paddle.GetRect())) {
    ball.BounceY();  // 先反弹
    float hitPos = ball.GetPosition().x - paddle.GetX();
    float paddleWidth = paddle.GetWidth();
    float offset = (hitPos / paddleWidth) - 0.5f;
    float newSpeedX = offset * 4.0f;
    if (newSpeedX > 4.0f) newSpeedX = 4.0f;
    if (newSpeedX < -4.0f) newSpeedX = -4.0f;
    // 获取当前垂直速度并保持方向
    float currentSpeedY = ball.GetPosition().y;
    float newSpeedY = (currentSpeedY > 0) ? -3.0f : 3.0f;
    ball.SetSpeed({newSpeedX, newSpeedY});
}

// 与砖块碰撞
for (auto& brick : bricks) {
    if (brick.IsActive() && 
        CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), 
                               brick.GetRect())) {
        brick.Deactivate();
        ball.BounceY();
        score += 10;
        bricksRemaining--;
        break;
    }
}
// 与砖块碰撞
for (auto& brick : bricks) {
    if (brick.IsActive() && 
        CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), 
                               brick.GetRect())) {
        brick.Deactivate();
        ball.BounceY();
        score += 10;
        bricksRemaining--;
        break;
    }
}
    
    // 检查胜利
    if (bricksRemaining <= 0) {
        gameRunning = false;
        gameWin = true;
    }
}

// 控制暂停
if (IsKeyPressed(KEY_P) && ballLaunched && !gameWin) {
    gamePaused = !gamePaused;
}

// 绘制
BeginDrawing();
ClearBackground(BLACK);

ball.Draw();
paddle.Draw();

for (auto& brick : bricks) {
    brick.Draw();
}

// 显示游戏信息
DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
DrawText(TextFormat("Bricks Left: %d", bricksRemaining), 10, 35, 20, WHITE);
DrawText(TextFormat("Lives: %d", lives), 10, 60, 20, RED);

int minutes = (int)gameTime / 60;
int seconds = (int)gameTime % 60;
DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), 10, 85, 20, WHITE);

// 显示提示
if (!ballLaunched && lives > 0) {
    DrawText("Press SPACE to launch ball!", screenWidth/2 - 130, screenHeight/2, 20, YELLOW);
    DrawText("Move mouse to control paddle", screenWidth/2 - 110, screenHeight/2 + 40, 16, WHITE);
    DrawText("Press P to pause", screenWidth/2 - 50, screenHeight/2 + 70, 16, WHITE);
}

if (gamePaused && ballLaunched && !gameWin) {
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
    DrawText("PAUSED", screenWidth/2 - 60, screenHeight/2 - 20, 40, YELLOW);
    DrawText("Press P to resume", screenWidth/2 - 80, screenHeight/2 + 30, 20, WHITE);
}
 // 游戏结束画面
        if (!gameRunning) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
            
            if (gameWin) {
                DrawText("YOU WIN!", screenWidth/2 - MeasureText("YOU WIN!", 40)/2, screenHeight/2 - 60, 40, GREEN);
                DrawText(TextFormat("Final Score: %d", score), screenWidth/2 - MeasureText(TextFormat("Final Score: %d", score), 25)/2, screenHeight/2 - 10, 25, WHITE);
                DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), screenWidth/2 - MeasureText(TextFormat("Time: %02d:%02d", minutes, seconds), 20)/2, screenHeight/2 + 20, 20, WHITE);
                DrawText(TextFormat("Lives Left: %d", lives), screenWidth/2 - MeasureText(TextFormat("Lives Left: %d", lives), 20)/2, screenHeight/2 + 50, 20, GREEN);
            } else {
                DrawText("GAME OVER!", screenWidth/2 - MeasureText("GAME OVER!", 40)/2, screenHeight/2 - 60, 40, RED);
                DrawText(TextFormat("Final Score: %d", score), screenWidth/2 - MeasureText(TextFormat("Final Score: %d", score), 25)/2, screenHeight/2 - 10, 25, WHITE);
                DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), screenWidth/2 - MeasureText(TextFormat("Time: %02d:%02d", minutes, seconds), 20)/2, screenHeight/2 + 20, 20, WHITE);
            }
            DrawText("Press R to Restart", screenWidth/2 - MeasureText("Press R to Restart", 20)/2, screenHeight/2 + 100, 20, WHITE);
            
            if (IsKeyPressed(KEY_R)) {
                // 重置游戏
                ball.SetPosition({400, 500});
                ball.RandomizeDirection();
                ballLaunched = false;
                gamePaused = false;
                
                bricks.clear();
                bricksRemaining = 0;
                for (int row = 0; row < 5; row++) {
                    for (int col = 0; col < 8; col++) {
                        bricks.emplace_back(
                            startX + col * (brickWidth + spacing),
                            startY + row * (brickHeight + spacing),
                            brickWidth, 
                            brickHeight,
                            colors[row]
                        );
                        bricksRemaining++;
                    }
                }
                
                score = 0;
                lives = 3;
                gameTime = 0.0f;
                gameRunning = true;
                gameWin = false;
            }
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}