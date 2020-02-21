#include "game.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>

static const int InitialCells = 10000;

Game::Game()
{
  for (int i = 0; i < InitialCells; ++i)
    state.emplace(rand() % 300, rand() % 300);
}

void Game::process()
{
  neighbors.clear();
  for (auto cell : state)
  {
    for (int x = -1; x <= 1; ++x)
      for (int y = -1; y <= 1; ++y)
      {
        if (x == 0 && y == 0)
          continue;
        neighbors[Vec2{cell.x + x, cell.y + y}]++;
      }
    histo[Vec2((cell.x + 0x4000'0000) / 16 - 0x4000'0000 / 16,
               (cell.y + 0x4000'0000) / 16 - 0x4000'0000 / 16)]++;
  }

  for (auto &bin : histo)
    bin.second = bin.second * 99 / 100;

  auto max = std::max_element(std::begin(histo),
                              std::end(histo),
                              [](const std::pair<Vec2, int> &x, const std::pair<Vec2, int> &y) {
                                return x.second < y.second;
                              });
  max->second += 16;
  maxX = max->first.x * 16 - 32;
  maxY = max->first.y * 16 - 32;

  newState.clear();
  for (auto n : neighbors)
  {
    auto isLive = state.find(n.first) != std::end(state);
    if (isLive && (n.second == 2 || n.second == 3))
      newState.insert(n.first);
    if (!isLive && n.second == 3)
      newState.insert(n.first);
  }
  std::swap(state, newState);
  if (state.size() < 1000)
  {
    const int x = rand() % 4000;
    const int y = rand() % 4000;
    for (int i = 0; i < InitialCells - 1000; ++i)
      state.emplace(rand() % 300 + x - 2000, rand() % 300 + y - 2000);
  }

  if (rand() % 400 == 0)
  {
    auto cnt = state.size() / 10;
    std::cout << "Deleting " << cnt << " cells" << std::endl;
    for (size_t i = 0; i < cnt; ++i)
      state.erase(state.begin());
  }
  if (z++ % 100 == 0)
    std::cout << "Number of cells: " << state.size() << std::endl;
}

Conway::State Game::getState(const Conway::ClientState &clientState) const
{
  Conway::State ret;
  ret.maxX = maxX;
  ret.maxY = maxY;
  ret.x = clientState.x;
  ret.y = clientState.y;
  ret.w = clientState.w;
  ret.h = clientState.h;
  ret.data.resize((ret.w * ret.h + 7) / 8);

  for (auto y = 0; y < ret.h; ++y)
    for (auto x = 0; x < ret.w; ++x)
    {
      const auto bit = (x + y * ret.w);
      if (state.find(Vec2{x + ret.x, y + ret.y}) != std::end(state))
        ret.data[bit / 8] |= (1u << (bit % 8));
      else
        ret.data[bit / 8] &= ~(1u << (bit % 8));
    }
  return ret;
}

int Game::getCellsCount() const
{
  return state.size();
}
