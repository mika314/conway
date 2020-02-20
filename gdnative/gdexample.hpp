#pragma once
#include <Godot.hpp>
#include <Spatial.hpp>
#include <sched/sched.hpp>
#include <proto/proto.hpp>
#include <net/conn.hpp>


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

    void operator()(const proto::Pos &pos);
    void operator()(const proto::State &);

  private:
    Sched sched;
    Net::Conn conn;
    bool wasConnected = false;
    const float CamSpeed = 14.0f;
    int maxX = 0;
    int maxY = 0;
    float nextTime = 0.0f;
    float time = 0.0f;
    bool isConnected = false;
    std::vector<char> buff;
  };
} // namespace godot
