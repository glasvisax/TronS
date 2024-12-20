# TronS

A C++ implementation of the classic Tron game with network multiplayer support.

## Screenshots
![Gameplay](images/gameplay.jpg)
![Menu](images/menu.jpg)
![GameOver](images/gameover.jpg)
![Lobby](images/lobby.jpg)

## Features
- OpenGL-based graphics rendering
- Multiplayer support using ENet
- ImGui-based user interface

## Prerequisites
- CMake 
- C++17 compatible compiler
- Dependencies:
  - GLFW
  - GLAD
  - GLM
  - ENet
  - ImGui

## Project Structure
- `src/` - Source code files
  - `misc/` - Miscellaneous utility functions
  - `network/` - Networking implementation
  - `objects/` - Game objects and entities
  - `shaders/` - GLSL shader files
  - `world/` - Game world and state management
- `bin/` - Compiled binaries
- `build/` - Build files
