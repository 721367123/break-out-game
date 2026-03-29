#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

class Brick {
private:
    Rectangle rect;
    bool active;
    Color color;

public:
    Brick(float x, float y, float w, float h, Color c);
    void Draw();
    bool IsActive();
    void Deactivate();
    Rectangle GetRect();
    float GetX();
    float GetY();
    float GetWidth();
    float GetHeight();
    Color GetColor();
};

#endif