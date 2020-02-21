#include "gdexample.hpp"
#include "public_key.hpp"
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

GDExample::GDExample()
{
}

GDExample::~GDExample()
{
  // add your cleanup here
}

void GDExample::_init() {}

void GDExample::_ready()
{
  Godot::print("ready");

  auto gridMap = static_cast<GridMap *>(get_node("GridMap"));
  gridMap->set_cell_item(0, 0, 0, 0);
}

void GDExample::_process(float delta)
{
  if (!conn)
  {
    conn = std::make_unique<Net::Conn>(sched, PublicKey, "localhost", 42069);
    conn->onConn = [this]() { isConnected = true; };
    conn->onRecv = [this](const char *buff, size_t sz) {
      ConwayProto proto;
      IStrm strm(buff, buff + sz);
      proto.deser(strm, *this);
    };
    conn->onDisconn = [this]() {
      isConnected = false;
      conn = nullptr;
    };
  }
  sched.processNoWait();
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
    tmp += dir * CamSpeed * delta;
    tmp.x += (maxX * 2 + w(camera->get_translation().y) - tmp.x) * 0.5 * delta;
    tmp.z += (maxY * 2 + h(camera->get_translation().y) - tmp.z) * 0.5 * delta;
    camera->set_translation(tmp);
  }

  if (!isConnected)
  {
    if (wasConnected)
    {
      Godot::print("Disconnected");
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
    proto::ClientState state;
    state.w = w(camera->get_translation().y);
    state.h = h(camera->get_translation().y);
    state.x = camera->get_translation().x / 2 - state.w / 2;
    state.y = camera->get_translation().z / 2 - state.h / 2;
    OStrm strm;
    ConwayProto proto;
    proto.ser(strm, state);
    conn->send(strm.str().data(), strm.str().size());
    nextTime += 0.25f;
  }

  time += delta;
}

void GDExample::operator()(const proto::ClientState &)
{
  Godot::print("Unexpected");
}

void GDExample::operator()(const proto::State &state)
{
  auto tmpX = state.x;
  auto tmpY = state.y;
  maxX = state.maxX;
  maxY = state.maxY;
  auto gridMap = static_cast<GridMap *>(get_node("GridMap"));
  for (int xx = 0; xx < state.w; ++xx)
    for (int yy = 0; yy < state.h; ++yy)
    {
      const auto bit = yy * state.w + xx;
      if ((state.data[bit / 8] & (1 << bit % 8)) == 0)
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

int GDExample::w(float y) const
{
  return y * 3 / 4;
}

int GDExample::h(float y) const
{
  return y * 3 / 8;
}
