#include "gdexample.h"
#include <Camera.hpp>
#include <GridMap.hpp>
#include <Input.hpp>
#include <string>

using namespace godot;

void GDExample::_register_methods()
{
  register_method("_process", &GDExample::_process);
  register_method("_ready", &GDExample::_ready);
}

GDExample::GDExample() : connection(StreamPeerTCP::_new()) {}

GDExample::~GDExample()
{
  // add your cleanup here
}

void GDExample::_init() {}

void GDExample::_ready()
{
  Godot::print("ready");
  auto res = connection->connect_to_host("127.0.0.1", 42069);
  if (res != Error::OK)
    Godot::print(("connect error " + std::to_string(static_cast<int>(res))).c_str());

  auto gridMap = static_cast<GridMap *>(get_node("GridMap"));
  gridMap->set_cell_item(0, 0, 0, 0);
}

void GDExample::_process(float delta)
{
  auto dir = Vector3{};
  auto input = Input::get_singleton();
  if (input->is_action_pressed("ui_right"))
    ++dir.x;
  if (input->is_action_pressed("ui_left"))
    --dir.x;
  if (input->is_action_pressed("ui_down"))
    ++dir.z;
  if (input->is_action_pressed("ui_up"))
    --dir.z;
  if (input->is_action_pressed("ui_page_down"))
    --dir.y;
  if (input->is_action_pressed("ui_page_up"))
    ++dir.y;
  dir = dir.rotated(Vector3{0, 1, 0}, get_rotation().y);
  auto camera = static_cast<Camera *>(get_node("Camera"));
  {
    auto tmp = camera->get_translation();
    tmp += dir * CAM_SPEED * delta;
    tmp.x += (maxX * 2 + 64 - tmp.x) * 0.5 * delta;
    tmp.z += (maxY * 2 + 64 - tmp.z) * 0.5 * delta;
    camera->set_translation(tmp);
  }
  auto status = connection->get_status();
  if (status == StreamPeerTCP::STATUS_ERROR)
    Godot::print("status error");

  if (status != StreamPeerTCP::STATUS_CONNECTED)
  {
    if (wasConnected)
    {
      Godot::print("reconnect");
      auto res = connection->connect_to_host("127.0.0.1", 42069);
      if (res != Error::OK)
        Godot::print(("connect error " + std::to_string(static_cast<int>(res))).c_str());
      wasConnected = false;
    }
    return;
  }

  if (!wasConnected)
  {
    Godot::print("connected");
    wasConnected = true;
  }

  if (time > nextTime)
  {
    nextTime += 0.25f;
    connection->put_32(camera->get_translation().x / 2 - 32);
    connection->put_32(camera->get_translation().z / 2 - 32);
  }

  time += delta;

  if (connection->get_available_bytes() <= 64 * 64 / 8 + 4 + 4)
    return;
  auto tmpX = connection->get_32();
  auto tmpY = connection->get_32();
  maxX = connection->get_32();
  maxY = connection->get_32();
  auto buffer = static_cast<Array>(connection->get_data(64 * 64 / 8));
  auto tmp = buffer[1].operator PoolByteArray();
  auto gridMap = static_cast<GridMap *>(get_node("GridMap"));
  for (int xx = 0; xx < 64; ++xx)
    for (int yy = 0; yy < 64; ++yy)
    {
      if ((tmp[yy * 8 + xx / 8] & (1 << xx % 8)) == 0)
        gridMap->set_cell_item(xx, 0, yy, 0);
      else
        gridMap->set_cell_item(xx, 0, yy, 1);
    }
  {
    auto tmp = gridMap->get_translation();
    tmp.x = tmpX * 2;
    tmp.z = tmpY * 2;
    gridMap->set_translation(tmp);
  }
}
