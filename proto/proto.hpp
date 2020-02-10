#pragma once
#include <ser/proto.hpp>
#include <ser/macro.hpp>

namespace proto
{

  struct State
  {
    int32_t x{};
    int32_t y{};
    int32_t maxX{};
    int32_t maxY{};
    std::vector<char> data;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(x);        \
  SER_PROPERTY(y);        \
  SER_PROPERTY(maxX);     \
  SER_PROPERTY(maxY);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct Pos
  {
    int32_t x{};
    int32_t y{};
#define SER_PROPERTY_LIST \
  SER_PROPERTY(x);        \
  SER_PROPERTY(y);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };
} // namespace proto

using ConwayProto = Proto<proto::State, proto::Pos>;
