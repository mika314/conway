#include "game.hpp"
#include "private_key.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <log/log.hpp>
#include <net/conn.hpp>
#include <net/server.hpp>
#include <proto/proto.hpp>
#include <sched/sched.hpp>
#include <ser/overloaded.hpp>
#include <unordered_map>
#include <unordered_set>

int main()
{

  Sched sched;
  std::unordered_map<Net::Conn *, std::pair<int, int>> peers;
  Net::Server server(sched, PrivateKey, 42069, [&peers](Net::Conn *conn) {
    std::cout << "new connection: " << conn << std::endl;
    auto ret = peers.emplace(conn, std::make_pair(0, 0));
    auto &peer = ret.first->second;
    conn->onRecv = [&peer](const char *buff, size_t sz) {
      ConwayProto proto;
      IStrm strm(buff, buff + sz);
      proto.deser(
        strm,
        overloaded{[&peer](const proto::Pos &pos) { peer = std::make_pair(pos.x, pos.y); },
                   [](const proto::State &) { LOG("Unexpected"); }});
    };
    conn->onDisconn = [conn, &peers] {
      std::cout << "Peer " << conn << " is disconnected\n";
      peers.erase(conn);
    };
  });

  Game game;
  auto processTimer =
    sched.regTimer([&game]() { game.process(); }, std::chrono::milliseconds{1000 / 10}, true);

  auto updateTimer = sched.regTimer(
    [&game, &peers]() {
      for (auto &peer : peers)
      {
        auto state = game.getState(peer.second.first, peer.second.second);
        proto::State st;
        st.data.resize(8 * 64);
        for (int y = 0; y < 64; ++y)
          for (int x = 0; x < 64; ++x)
            if (state.data[y][x])
              st.data[y * 8 + x / 8] |= 1 << (x % 8);
        st.x = peer.second.first;
        st.y = peer.second.second;
        st.maxX = state.maxX;
        st.maxY = state.maxY;
        std::vector<char> buff;
        ConwayProto proto;
        OStrm strm{buff};
        proto.ser(strm, st);
        peer.first->send(buff.data(), buff.size());
      }
    },
    std::chrono::milliseconds{1000 / 10},
    true);
  for (;;)
    sched.process();
}
