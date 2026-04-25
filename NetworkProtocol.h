#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <cstdint>

enum PacketType {
    PKT_JOIN_REQUEST = 1,
    PKT_JOIN_ACCEPT,
    PKT_JOIN_DENY,
    PKT_PADDLE_UPDATE,
    PKT_GAME_STATE,
};

#pragma pack(push, 1)
struct NetworkPacket {
    uint32_t type;
    uint32_t playerId;
    float paddleX;
    float ballX, ballY;
    float ballVx, ballVy;
    int score;
    int lives;
    int gameStarted;
    char data[128];      // 砖块状态
    int brickColors[128]; // 砖块颜色
};
#pragma pack(pop)

#endif