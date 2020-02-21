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
  std::unordered_map<Net::Conn *, proto::ClientState> peers;
  Net::Server server(sched, PrivateKey, 42069, [&peers](Net::Conn *conn) {
    std::cout << "new connection: " << conn << std::endl;
    auto ret = peers.emplace(conn, proto::ClientState{});
    auto &peer = ret.first->second;
    conn->onRecv = [&peer](const char *buff, size_t sz) {
      ConwayProto proto;
      IStrm strm(buff, buff + sz);
      proto.deser(strm,
                  overloaded{[&peer](const proto::ClientState &pos) { peer = pos; },
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
        ConwayProto proto;
        OStrm strm;
        proto.ser(strm, game.getState(peer.second));
        peer.first->send(strm.str().data(), strm.str().size());
      }
    },
    std::chrono::milliseconds{1000 / 10},
    true);
  for (;;)
    sched.process();
}
