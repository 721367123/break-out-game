#pragma once
#include <vector>
#include <string>
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER
};

class Game {
public:
    Game();
    ~Game();
    void Init();
    void Update();
    void Draw();
    bool ShouldClose();
    void Shutdown();
    
private:
    // 窗口
    int screenWidth;
    int screenHeight;
    std::string windowTitle;
    
    // 游戏对象
    Ball* ball;
    Paddle* paddle;
    std::vector<Brick*> bricks;
    
    // 配置参数
    float ballRadius;
    float ballSpeedX;
    float ballSpeedY;
    float paddleWidth;
    float paddleHeight;
    float paddleSpeed;
    int brickRows;
    int brickCols;
    float brickWidth;
    float brickHeight;
    float brickStartX;
    float brickStartY;
    float brickSpacing;
    int initialLives;
    int scorePerBrick;
    
    // 游戏状态
    int score;
    int lives;
    int bricksRemaining;
    GameState currentState;
    
    // 颜色
    Color brickColors[5];
    
    // 函数
    void LoadConfig(const std::string& path);
    void CreateBricks();
    void CheckCollisions();
    void ResetGame();
    
    // 状态机
    void UpdateMenu();
    void UpdatePlaying();
    void UpdatePaused();
    void UpdateGameOver();
    
    void DrawMenu();
    void DrawPlaying();
    void DrawPaused();
    void DrawGameOver();
};