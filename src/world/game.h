#pragma once

#include <glm.hpp>
#include <random>

#include "../objects/snake.h"
#include "../network/network_manager.h"
#include "camera.h"
#include "../misc/game_types.h"

extern bool lastRender;

class Game {
public:
    Game(int gridSizeX = 10, int gridSizeZ = 10);
    ~Game() = default;

    void Update(float deltaTime);
    void ProcessInput(int key);
    void Reset();
    void ServerGameStart();
    void GameStart(glm::vec2* snake1_body, glm::vec2* snake2_body, glm::vec2 apple_pos);

    const Snake& GetSnake() const { return snake1; }
    const Snake& GetSnake2() const { return snake2; }
    const glm::vec2& GetApplePosition() const { return applePosition; }
    const glm::vec2& GetGridSize() const { return gridSize; }
    const Camera& GetCamera() const { return camera; }
    bool IsGameOver() const { return gameOver; }
    void SetGridSize(int gridSizeX, int gridSizeZ)
    {
        camera = Camera(50.0f, glm::vec3((gridSizeX - 1) / 2, 25, gridSizeX + 7), glm::vec3((gridSizeX - 1) / 2, 0.0f, (gridSizeZ - 1) / 2));
        gridSize = glm::vec2(gridSizeX, gridSizeZ);
        xDist = std::uniform_real_distribution<float>(0, gridSizeX - 1);
        zDist = std::uniform_real_distribution<float>(0, gridSizeZ - 1);
    }
    void shutDownConnection()
    {
        networkManager.Shutdown();
    }
    void initializeClient(int port, const char* address);
    void initializeServer(int& port);

    GameState getState() { return state; }

    bool isServer() { return networkManager.IsServer(); }

    void (*onConnected)();
    void (*onDisconnected)();
    void (*onClientReceivedStart)();
    void (*onGameOver)(GameResult result);

private:
    void sendGameStateMsg();
    void spawnApple();
    glm::vec2 getAccessibleApplePos();
    bool IsValidApplePosition(const glm::vec2& pos) const;
    bool CheckSnakesCollision(GameResult& res) const;

    void onConnectionChanged(bool Connected);
    void onStartGameReceived(StartGameMsg* msg);
    void onGameStateReceived(GameStateMsg* msg);
    void onStopGameReceived(StopGameMsg* msg);
    void onSnakeDirChangeReceived(SnakeDirChangeMsg* msg);

    //void processNetwork();

    Snake snake1; 
    Snake snake2; 
    Camera camera;
    glm::vec2 applePosition;
    glm::vec2 gridSize;
    bool gameOver = false;
    float updateTimer;
    float updateInterval; 

    GameResult result;
    GameState state;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> xDist;
    std::uniform_real_distribution<float> zDist;

    NetworkManager networkManager;
};
