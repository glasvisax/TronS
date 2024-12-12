#pragma once

#include <enet/enet.h>
#include <string>
#include <functional>
#include <glm.hpp>

#ifndef GAME_PREF 
    #define GAME_PREF
    #include "../misc/game_preferences.h"
#endif 

#include "../objects/snake.h"
#include "../misc/game_types.h"

struct pos
{
    uint8_t x;
    uint8_t z;
};

struct GameStateMsg
{
    uint8_t type = uint8_t(0);
    uint8_t snake1_body_sz = 3;
    uint8_t snake2_body_sz = 3;
    Direction snake1_dir;
    Direction snake2_dir;
    pos snake1_body[maxSnakeSize], snake2_body[maxSnakeSize], apple_pos;
};

struct StartGameMsg
{
    uint8_t type = uint8_t(1);
    uint8_t grid_size_x;
    uint8_t grid_size_z;
    pos snake1_body[3], snake2_body[3], apple_pos;
};

struct StopGameMsg
{
    uint8_t type = uint8_t(2);
    GameResult result;
};

struct SnakeDirChangeMsg 
{
    uint8_t type = uint8_t(3);
    Direction direction;
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool InitializeServer(int& port);
    bool InitializeClient(const char* address, int port = 1234);
    void Update();
    void Shutdown();
    void Disconnect();

    void sendStartGame(StartGameMsg* msg);
    void sendGameState(GameStateMsg* msg);
    void sendStopGame(StopGameMsg* msg);
    void sendSnakeDirChange(SnakeDirChangeMsg* msg);

    bool IsServer() const { return isServer; }
    bool IsConnected() const { return peer != nullptr && peer->state == ENET_PEER_STATE_CONNECTED; }
   
    std::function<void(bool)> onConnectionChange = nullptr;
    std::function<void(StartGameMsg*)> onStartGameReceive = nullptr;
    std::function<void(GameStateMsg*)> onGameStateReceive = nullptr;
    std::function<void(StopGameMsg*)> onStopGameReceive = nullptr;
    std::function<void(SnakeDirChangeMsg*)> onSnakeDirChangeReceive = nullptr;

private:
    ENetHost* host;
    ENetPeer* peer;
    bool isServer;
};
