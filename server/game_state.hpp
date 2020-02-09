#pragma once
#include <array>

struct GameState
{
  int maxX = 0;
  int maxY = 0;
  std::array<std::array<bool, 64>, 64> data;
};
