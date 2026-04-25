#include "enet/enet.h"
#include "NetworkProtocol.h"
#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

double GetTime() {
    static auto start = chrono::steady_clock::now();
    auto now = chrono::steady_clock::now();
    return chrono::duration<double>(now - start).count();
}

class Ball {
public:
    float x, y, vx, vy;
    float radius;
    
    Ball() : x(400), y(300), vx(0), vy(0), radius(8.0f) {}
    
    void Update() {
        x += vx;
        y += vy;
        
        // 限制最大速度
        if (fabs(vx) > 8.0f) vx = vx > 0 ? 8.0f : -8.0f;
        if (fabs(vy) > 8.0f) vy = vy > 0 ? 8.0f : -8.0f;
        
        if (x - radius < 0) { x = radius; vx = -vx; }
        if (x + radius > 800) { x = 800 - radius; vx = -vx; }
        if (y - radius < 0) { y = radius; vy = -vy; }
    }
    
    void Reset() {
        x = 400;
        y = 300;
        vx = 0;
        vy = 0;
    }
    
    void Start() {
        vx = 4.0f * (rand() % 2 == 0 ? 1 : -1);
        vy = -4.0f;
    }
    
    bool IsOutOfBounds() const { return y + radius > 600; }
    float GetX() { return x; }
    float GetY() { return y; }
    float GetVx() { return vx; }
    float GetVy() { return vy; }
};

class Brick {
public:
    float x, y;
    float width, height;
    bool active;
    int colorId;
    
    Brick() : x(0), y(0), width(55), height(22), active(true), colorId(0) {}
    Brick(float posX, float posY, int cid) : x(posX), y(posY), width(55), height(22), active(true), colorId(cid) {}
    void Destroy() { active = false; }
    bool IsActive() const { return active; }
};

struct ClientInfo {
    ENetPeer* peer;
    int playerId;
    float paddleX;
    float paddleY;
    
    ClientInfo() : peer(nullptr), playerId(-1), paddleX(350), paddleY(550) {}
    ClientInfo(ENetPeer* p, int id) : peer(p), playerId(id), paddleX(350), paddleY(550) {}
};

class GameServer {
private:
    ENetHost* server;
    map<int, ClientInfo> clients;
    int nextPlayerId;
    bool gameRunning;
    bool gameStarted;
    Ball ball;
    vector<Brick> bricks;
    int totalScore;
    int totalLives;
    bool justHitPaddle;
    
    void InitGameState() {
        ball = Ball();
        bricks.clear();
        int brickWidth = 55, brickHeight = 22, cols = 12, rows = 7;
        int startX = (800 - cols * brickWidth) / 2;
        int startY = 60;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                bricks.push_back(Brick(startX + col * brickWidth, startY + row * brickHeight, row % 7));
            }
        }
        totalScore = 0;
        totalLives = 3;
        gameStarted = false;
        justHitPaddle = false;
    }
    
    void SendPacket(ENetPeer* peer, NetworkPacket& packet) {
        ENetPacket* enetPacket = enet_packet_create(&packet, sizeof(NetworkPacket), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, enetPacket);
    }
    void BroadcastGameState() {
    if (clients.empty()) return;
    NetworkPacket packet;
    packet.type = PKT_GAME_STATE;
    packet.ballX = ball.GetX();
    packet.ballY = ball.GetY();
    packet.ballVx = ball.GetVx();
    packet.ballVy = ball.GetVy();
    packet.score = totalScore;
    packet.lives = totalLives;
    packet.gameStarted = gameStarted ? 1 : 0;
    for (int i = 0; i < (int)bricks.size() && i < 128; i++) {
        packet.data[i] = bricks[i].IsActive() ? 1 : 0;
        packet.brickColors[i] = bricks[i].colorId;
    }
    for (auto& pair : clients) {
        SendPacket(pair.second.peer, packet);
    }
}

void BroadcastPaddleUpdate(int playerId, float paddleX) {
    NetworkPacket packet;
    packet.type = PKT_PADDLE_UPDATE;
    packet.playerId = playerId;
    packet.paddleX = paddleX;
    for (auto& pair : clients) {
        if (pair.first != playerId) {
            SendPacket(pair.second.peer, packet);
        }
    }
}

void HandleJoinRequest(ENetPeer* peer) {
    int playerId = nextPlayerId++;
    clients[playerId] = ClientInfo(peer, playerId);
    NetworkPacket packet;
    packet.type = PKT_JOIN_ACCEPT;
    packet.playerId = playerId;
    SendPacket(peer, packet);
    cout << "玩家 " << playerId << " 加入游戏！ 当前玩家数: " << clients.size() << endl;
    BroadcastGameState();
}

void HandlePaddleUpdate(NetworkPacket* packet, ENetPeer* peer) {
    for (auto& pair : clients) {
        if (pair.second.peer == peer) {
            pair.second.paddleX = packet->paddleX;
            BroadcastPaddleUpdate(pair.first, packet->paddleX);
            break;
        }
    }
}void CheckCollisions() {
        if (!gameStarted) return;
        
        // 球与挡板碰撞
        for (auto& pair : clients) {
            ClientInfo& client = pair.second;
            if (ball.y + ball.radius >= client.paddleY && ball.y - ball.radius <= client.paddleY + 20) {
                if (ball.x + ball.radius >= client.paddleX && 
                    ball.x - ball.radius <= client.paddleX + 100) {
                    
                    if (justHitPaddle) break;
                    justHitPaddle = true;
                    
                    // 确保球向上反弹
                    ball.vy = -abs(ball.vy);
                    // 根据击中位置改变水平速度
                    float hitPos = (ball.x - client.paddleX) / 100.0f;
                    ball.vx = (hitPos - 0.5f) * 10.0f;
                    if (fabs(ball.vx) < 2.0f) ball.vx = ball.vx > 0 ? 2.0f : -2.0f;
                    // 把球放到挡板上面
                    ball.y = client.paddleY - ball.radius - 1;
                    break;
                }
            }
        }
        
        // 重置冷却
        if (ball.y + ball.radius < 540) {
            justHitPaddle = false;
        }
        
        // 球与砖块碰撞
        for (int i = 0; i < (int)bricks.size(); i++) {
            if (!bricks[i].IsActive()) continue;
            Brick& brick = bricks[i];
            if (ball.x + ball.radius >= brick.x && 
                ball.x - ball.radius <= brick.x + brick.width &&
                ball.y + ball.radius >= brick.y && 
                ball.y - ball.radius <= brick.y + brick.height) {
                
                bricks[i].Destroy();
                totalScore += 10;
                
                float overlapLeft = (ball.x + ball.radius) - brick.x;
                float overlapRight = (brick.x + brick.width) - (ball.x - ball.radius);
                float overlapTop = (ball.y + ball.radius) - brick.y;
                float overlapBottom = (brick.y + brick.height) - (ball.y - ball.radius);
                
                if (overlapLeft < overlapRight && overlapLeft < overlapTop && overlapLeft < overlapBottom) {
                    ball.vx = -abs(ball.vx);
                } else if (overlapRight < overlapLeft && overlapRight < overlapTop && overlapRight < overlapBottom) {
                    ball.vx = abs(ball.vx);
                } else if (overlapTop < overlapLeft && overlapTop < overlapRight && overlapTop < overlapBottom) {
                    ball.vy = -abs(ball.vy);
                } else {
                    ball.vy = abs(ball.vy);
                }
                break;
            }
        }
        
        // 球超出底部
        if (ball.IsOutOfBounds()) {
            totalLives--;
            cout << "生命剩余: " << totalLives << endl;
            if (totalLives <= 0) {
                gameStarted = false;
                cout << "游戏结束！" << endl;
            }
            ball.Reset();
            BroadcastGameState();
        }
        
        bool allBricksDestroyed = true;
        for (auto& brick : bricks) {
            if (brick.IsActive()) { allBricksDestroyed = false; break; }
        }
        if (allBricksDestroyed) {
            gameStarted = false;
            cout << "胜利！所有砖块被摧毁！" << endl;
        }
    }

public:
    GameServer() : server(nullptr), nextPlayerId(0), gameRunning(false), gameStarted(false), totalScore(0), totalLives(3), justHitPaddle(false) {}
    ~GameServer() { if (server) enet_host_destroy(server); }

bool Start(int port) {
        srand((unsigned)time(nullptr));
        if (enet_initialize() != 0) {
            cerr << "ENet 初始化失败！" << endl;
            return false;
        }
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;
        server = enet_host_create(&address, 32, 2, 0, 0);
        if (!server) {
            cerr << "无法创建服务器！" << endl;
            return false;
        }
        InitGameState();
        gameRunning = true;
        cout << "服务器启动成功，端口: " << port << endl;
        cout << "等待玩家连接... 按空格开始游戏" << endl;
        return true;
    }
    
    void Run() {
        ENetEvent event;
        float lastUpdateTime = 0;
        float updateInterval = 1.0f / 60.0f;
        bool lastSpaceState = false;
        
        while (gameRunning) {
            bool currentSpaceState = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
            if (currentSpaceState && !lastSpaceState && !gameStarted && clients.size() >= 1) {
                gameStarted = true;
                ball.Start();
                cout << "游戏开始！" << endl;
                BroadcastGameState();
            }
            lastSpaceState = currentSpaceState;
            
            while (enet_host_service(server, &event, 0) > 0) {
                switch (event.type) {
                    case ENET_EVENT_TYPE_CONNECT:
                        cout << "新客户端连接" << endl;
                        break;
                    case ENET_EVENT_TYPE_RECEIVE: {
                        NetworkPacket* packet = (NetworkPacket*)event.packet->data;
                        switch (packet->type) {
                            case PKT_JOIN_REQUEST:
                                HandleJoinRequest(event.peer);
                                break;
                            case PKT_PADDLE_UPDATE:
                                HandlePaddleUpdate(packet, event.peer);
                                break;
                        }
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT:
                        cout << "客户端断开连接" << endl;
                        for (auto it = clients.begin(); it != clients.end(); ++it) {
                            if (it->second.peer == event.peer) {
                                clients.erase(it);
                                break;
                            }
                        }
                        break;
                    case ENET_EVENT_TYPE_NONE:
                        break;
                }
            }
            
            float currentTime = (float)GetTime();
            if (currentTime - lastUpdateTime >= updateInterval) {
                lastUpdateTime = currentTime;
                if (gameStarted) {
                    ball.Update();
                    CheckCollisions();
                }
                BroadcastGameState();
            }
        }
    }
    
    void Stop() {
        gameRunning = false;
        if (server) {
            enet_host_destroy(server);
            server = nullptr;
        }
        enet_deinitialize();
    }
};

int main() {
    GameServer server;
    if (!server.Start(1234)) {
        cerr << "服务器启动失败！" << endl;
        return -1;
    }
    server.Run();
    server.Stop();
    return 0;
}