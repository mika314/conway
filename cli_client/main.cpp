#include "public_key.hpp"
#include <iostream>
#include <net/conn.hpp>
#include <proto/proto.hpp>
#include <sched/sched.hpp>
#include <ser/overloaded.hpp>

int main()
{
  Sched sched;
  Net::Conn conn(sched, PublicKey, "localhost", 42069);
  conn.onRecv = [](const char *buff, size_t sz) {
    ConwayProto proto;
    IStrm strm(buff, buff + sz);
    proto.deser(strm,
                overloaded{[](const proto::ClientState &) { std::clog << "pos" << std::endl; },
                           [](const proto::State &) { std::clog << "state" << std::endl; }});
  };
  conn.onConn = []() { std::clog << "connected" << std::endl; };
  conn.onDisconn = []() { std::clog << "disconnected" << std::endl; };
}
