#include "Paddle.h"

Paddle::Paddle(float startX, float startY, float w, float h) {
    x = startX;
    y = startY;
    width = w;
    height = h;
    mousePos = {0, 0};
}

void Paddle::Update() {
    mousePos = GetMousePosition();
    x = mousePos.x - width / 2;
    
    if (x < 0) x = 0;
    if (x + width > 800) x = 800 - width;
}

void Paddle::Draw() {
    DrawRectangle(x, y, width, height, BLUE);
}

Rectangle Paddle::GetRect() {
    return {x, y, width, height};
}

float Paddle::GetX() { return x; }
float Paddle::GetY() { return y; }
float Paddle::GetWidth() { return width; }
float Paddle::GetHeight() { return height; }