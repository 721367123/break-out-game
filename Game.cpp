#include "Game.h"
#include <iostream>

Game::Game() 
    : screenWidth(800), screenHeight(600)
    , ball(nullptr), paddle(nullptr)
    , score(0), lives(3), bricksRemaining(0)
    , currentState(GameState::MENU) {
    
    brickColors[0] = RED;
    brickColors[1] = ORANGE;
    brickColors[2] = YELLOW;
    brickColors[3] = GREEN;
    brickColors[4] = BLUE;
    
    ballRadius = 8;
    ballSpeedX = 250;
    ballSpeedY = -250;
    
    paddleWidth = 100;
    paddleHeight = 20;
    
    brickRows = 5;
    brickCols = 8;
    brickWidth = 70;
    brickHeight = 25;
    brickStartX = 45;
    brickStartY = 50;
    brickSpacing = 5;
    
    initialLives = 3;
    scorePerBrick = 10;
}

Game::~Game() {
    delete ball;
    delete paddle;
    for (auto b : bricks) delete b;
}

void Game::Init() {
    InitWindow(screenWidth, screenHeight, "打砖块游戏");
    SetTargetFPS(60);
    
    ball = new Ball(screenWidth/2, screenHeight - 100, ballRadius);
    ball->SetSpeed(ballSpeedX, ballSpeedY);
    
    paddle = new Paddle((screenWidth - paddleWidth)/2, screenHeight - 50, paddleWidth, paddleHeight);
    
    CreateBricks();
}

void Game::CreateBricks() {
    for (int row = 0; row < brickRows; row++) {
        for (int col = 0; col < brickCols; col++) {
            float x = brickStartX + col * (brickWidth + brickSpacing);
            float y = brickStartY + row * (brickHeight + brickSpacing);
            bricks.push_back(new Brick(x, y, brickWidth, brickHeight, brickColors[row % 5]));
        }
    }
    bricksRemaining = bricks.size();
}

void Game::ResetGame() {
    score = 0;
    lives = initialLives;
    bricksRemaining = bricks.size();
    
    ball->SetPos(screenWidth/2, screenHeight - 100);
    ball->SetSpeed(ballSpeedX, ballSpeedY);
    
    paddle->SetX((screenWidth - paddleWidth)/2);
    
    for (auto b : bricks) delete b;
    bricks.clear();
    CreateBricks();
}
void Game::Update() {
    switch (currentState) {
        case GameState::MENU:
            if (IsKeyPressed(KEY_SPACE)) {
                currentState = GameState::PLAYING;
                ResetGame();
            }
            break;
        case GameState::PLAYING:
            if (IsKeyPressed(KEY_P)) {
                currentState = GameState::PAUSED;
            }
            ball->Update();
            paddle->Update();
            CheckCollisions();
            if (ball->GetPos().y + ball->GetR() >= screenHeight) {
                lives--;
                if (lives <= 0) {
                    currentState = GameState::GAMEOVER;
                } else {
                    ball->SetPos(screenWidth/2, screenHeight - 100);
                }
            }
            if (bricksRemaining <= 0) {
                currentState = GameState::GAMEOVER;
            }
            break;
        case GameState::PAUSED:
            if (IsKeyPressed(KEY_P)) {
                currentState = GameState::PLAYING;
            }
            break;
        case GameState::GAMEOVER:
            if (IsKeyPressed(KEY_R)) {
                currentState = GameState::MENU;
                ResetGame();
            }
            break;
    }
}

void Game::CheckCollisions() {
    Vector2 ballPos = ball->GetPos();
    float ballR = ball->GetR();
    
    if (ballPos.x - ballR <= 0 || ballPos.x + ballR >= screenWidth) {
        ball->ReverseX();
    }
    if (ballPos.y - ballR <= 0) {
        ball->ReverseY();
    }
    if (CheckCollisionCircleRec(ballPos, ballR, paddle->GetRect())) {
        ball->ReverseY();
    }
    
    for (auto it = bricks.begin(); it != bricks.end();) {
        if ((*it)->IsAlive() && CheckCollisionCircleRec(ballPos, ballR, (*it)->GetRect())) {
            ball->ReverseY();
            (*it)->Hit();
            score += scorePerBrick;
            bricksRemaining--;
            delete *it;
            it = bricks.erase(it);
        } else {
            ++it;
        }
    }
}
void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    
    switch (currentState) {
        case GameState::MENU:
            DrawText("BRICK BREAKER", screenWidth/2 - 150, screenHeight/2 - 100, 50, YELLOW);
            DrawText("Press SPACE to start", screenWidth/2 - 130, screenHeight/2, 25, WHITE);
            break;
        case GameState::PLAYING:
            ball->Draw();
            paddle->Draw();
            for (auto b : bricks) {
                b->Draw();
            }
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, WHITE);
            break;
        case GameState::PAUSED:
            ball->Draw();
            paddle->Draw();
            for (auto b : bricks) {
                b->Draw();
            }
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
            DrawText("PAUSED", screenWidth/2 - 60, screenHeight/2, 40, YELLOW);
            DrawText("Press P to resume", screenWidth/2 - 100, screenHeight/2 + 50, 20, WHITE);
            break;
        case GameState::GAMEOVER:
            ball->Draw();
            paddle->Draw();
            for (auto b : bricks) {
                b->Draw();
            }
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
            if (bricksRemaining <= 0) {
                DrawText("YOU WIN!", screenWidth/2 - 70, screenHeight/2 - 50, 50, GREEN);
            } else {
                DrawText("GAME OVER", screenWidth/2 - 100, screenHeight/2 - 50, 50, RED);
            }
            DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 20, 20, WHITE);
            break;
    }
    
    EndDrawing();
}

bool Game::ShouldClose() {
    return WindowShouldClose();
}

void Game::Shutdown() {
    CloseWindow();
}