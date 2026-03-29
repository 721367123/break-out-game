#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"

class Paddle {
private:
    float x, y;
    float width, height;
    Vector2 mousePos;

public:
    Paddle(float startX, float startY, float w, float h);
    void Update();
    void Draw();
    Rectangle GetRect();
    float GetX();
    float GetY();
    float GetWidth();
    float GetHeight();
};

#endif