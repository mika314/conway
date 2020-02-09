#include "game.hpp"
#include <Winsock2.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

int main()
{
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD(2, 2);
  auto err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0)
  {
    printf("WSAStartup failed with error: %d\n", err);
    return 1;
  }

  auto serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == 0)
  {
    std::cerr << "ERROR opening socket\n";
    return 2;
  }

  Game game;

  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(42069);
  auto res = bind(serverFd, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (res < 0)
  {
    auto e = errno;
    std::cerr << "ERROR on binding: " << e << std::endl;
    return 3;
  }
  listen(serverFd, 5);

  std::unordered_map<decltype(serverFd), std::pair<int, int>> peers;
  auto time = std::chrono::system_clock::now();
  auto nextProcessTime = time;
  auto nextUpdateTime = time;
  for (;;)
  {
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    auto maxFd = serverFd;
    FD_SET(serverFd, &rfds);
    maxFd = std::max(maxFd, serverFd);
    for (const auto &peer : peers)
    {
      FD_SET(peer.first, &rfds);
      FD_SET(peer.first, &efds);
      maxFd = std::max(maxFd, peer.first);
    }
    time = std::chrono::system_clock::now();
    if (time >= nextUpdateTime)
    {
      nextUpdateTime += std::chrono::milliseconds{1000 / 10};
      for (const auto &peer : peers)
        FD_SET(peer.first, &wfds);
    }

    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::min(nextProcessTime, nextUpdateTime) - time)
                  .count();
    if (usec < 0)
      usec = 0;

    struct timeval tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;
    int retval = select(maxFd + 1, &rfds, &wfds, &efds, &tv);
    if (retval == -1)
    {
      auto err = WSAGetLastError();
      std::cerr << "ERROR select failed\n";
      switch (err)
      {
      case WSANOTINITIALISED:
        std::cerr << "A successful WSAStartup call must occur before using this function.\n";
        break;
      case WSAEFAULT:
        std::cerr << "The Windows Sockets implementation was unable to allocate needed resources "
                     "for its internal operations, or the readfds, writefds, exceptfds, or timeval "
                     "parameters are not part of the user address space.\n";
        break;
      case WSAENETDOWN: std::cerr << "The network subsystem has failed.\n"; break;
      case WSAEINVAL:
        std::cerr
          << "The time-out value is not valid, or all three descriptor parameters were null.\n";
        break;
      case WSAEINTR:
        std::cerr
          << "A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n";
        break;
      case WSAEINPROGRESS:
        std::cerr << "A blocking Windows Sockets 1.1 call is in progress, or the service provider "
                     "is still processing a callback function.\n";
        break;
      case WSAENOTSOCK:
        std::cerr << "One of the descriptor sets contains an entry that is not a socket.\n";
        break;
      default: std::cerr << "Unknown error: " << err << std::endl;
      }
      return 4;
    }
    if (retval == 0)
    {
      continue;
    }
    if (FD_ISSET(serverFd, &rfds))
    {
      struct sockaddr_in clientAddr;
      int clientLen = sizeof(clientAddr);
      auto clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientLen);
      if (clientFd < 0)
      {
        std::cerr << "ERROR on accept\n";
      }
      std::cout << "new connection: " << clientFd << std::endl;
      peers.emplace(clientFd, std::make_pair(0, 0));
    }

    for (auto &peer : peers)
    {
      if (!FD_ISSET(peer.first, &wfds))
        continue;
      auto state = game.getState(peer.second.first, peer.second.second);
      std::array<uint64_t, 64> out = {};
      int y = 0;
      for (const auto &row : state.data)
      {
        int x = 0;
        for (auto cell : row)
        {
          if (cell)
            out[y] |= 1ULL << x;
          ++x;
        }
        ++y;
      }
      send(peer.first, (char *)&peer.second.first, sizeof(peer.second.first), 0);
      send(peer.first, (char *)&peer.second.second, sizeof(peer.second.second), 0);
      send(peer.first, (char *)&state.maxX, sizeof(state.maxX), 0);
      send(peer.first, (char *)&state.maxY, sizeof(state.maxY), 0);
      send(peer.first, (char *)out.data(), 8 * 64, 0);
    }

    time = std::chrono::system_clock::now();
    if (time >= nextProcessTime)
    {
      nextProcessTime += std::chrono::milliseconds{1000 / 10};
      game.process();
    }

    std::unordered_set<decltype(serverFd)> toDel;
    for (auto &peer : peers)
    {
      if (FD_ISSET(peer.first, &rfds))
      {
        int32_t x, y;
        auto n = recv(peer.first, (char *)&x, sizeof(x), 0);
        if (n <= 0)
        {
          std::cout << "Peer " << peer.first << " is disconnected\n";
          closesocket(peer.first);
          toDel.insert(peer.first);
          continue;
        }
        n = recv(peer.first, (char *)&y, sizeof(y), 0);
        if (n <= 0)
        {
          std::cout << "Peer " << peer.first << " is disconnected\n";
          closesocket(peer.first);
          toDel.insert(peer.first);
        }
        peer.second = std::make_pair(x, y);
      }
      if (FD_ISSET(peer.first, &efds))
      {
        std::cout << "Peer " << peer.first << " is disconnected\n";
        closesocket(peer.first);
        toDel.insert(peer.first);
      }
    }
    for (auto fd : toDel)
      peers.erase(fd);
  }
  closesocket(serverFd);
}
