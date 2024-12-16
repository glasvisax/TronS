#include "game.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include "../misc/game_utils.h"

bool messageShown = false;
bool lastRender = false;
bool hasCurrentState = true;

Game::Game(int gridSizeX, int gridSizeZ):
	camera(50.0f, glm::vec3((gridSizeX-1)/2, 25, gridSizeX + 7), glm::vec3((gridSizeX - 1) / 2, 0.0f, (gridSizeZ - 1) / 2)),
	gridSize(gridSizeX, gridSizeZ),
	gen(rd()),
	xDist(0, gridSizeX - 1),
	zDist(0, gridSizeZ - 1)
{
}

void Game::Update(float deltaTime) 
{
	updateTimer += deltaTime;
	if (updateTimer >= updateInterval) {
		updateTimer = 0.0f;
		if (networkManager.IsServer())
		{
			networkManager.Update();
		}
		else 
		{
			do
			{
				networkManager.Update();
			} while (!hasCurrentState);
			if (state == GameState::Active) hasCurrentState = false;
			return;
		}

		if (state == GameState::Active) {

			snake1.Update(gridSize);
			snake2.Update(gridSize);

			// Сначала проверяем столкновения между змейками
			if (CheckSnakesCollision(result)) {
				StopGameMsg msg;
				msg.result = result;
				gameOver = true;
				state = GameState::Pause;
				lastRender = true;
				sendGameStateMsg();
				networkManager.sendStopGame(&msg);
				onGameOver(result);
				return;
			}

			// Затем проверяем самостолкновения
			if (snake1.CheckCollision()) {
				StopGameMsg msg;
				gameOver = true;
				result = GameResult::Snake2;
				state = GameState::Pause;
				lastRender = true;
				msg.result = result;
				sendGameStateMsg();
				networkManager.sendStopGame(&msg);
				onGameOver(result);
				return;
			}

			if (snake2.CheckCollision()) {
				StopGameMsg msg;
				gameOver = true;
				result = GameResult::Snake1;
				state = GameState::Pause;
				lastRender = true;
				msg.result = result;
				sendGameStateMsg();
				networkManager.sendStopGame(&msg);
				onGameOver(result);
				return;
			}

			if (snake1.HasEatenApple(applePosition)) {
				snake1.AddBodyPart(snake1.GetBodyParts().back());
				spawnApple();

				//updateInterval = max(0.15f, updateInterval - 0.01f);
			}
			else if (snake2.HasEatenApple(applePosition)) {
				snake2.AddBodyPart(snake2.GetBodyParts().back());
				spawnApple();
				//updateInterval = max(0.15f, updateInterval - 0.01f);
			}
			sendGameStateMsg();
		}
	}
}

void Game::ProcessInput(int key)
{
	switch (key) {
		case GLFW_KEY_W: {};
		case GLFW_KEY_UP:
		{
			if (networkManager.IsServer()) {
				snake1.SetDirection(Direction::FORWARD);
			}
			else {
				SnakeDirChangeMsg msg;
				msg.direction = Direction::FORWARD;
				networkManager.sendSnakeDirChange(&msg);
			}
			break;
		}
		case GLFW_KEY_DOWN: {};
		case GLFW_KEY_S:
		{
			if (networkManager.IsServer()) {
				snake1.SetDirection(Direction::BACKWARD);
			}
			else {
				SnakeDirChangeMsg msg;
				msg.direction = Direction::BACKWARD;
				networkManager.sendSnakeDirChange(&msg);
			}
			break;
		}
		case GLFW_KEY_LEFT: {};
		case GLFW_KEY_A:
		{
			if (networkManager.IsServer()) {
				snake1.SetDirection(Direction::LEFT);
			}
			else {
				SnakeDirChangeMsg msg;
				msg.direction = Direction::LEFT;
				networkManager.sendSnakeDirChange(&msg);
			}
			break;
		}
		case GLFW_KEY_RIGHT: {};
		case GLFW_KEY_D:
		{
			if (networkManager.IsServer()) {
				snake1.SetDirection(Direction::RIGHT);
			}
			else {
				SnakeDirChangeMsg msg;
				msg.direction = Direction::RIGHT;
				networkManager.sendSnakeDirChange(&msg);
			}
			break;
		}	

		default: break;
	}
}

void Game::Reset()
{
	snake1 = Snake();
	snake2 = Snake();

	messageShown = false;
	gameOver = false;
	updateTimer = 0.0f;
	updateInterval = 0.25f;
	lastRender = false;
	hasCurrentState = true;
}

void Game::ServerGameStart()
{
	Reset();
	glm::vec2 snake1_body[3];
	glm::vec2 snake2_body[3];
	glm::vec2 applePos = getAccessibleApplePos();
	snake1_body[0] = glm::vec2(2, 2);
	snake1_body[1] = snake1_body[0] - glm::vec2(1, 0);
	snake1_body[2] = snake1_body[0] - glm::vec2(2, 0);

	snake2_body[0] = glm::vec2(8, 8);
	snake2_body[1] = snake2_body[0] - glm::vec2(1, 0);
	snake2_body[2] = snake2_body[0] - glm::vec2(2, 0);
	GameStart(snake1_body, snake2_body, applePos);

	StartGameMsg msg;
	msg.apple_pos = msg.apple_pos = pos{ f_u8.get(applePos.x), f_u8.get(applePos.y) };
	auto& bodyParts1 = snake1.GetBodyParts();
	auto& bodyParts2 = snake2.GetBodyParts();
	for (uint8_t i = 0; i < 3; ++i)
	{
		msg.snake1_body[i] = pos{ f_u8.get(bodyParts1[i].x), f_u8.get(bodyParts1[i].y) };
	}
	for (uint8_t i = 0; i < 3; ++i)
	{
		msg.snake2_body[i] = pos{ f_u8.get(bodyParts2[i].x), f_u8.get(bodyParts2[i].y) };
	}
	msg.grid_size_x = gridSize.x;
	msg.grid_size_z = gridSize.y;
	networkManager.sendStartGame(&msg);
}

void Game::GameStart(glm::vec2* snake1_body, glm::vec2* snake2_body, glm::vec2 apple_pos)
{
	snake1.Reset();
	snake2.Reset();

	for(size_t i = 0; i < 3; ++i) {
		snake1.AddBodyPart(snake1_body[i]);
	}

	for (size_t i = 0; i < 3; ++i) {
		snake2.AddBodyPart(snake2_body[i]);
	}
	
	applePosition = apple_pos;
	state = GameState::Active;
}

void Game::initializeClient(int port, const char* address)
{
	if (!networkManager.InitializeClient(address, port))
	{
		assert(0 && "if (networkManager.InitializeClient(address, port))");
	}
	networkManager.onConnectionChange = std::bind(&Game::onConnectionChanged, this, std::placeholders::_1);
	networkManager.onGameStateReceive = std::bind(&Game::onGameStateReceived, this, std::placeholders::_1);
	networkManager.onStartGameReceive = std::bind(&Game::onStartGameReceived, this, std::placeholders::_1);
	networkManager.onStopGameReceive = std::bind(&Game::onStopGameReceived, this, std::placeholders::_1);
}

void Game::initializeServer(int& port)
{
	if(!networkManager.InitializeServer(port))
	{
		assert(0 && "if(networkManager.InitializeServer(port))");
	}
	networkManager.onConnectionChange = std::bind(&Game::onConnectionChanged, this, std::placeholders::_1);
	networkManager.onSnakeDirChangeReceive = std::bind(&Game::onSnakeDirChangeReceived, this, std::placeholders::_1);	
}

void Game::sendGameStateMsg()
{
	GameStateMsg msg;
	msg.apple_pos = pos{ f_u8.get(applePosition.x), f_u8.get(applePosition.y) };
	auto& bodyParts1 = snake1.GetBodyParts();
	auto& bodyParts2 = snake2.GetBodyParts();
	msg.snake1_body_sz = bodyParts1.size();
	msg.snake2_body_sz = bodyParts2.size();
	msg.snake1_dir = snake1.GetCurrentDirection();
	msg.snake2_dir = snake2.GetCurrentDirection();
	for (uint8_t i = 0; i < msg.snake1_body_sz; ++i) {
		msg.snake1_body[i] = pos{ f_u8.get(bodyParts1[i].x), f_u8.get(bodyParts1[i].y) };
	}
	for (uint8_t i = 0; i < msg.snake2_body_sz; ++i) {
		msg.snake2_body[i] = pos{ f_u8.get(bodyParts2[i].x), f_u8.get(bodyParts2[i].y) };
	}

	networkManager.sendGameState(&msg);
}

void Game::spawnApple()
{
	applePosition = getAccessibleApplePos();
}

glm::vec2 Game::getAccessibleApplePos()
{
	glm::vec2 newPos;
	do {
		newPos = glm::vec2(static_cast<int>(xDist(gen)), static_cast<int>(zDist(gen))
		);
	} while (!IsValidApplePosition(newPos));

	return newPos;
}

bool Game::IsValidApplePosition(const glm::vec2& pos) const 
{
	const auto& bodyParts1 = snake1.GetBodyParts();
	const auto& bodyParts2 = snake2.GetBodyParts();
	
	bool validForSnake1 = std::none_of(bodyParts1.begin(), bodyParts1.end(),
		[&pos](const glm::vec2& part) { return pos == part; });
		
	bool validForSnake2 = std::none_of(bodyParts2.begin(), bodyParts2.end(),
		[&pos](const glm::vec2& part) { return pos == part; });
		
	return validForSnake1 && validForSnake2;
}

bool Game::CheckSnakesCollision(GameResult& gameResult) const
{
    const auto& snake1Parts = snake1.GetBodyParts();
    const auto& snake2Parts = snake2.GetBodyParts();
    
    const auto& snake1Head = snake1Parts.front();
    const auto& snake2Head = snake2Parts.front();
    
    // Проверяем случай, когда змейки пытаются поменяться местами или сталкиваются головами
    if (snake1Head == snake2Head || 
        (snake1Parts.size() > 1 && snake2Parts.size() > 1 && 
         snake1Head == snake2Parts[1] && snake2Head == snake1Parts[1])) {
        gameResult = GameResult::Tie;
        return true;
    }
    
    // Проверяем столкновение головы первой змейки с телом второй
    for (const auto& part : snake2Parts) {
        if (snake1Head == part) {
            gameResult = GameResult::Snake2;
            return true;
        }
    }
    
    // Проверяем столкновение головы второй змейки с телом первой
    for (const auto& part : snake1Parts) {
        if (snake2Head == part) {
            gameResult = GameResult::Snake1;
            return true;
        }
    }
    
    return false;
}

void Game::onConnectionChanged(bool Connected)
{
	if(Connected) {
		if(state != GameState::Active) {
			state = GameState::Pause;
			if(onConnected)
			{
				onConnected();
			}
		}
	} else {
		if (state != GameState::NonActive) {
			state = GameState::NonActive;
			if (onDisconnected)
			{
				onDisconnected();
			}
		}
	}
	hasCurrentState = true;
}
void Game::onStartGameReceived(StartGameMsg* msg)
{
	Reset();
	hasCurrentState == true;
	glm::vec2 snake1_body[3];
	glm::vec2 snake2_body[3];

	for (size_t i = 0; i < 3; ++i) {
		snake1_body[i] = glm::vec2(u8_f.get(msg->snake1_body[i].x), u8_f.get(msg->snake1_body[i].z));
	}

	for (size_t i = 0; i < 3; ++i) {
		snake2_body[i] = glm::vec2(u8_f.get(msg->snake2_body[i].x), u8_f.get(msg->snake2_body[i].z));
	}
	SetGridSize(msg->grid_size_x, msg->grid_size_z);
	GameStart(snake1_body, snake2_body, glm::vec2(u8_f.get(msg->apple_pos.x), u8_f.get(msg->apple_pos.z )));
	if (onClientReceivedStart) onClientReceivedStart();
}
void Game::onGameStateReceived(GameStateMsg* msg)
{
	hasCurrentState = true;
	applePosition = glm::vec2(msg->apple_pos.x, msg->apple_pos.z);
	auto& bodyParts1 = snake1.getBodyParts();
	auto& bodyParts2 = snake2.getBodyParts();

	if(bodyParts1.size() < msg->snake1_body_sz) {
		bodyParts1.resize(msg->snake1_body_sz);
	}
	if (bodyParts2.size() < msg->snake2_body_sz) {
		bodyParts2.resize(msg->snake2_body_sz);
	}
	for (uint8_t i = 0; i < msg->snake1_body_sz; ++i) {
		bodyParts1[i] = glm::vec2{ u8_f.get(msg->snake1_body[i].x), u8_f.get(msg->snake1_body[i].z) };
	}
	for (uint8_t i = 0; i < msg->snake2_body_sz; ++i) {
		bodyParts2[i] = glm::vec2{ u8_f.get(msg->snake2_body[i].x), u8_f.get(msg->snake2_body[i].z) };
	}
	snake1.SetDirection(msg->snake1_dir);
	snake2.SetDirection(msg->snake2_dir);
}
void Game::onStopGameReceived(StopGameMsg* msg)
{
	result = msg->result;
	gameOver = true;
	state = GameState::Pause;
	lastRender = true;
	hasCurrentState = true;
	onGameOver(result);
}
void Game::onSnakeDirChangeReceived(SnakeDirChangeMsg* msg)
{
	snake2.SetDirection(msg->direction);
}
