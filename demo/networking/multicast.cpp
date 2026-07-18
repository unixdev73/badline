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

#include <badline/networking.hpp>
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
  constexpr char const *const group = "224.0.0.1";
  bool isServer = false;
  int port{};

  if (argc < 2) {
    std::cerr << "Usage: [--server] port" << std::endl;
    return 1;
  }

  if (argc == 2) {
    try {
      port = std::stoi(argv[1]);
    } catch (...) {
      std::cerr << "The input: " << argv[1] << " is not a valid port number."
                << std::endl;
      return 1;
    }
  } else {
    isServer = true;
    if (std::string{argv[1]} != "--server") {
      std::cerr << "Usage: [--server] port" << std::endl;
      return 1;
    }
    try {
      port = std::stoi(argv[2]);
    } catch (...) {
      std::cerr << "The input: " << argv[2] << " is not a valid port number."
                << std::endl;
      return 1;
    }
  }

  std::cout << (isServer ? "Running as server" : "Running as client");
  std::cout << " on port " << port << std::endl;

  int id = socket(PF_INET, SOCK_DGRAM, 0);
  if (id == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return 1;
  }

  /* AVOID WIRELESS */
  ifaddrs *ifs{};
  getifaddrs(&ifs);
  in_addr addr{};

  for (auto *p = ifs; p; p = p->ifa_next) {
    if (!p->ifa_addr)
      continue;
    if (strcmp(p->ifa_name, "wlan0") != 0 &&
        p->ifa_addr->sa_family == AF_INET) {
      addr = ((sockaddr_in *)p->ifa_addr)->sin_addr;
      break;
    }
  }
  freeifaddrs(ifs);

  if (isServer) {
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(group);

    in_addr multi{};
    multi.s_addr = addr.s_addr;
    if (setsockopt(id, IPPROTO_IP, IP_MULTICAST_IF, &multi, sizeof(multi)) ==
        -1) {
      std::cerr << "Failed to set server source interface" << std::endl;
      return 1;
    }

    constexpr const char *const msg = "hello world";

    while (1) {
      std::cout << "sending payload" << std::endl;
      int len =
          sendto(id, msg, strlen(msg), 0, (sockaddr *)&target, sizeof(target));
      if (len == -1) {
        std::cerr << "Failed to send message" << std::endl;
        return 1;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  } else {
    ip_mreq req{};
    req.imr_multiaddr.s_addr = inet_addr(group);
    req.imr_interface.s_addr = addr.s_addr;

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.s_addr, ip, sizeof(ip));

    std::cout << "Using multicast interface: " << ip << std::endl;

    sockaddr_in clientinf{};
    clientinf.sin_family = AF_INET;
    clientinf.sin_port = htons(port);
    clientinf.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(id, (sockaddr *)&clientinf, sizeof(clientinf)) == -1) {
      std::cerr << "Failed to bind client socket" << std::endl;
      return 1;
    }

    if (setsockopt(id, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&req,
                   sizeof(req)) == -1) {
      std::cerr << "Failed to configure client socket for multicast"
                << std::endl;
      return 1;
    }

    constexpr std::size_t sz = 512;

    while (1) {
      char data[sz];
      unsigned clientlen = sizeof(clientinf);
      int len = recvfrom(id, data, sz, 0, (sockaddr *)&clientinf, &clientlen);
      if (len == -1) {
        std::cerr << "Failed to receive data" << std::endl;
        return 1;
      }
      data[len] = '\0';
      std::cout << data << std::endl;
    }
  }

  return 0;
}
