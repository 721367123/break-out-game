#include "Ball.h"
#include <cstdlib>
#include <ctime>

Ball::Ball(Vector2 pos, float rad) {
    position = pos;
    radius = rad;
    active = true;
    RandomizeDirection();
}

void Ball::Move() {
    if (!active) return;
    position.x += speed.x;
    position.y += speed.y;
}

void Ball::Draw() {
    if (active) {
        DrawCircleV(position, radius, WHITE);
    }
}

void Ball::BounceX() {
    speed.x *= -1;
}

void Ball::BounceY() {
    speed.y *= -1;
}

Vector2 Ball::GetPosition() {
    return position;
}

float Ball::GetRadius() {
    return radius;
}

void Ball::SetPosition(Vector2 pos) {
    position = pos;
}

void Ball::SetSpeed(Vector2 spd) {
    speed = spd;
}

Vector2 Ball::GetSpeed() {
    return speed;
}

void Ball::RandomizeDirection() {
    static bool seeded = false;
    if (!seeded) {
        srand(time(NULL));
        seeded = true;
    }
    
    int directionX = (rand() % 2 == 0) ? -1 : 1;
    int directionY = (rand() % 2 == 0) ? -1 : 1;
    
    float speedX = (rand() % 40 + 25) / 10.0f;
    float speedY = (rand() % 30 + 25) / 10.0f;
    
    speed.x = directionX * speedX;
    speed.y = directionY * speedY;
    
    if (speed.x < 2.0f && speed.x > -2.0f) speed.x = (speed.x > 0) ? 2.0f : -2.0f;
    if (speed.y < 2.0f && speed.y > -2.0f) speed.y = (speed.y > 0) ? 2.0f : -2.0f;
}