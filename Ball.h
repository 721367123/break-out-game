#ifndef BALL_H
#define BALL_H

#include "raylib.h"

class Ball {
private:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;

public:
    Ball(Vector2 pos, float rad);
    void Move();
    void Draw();
    void BounceX();
    void BounceY();
    Vector2 GetPosition();
    float GetRadius();
    void SetPosition(Vector2 pos);
    void SetSpeed(Vector2 spd);
    Vector2 GetSpeed();
    bool IsActive();
    void SetActive(bool act);
    void RandomizeDirection();
};

#endif