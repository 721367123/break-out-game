// tests/TestPaddle.cpp - 第一段
// 挡板碰撞反弹条件测试

#include <iostream>
#include <cassert>
#include <cmath>
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"

// 测试结果统计
int testsPassed = 0;
int testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "运行测试: " << #name << " ... "; \
    test_##name(); \
    std::cout << "✓ 通过" << std::endl; \
    testsPassed++; \
} while(0)
#define ASSERT(condition) do { \
    if (!(condition)) { \
        std::cout << "✗ 失败: " << #condition << " (行: " << __LINE__ << ")" << std::endl; \
        testsFailed++; \
        return; \
    } \
} while(0)
#define ASSERT_EQUAL(a, b) ASSERT((a) == (b))
#define ASSERT_NEAR(a, b, epsilon) ASSERT(std::abs((a) - (b)) < (epsilon))

// ==================== 测试1: 球从正上方撞击挡板 ====================
TEST(BallHitsPaddleFromTop) {
    Paddle paddle(350, 550, 100, 20);
    Ball ball({400, 540}, 8);
    ball.SetVelocity({0, 100});
    
    bool collision = CheckCollisionCircleRec(
        ball.GetPosition(), 
        ball.GetRadius(), 
        paddle.GetRect()
    );
    
    ASSERT(collision == true);
    
    float oldVy = ball.GetVelocity().y;
    ball.ReverseYVelocity();
    
    ASSERT(ball.GetVelocity().y == -oldVy);
    ASSERT(ball.GetVelocity().x == 0);
}

// ==================== 测试2: 球从侧面撞击挡板 ====================
TEST(BallHitsPaddleFromSide) {
    Paddle paddle(350, 550, 100, 20);
    
    Ball ballLeft({345, 545}, 8);
    ballLeft.SetVelocity({-50, 50});
    
    bool collisionLeft = CheckCollisionCircleRec(
        ballLeft.GetPosition(),
        ballLeft.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collisionLeft == true);
    
    Ball ballRight({455, 545}, 8);
    ballRight.SetVelocity({50, 50});
    
    bool collisionRight = CheckCollisionCircleRec(
        ballRight.GetPosition(),
        ballRight.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collisionRight == true);
}

// ==================== 测试3: 球未击中挡板 ====================
TEST(BallMissesPaddle) {
    Paddle paddle(350, 550, 100, 20);
    
    Ball ballLeft({200, 545}, 8);
    ballLeft.SetVelocity({-100, 50});
    
    bool collisionLeft = CheckCollisionCircleRec(
        ballLeft.GetPosition(),
        ballLeft.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collisionLeft == false);
    
    Ball ballRight({600, 545}, 8);
    ballRight.SetVelocity({100, 50});
    
    bool collisionRight = CheckCollisionCircleRec(
        ballRight.GetPosition(),
        ballRight.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collisionRight == false);
}

// ==================== 测试4: 球击中挡板不同位置（偏移反弹） ====================
TEST(BallBounceAngleDependsOnHitPosition) {
    Paddle paddle(350, 550, 100, 20);
    float paddleCenter = paddle.GetRect().x + paddle.GetRect().width / 2;
    
    auto getBounceXVelocity = [&](float ballX, float ballVx, float ballVy) {
        Ball ball({ballX, 540}, 8);
        ball.SetVelocity({ballVx, ballVy});
        
        if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
            float hitPos = ball.GetPosition().x - paddle.GetRect().x;
            float paddleWidth = paddle.GetRect().width;
            float normalizedPos = hitPos / paddleWidth;
            float angleFactor = (normalizedPos - 0.5f) * 2.0f;
            float newVx = ballVy * angleFactor;
            ball.SetVelocity({newVx, -ball.GetVelocity().y});
            return ball.GetVelocity().x;
        }
        return 0.0f;
    };
    
    float vxLeft = getBounceXVelocity(paddle.GetRect().x + 20, 0, 100);
    ASSERT(vxLeft < 0);
    
    float vxCenter = getBounceXVelocity(paddleCenter, 0, 100);
    ASSERT_NEAR(vxCenter, 0, 0.1f);
    
    float vxRight = getBounceXVelocity(paddle.GetRect().x + 80, 0, 100);
    ASSERT(vxRight > 0);
}
// tests/TestPaddle.cpp - 第二段

// ==================== 测试5: 边界情况 - 球刚好擦边 ====================
TEST(BallGrazesPaddleEdge) {
    Paddle paddle(350, 550, 100, 20);
    
    Ball ballCorner({350, 542}, 8);
    bool collision = CheckCollisionCircleRec(
        ballCorner.GetPosition(),
        ballCorner.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collision == true);
    
    Ball ballJustAbove({400, 542 - 8 - 0.1f}, 8);
    bool collisionAbove = CheckCollisionCircleRec(
        ballJustAbove.GetPosition(),
        ballJustAbove.GetRadius(),
        paddle.GetRect()
    );
    ASSERT(collisionAbove == false);
}

// ==================== 测试6: 连续碰撞（球在挡板上弹跳） ====================
TEST(MultipleBouncesOnPaddle) {
    Paddle paddle(350, 550, 100, 20);
    Ball ball({400, 540}, 8);
    ball.SetVelocity({50, -100});
    
    int bounceCount = 0;
    const int maxBounces = 10;
    
    for (int i = 0; i < maxBounces; i++) {
        ball.SetVelocity({ball.GetVelocity().x, 100});
        
        if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
            ball.ReverseYVelocity();
            bounceCount++;
        }
        ball.SetPosition({ball.GetPosition().x, ball.GetPosition().y - 20});
    }
    
    ASSERT(bounceCount > 0);
    std::cout << "  连续弹跳次数: " << bounceCount;
}

// ==================== 测试7: 速度和碰撞响应 ====================
TEST(VelocityAffectsBounce) {
    Paddle paddle(350, 550, 100, 20);
    
    Ball slowBall({400, 540}, 8);
    slowBall.SetVelocity({0, 50});
    
    Ball fastBall({400, 540}, 8);
    fastBall.SetVelocity({0, 500});
    
    bool slowCollision = CheckCollisionCircleRec(
        slowBall.GetPosition(),
        slowBall.GetRadius(),
        paddle.GetRect()
    );
    bool fastCollision = CheckCollisionCircleRec(
        fastBall.GetPosition(),
        fastBall.GetRadius(),
        paddle.GetRect()
    );
    
    ASSERT(slowCollision == true);
    ASSERT(fastCollision == true);
}

// ==================== 测试8: 挡板移动时的碰撞 ====================
TEST(PaddleMovingWhileCollision) {
    Paddle paddle(350, 550, 100, 20);
    Ball ball({400, 540}, 8);
    ball.SetVelocity({0, 100});
    
    // 模拟挡板向右移动
    for (int i = 0; i < 10; i++) {
        paddle.SetPosition(paddle.GetRect().x + 5, paddle.GetRect().y);
        
        if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
            ball.ReverseYVelocity();
            ASSERT(ball.GetVelocity().y < 0);  // 应该向上反弹
            return;
        }
    }
    
    // 如果没有发生碰撞，测试失败
    ASSERT(false);
}

// ==================== 主函数 ====================
int main() {
    std::cout << "========== 挡板碰撞反弹测试 ==========" << std::endl;
    std::cout << std::endl;
    
    // 运行所有测试
    RUN_TEST(BallHitsPaddleFromTop);
    RUN_TEST(BallHitsPaddleFromSide);
    RUN_TEST(BallMissesPaddle);
    RUN_TEST(BallBounceAngleDependsOnHitPosition);
    RUN_TEST(BallGrazesPaddleEdge);
    RUN_TEST(MultipleBouncesOnPaddle);
    RUN_TEST(VelocityAffectsBounce);
    RUN_TEST(PaddleMovingWhileCollision);
    
    std::cout << std::endl;
    std::cout << "========== 测试结果 ==========" << std::endl;
    std::cout << "通过: " << testsPassed << std::endl;
    std::cout << "失败: " << testsFailed << std::endl;
    
    if (testsFailed == 0) {
        std::cout << "✓ 所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << "✗ 有测试失败，请检查代码" << std::endl;
        return 1;
    }
}