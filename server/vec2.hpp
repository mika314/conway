#pragma once
#include <unordered_map>

struct Vec2
{
  constexpr Vec2(int x = 0, int y = 0) : x(x), y(y) {}
  int x = 0;
  int y = 0;
  constexpr bool operator==(const Vec2 &other) const { return x == other.x && y == other.y; }
};

namespace std
{
  template <>
  class hash<Vec2>
  {
  public:
    size_t operator()(const Vec2 &v) const { return (v.x + v.y) * (v.x + v.y + 1) / 2 + v.y; }
  };
}; // namespace std