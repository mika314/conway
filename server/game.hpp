#pragma once
#include "game_state.hpp"
#include "vec2.hpp"
#include <unordered_set>

class Game
{
public:
  Game();
  void process();
  GameState getState(int x, int y) const;
  int getCellsCount() const;

private:
  std::unordered_set<Vec2> state;
  std::unordered_map<Vec2, int> histo;
  int maxX = 0;
  int maxY = 0;
  std::unordered_map<Vec2, int> neighbors;
  std::unordered_set<Vec2> newState;
  int z = 0;
};
