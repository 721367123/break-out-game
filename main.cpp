#include "Game.h"

int main() {
    Game game;
    
    // 游戏主循环 - 窗口不关闭且游戏在运行时就一直循环
    while (!WindowShouldClose()) {
        game.Update();
        
        BeginDrawing();
        ClearBackground(BLACK);
        game.Draw();
        EndDrawing();
    }
    
    return 0;
}