#pragma once

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

enum class Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

class Snake {
public:
    Snake();
    ~Snake() = default;

    void Update(const glm::vec2& gridSize);
    bool SetDirection(Direction dir);
    void Reset();
    //void Reset(const glm::vec2& position);
    void AddBodyPart(const glm::vec2& pos);
    bool CheckCollision() const;
    bool HasEatenApple(const glm::vec2& applePos) const;
    std::vector<glm::vec2>& getBodyParts() { return bodyParts; }

    const std::vector<glm::vec2>& GetBodyParts() const { return bodyParts; }
    const glm::vec2& GetHeadPosition() const { return bodyParts.front(); }
    Direction GetCurrentDirection() const { return currentDirection; }

private:
    std::vector<glm::vec2> bodyParts;
    Direction currentDirection;
    Direction lastDirection;
    glm::vec2 startPosition;
    bool isAlive;

    void WrapPosition(glm::vec2& position, const glm::vec2& gridSize);
    bool IsOppositeDirection(Direction dir1, Direction dir2) const;
};
