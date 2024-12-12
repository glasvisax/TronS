#include "snake.h"
#include <iostream>

#define GAME_PREF
#include "../misc/game_preferences.h"

void printDirection(Direction dir);

Snake::Snake() : currentDirection(Direction::FORWARD), lastDirection(Direction::FORWARD), isAlive(true)
{
    bodyParts.reserve(maxSnakeSize);
    Reset();
}

void Snake::Reset() {
    bodyParts.clear();
    isAlive = true;
    currentDirection = Direction::FORWARD;
    lastDirection = Direction::FORWARD;
}

void Snake::Update(const glm::vec2& gridSize) {
    if (!isAlive) return;

    // Store current head position
    glm::vec2 oldHeadPos = bodyParts.front();
    glm::vec2 newHeadPos = oldHeadPos;

    // Update head position based on direction
    switch (currentDirection) {
        case Direction::FORWARD:
            newHeadPos.y -= 1;
            break;
        case Direction::BACKWARD:
            newHeadPos.y += 1;
            break;
        case Direction::LEFT:
            newHeadPos.x -= 1;
            break;
        case Direction::RIGHT:
            newHeadPos.x += 1;
            break;
    }
    lastDirection = currentDirection;

    // Wrap the position if needed
    WrapPosition(newHeadPos, gridSize);

    // Move body parts
    for (size_t i = bodyParts.size() - 1; i > 0; --i) {
        bodyParts[i] = bodyParts[i - 1];
    }
    bodyParts[0] = newHeadPos;

    // Check for collisions
    if (CheckCollision()) {
        isAlive = false;
        return;
    }
}

bool Snake::SetDirection(Direction dir) 
{
    // Prevent changing to opposite direction
    if (IsOppositeDirection(dir, lastDirection)) {
        return false;
    }
    currentDirection = dir;
    return true;
}

void Snake::AddBodyPart(const glm::vec2& pos) 
{
    if (maxSnakeSize <= bodyParts.size())
    {
        return;
    }
    bodyParts.push_back(pos);
}

bool Snake::CheckCollision() const {
    // Get head position
    const glm::vec2& head = bodyParts.front();

    // Check collision with body
    for (size_t i = 1; i < bodyParts.size(); ++i) {
        if (head == bodyParts[i]) {
            return true;
        }
    }

    return false;
}

bool Snake::HasEatenApple(const glm::vec2& applePos) const {
    return bodyParts.front() == applePos;
}

void Snake::WrapPosition(glm::vec2& position, const glm::vec2& gridSize) {
    // Wrap around each axis
    if (position.x < 0) position.x = gridSize.x - 1;
    else if (position.x >= gridSize.x) position.x = 0;

    if (position.y < 0) position.y = gridSize.y - 1;
    else if (position.y >= gridSize.y) position.y = 0;
}

bool Snake::IsOppositeDirection(Direction dir1, Direction dir2) const {
    return (dir1 == Direction::FORWARD && dir2 == Direction::BACKWARD) ||
        (dir1 == Direction::BACKWARD && dir2 == Direction::FORWARD) ||
        (dir1 == Direction::LEFT && dir2 == Direction::RIGHT) ||
        (dir1 == Direction::RIGHT && dir2 == Direction::LEFT);
}

void printDirection(Direction dir)
{
    switch(dir)
    {
        case(Direction::BACKWARD):
        {
            std::cout << "\nBACKWARD";
            break;
        }
        case(Direction::FORWARD):
        {
            std::cout << "\nFORWARD";
            break;
        }
        case(Direction::RIGHT):
        {
            std::cout << "\nRIGHT";
            break;
        }
        case(Direction::LEFT):
        {
            std::cout << "\nLEFT";
            break;
        }
    }
}