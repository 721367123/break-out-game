#include "Brick.h"

Brick::Brick(float x, float y, float w, float h, Color c) {
    rect = {x, y, w, h};
    active = true;
    color = c;
}

void Brick::Draw() {
    if (active) {
        DrawRectangleRec(rect, color);
    }
}

bool Brick::IsActive() {
    return active;
}

void Brick::Deactivate() {
    active = false;
}

Rectangle Brick::GetRect() {
    return rect;
}

float Brick::GetX() { return rect.x; }
float Brick::GetY() { return rect.y; }
float Brick::GetWidth() { return rect.width; }
float Brick::GetHeight() { return rect.height; }
Color Brick::GetColor() { return color; }