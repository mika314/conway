#pragma once
#include <vector>

class OStrm
{
public:
  constexpr explicit OStrm(std::vector<char> &buff) noexcept : buff(buff) {}
  OStrm(const OStrm&) = delete;
  OStrm &operator=(const OStrm &) = delete;
  constexpr auto write(const char *b, size_t sz) noexcept -> void
  {
    for (; sz > 0; --sz, ++b)
      buff.push_back(*b);
  }

private:
  std::vector<char> &buff;
};
