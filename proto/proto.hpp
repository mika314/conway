#pragma once
#include <ser/proto.hpp>
#include <ser/macro.hpp>

namespace Conway
{

  struct State
  {
    int32_t x{};
    int32_t y{};
    int32_t w{};
    int32_t h{};
    int32_t maxX{};
    int32_t maxY{};
    std::vector<unsigned char> data;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(x);        \
  SER_PROPERTY(y);        \
  SER_PROPERTY(w);        \
  SER_PROPERTY(h);        \
  SER_PROPERTY(maxX);     \
  SER_PROPERTY(maxY);     \
  SER_PROPERTY(data);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct ClientState
  {
    int32_t x{};
    int32_t y{};
    int32_t w{};
    int32_t h{};
#define SER_PROPERTY_LIST \
  SER_PROPERTY(x);        \
  SER_PROPERTY(y);        \
  SER_PROPERTY(w);        \
  SER_PROPERTY(h);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };
} // namespace Conway

using ConwayProto = Proto<Conway::State, Conway::ClientState>;
