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
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <sys/wait.h>
#include <unistd.h>

std::unique_ptr<ne::Endpoint, void (*)(ne::Endpoint *const)>
createEndpoint(std::string const &interface, int const port,
               std::size_t const flags) {
  ne::Endpoint *handle{};
  ne::create(&handle, interface, port, flags);
  assert(handle);
  return {handle, ne::destroy};
}

int main(int argc, char **argv) {
  std::string interface = ne::getLoopbackInterface();
  if (!interface.size())
    return 1; // The test is unsupported on this machine. No loopback interface
  int port = 60'000;

  pid_t server = fork();
  if (!server) {
    auto endpoint =
        createEndpoint(interface, port,
                       ne::ENDPOINT_CREATION_FLAGS_VERBOSE |
                           ne::ENDPOINT_CREATION_FLAGS_USAGE_SERVER |
                           ne::ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
    std::size_t iterations = 5;

    char data[ne::BLOM_MAX_PACKET_SIZE];
    while (iterations--) {
      blomAnnounce(endpoint.get());
      memset(data, 0, ne::BLOM_MAX_PACKET_SIZE);
      int size{};
      blomReceive(endpoint.get(), data, &size);
      if (size) {
        std::cout << "received: " << data << std::endl;
        return 0;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 1;
  }

  pid_t client = fork();
  if (!client) {
    auto endpoint =
        createEndpoint(interface, port,
                       ne::ENDPOINT_CREATION_FLAGS_VERBOSE |
                           ne::ENDPOINT_CREATION_FLAGS_USAGE_CLIENT |
                           ne::ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
    std::size_t iterations = 5;
    bool connected = false;
    char data[] = "Hello world!";

    while (iterations--) {
      if (blomConnect(endpoint.get())) {
        std::cout << "connected to server" << std::endl;
        connected = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!connected) {
      std::cout << "failed to connect to server" << std::endl;
      return 1;
    }

    blomSend(endpoint.get(), data, sizeof(data));
    return 0;
  }

  wait(&server);
  wait(&client);
  return WEXITSTATUS(server) || WEXITSTATUS(client);
}
