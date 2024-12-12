#pragma once


enum class GameResult : uint8_t
{
    Tie,
    Snake1,
    Snake2
};

enum class GameState : uint8_t
{
    NonActive,
    Pause,
    Active
};
