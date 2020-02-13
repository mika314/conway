#pragma once
#include <Godot.hpp>
#include <Spatial.hpp>
#include <StreamPeerTCP.hpp>
#include <memory>

namespace godot
{
  class GDExample : public Spatial
  {
    GODOT_CLASS(GDExample, Spatial)

  private:
    StreamPeerTCP *connection;
    bool wasConnected = false;
    const float CAM_SPEED = 14.0f;
    int maxX = 0;
    int maxY = 0;
    float nextTime = 0.0f;
    float time = 0.0f;

  public:
    static void _register_methods();

    GDExample();
    ~GDExample();

    void _init(); // our initializer called by Godot
    void _ready();

    void _process(float delta);
  };
} // namespace godot
