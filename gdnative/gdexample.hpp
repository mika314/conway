#pragma once
#include <Godot.hpp>
#include <Spatial.hpp>
#include <net/conn.hpp>
#include <proto/proto.hpp>
#include <sched/sched.hpp>

namespace godot
{
  class GDExample : public Spatial
  {
    GODOT_CLASS(GDExample, Spatial)

  public:
    static void _register_methods();

    GDExample();
    ~GDExample();

    void _init(); // our initializer called by Godot
    void _ready();

    void _process(float delta);

    void operator()(const Conway::ClientState &);
    void operator()(const Conway::State &);

  private:
    Sched sched;
    std::unique_ptr<Net::Conn> conn;
    bool wasConnected = false;
    const float CamSpeed = 14.0f;
    int maxX = 0;
    int maxY = 0;
    float nextTime = 0.0f;
    float time = 0.0f;
    bool isConnected = false;
    std::vector<char> buff;
    int w(float y) const;
    int h(float y) const;
  };
} // namespace godot
