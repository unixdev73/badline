/* Copyright (c) 2026 unixdev73@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <badline/networking.hpp>

namespace ne {
constexpr char const MULTICAST_ADDRESS[] = "224.0.0.1";

struct BLOM_ServerEndpoint {
  static constexpr int MAX_BACKUP = 256;
  int udpSocket{-1}; // Sender
  char *udpOutDataStorage{};
  int *udpOutDataSizes{};
  int udpDataIndex{}; // Number of packet - used for retransmission

  static constexpr int MAX_TCP_CONNECTIONS = 64;
  int tcpSocket{-1};     // Listener
  int *tcpConnections{}; // Receivers
  char *tcpInDataStorage{};
  int *tcpInDataSizes{};
  int tcpActiveClient{};
  unsigned clientCount{};
};

struct BLOM_ClientEndpoint {
  int udpSocket{-1}; // Receiver
  char udpInDataStorage[BLOM_MAX_PACKET_SIZE];
  int udpInDataSize{};
  int udpDataIndex{}; // Expected number of packet

  int tcpSocket{-1};        // Sender
  in_addr tcpDestination{}; // Server address
  bool isConnected{};
};

struct BLOM_Endpoint {
  union {
    BLOM_ServerEndpoint server;
    BLOM_ClientEndpoint client;
  };
};

struct Endpoint {
  // Common data
  std::string activeInterface{};
  in_addr activeAddress{};
  std::size_t flags{};
  int port{};
  bool customAlloc{};

  // Protocol-specific data
  union {
    BLOM_Endpoint blom;
  };
};

void storeUDP(BLOM_ServerEndpoint *const handle, void *data,
              std::size_t const size);
} // namespace ne
