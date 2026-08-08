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

#include <badline/argParser.hpp>
#include <badline/networking.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

std::unique_ptr<ap::ArgParser, void (*)(ap::ArgParser *const)>
createArgParser() {
  ap::ArgParser *handle{};
  ap::create(&handle);
  assert(handle);
  return {handle, ap::destroy};
}

std::unique_ptr<ne::Endpoint, void (*)(ne::Endpoint *const)>
createEndpoint(std::string const &ifc, int const port,
               std::size_t const flags) {
  ne::Endpoint *handle{};
  ne::create(&handle, ifc, port, flags);
  assert(handle);
  return {handle, ne::destroy};
}

int main(int argc, char **argv) {
  auto parser = createArgParser();

  ap::addOption(parser.get(), "interface", 'i');
  ap::addOption(parser.get(), "port", 'p');
  ap::addFlag(parser.get(), "server", 's');

  if (!ap::parse(parser.get(), argv, 0, argc)) {
    std::cout << "Parsing failed" << std::endl;
    std::cout << "Usage: [--server] --interface eth" << std::endl;
    return 1;
  }

  unsigned server{}, ifaceCount{}, portCount{};
  char const *ifc{}, *portStr{};
  unsigned port{};

  ap::getFlagCount(parser.get(), "server", &server);

  ap::getOptionCount(parser.get(), "interface", &ifaceCount);
  assert(ifaceCount && "An interface must be supplied");
  ap::getOptionValue(parser.get(), "interface", 0, &ifc);

  ap::getOptionCount(parser.get(), "port", &portCount);
  assert(portCount && "A port must be supplied");
  ap::getOptionValue(parser.get(), "port", 0, &portStr);
  try {
    port = std::stoi(std::string{portStr});
  } catch (...) {
    std::cerr << "The supplied port: " << portStr << " is not a valid number"
              << std::endl;
    return 1;
  }

  if (server) {
    auto endpoint =
        createEndpoint(ifc, port,
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
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
  }

  auto endpoint = createEndpoint(ifc, port,
                                 ne::ENDPOINT_CREATION_FLAGS_VERBOSE |
                                     ne::ENDPOINT_CREATION_FLAGS_USAGE_CLIENT |
                                     ne::ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
  std::size_t iterations = 3;
  bool connected = false;
  char data[] = "Hello world!";

  while (iterations--) {
    if (blomConnect(endpoint.get())) {
      std::cout << "connected to server" << std::endl;
      connected = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (!connected) {
    std::cout << "failed to connect to server" << std::endl;
  } else {
    blomSend(endpoint.get(), data, sizeof(data));
  }
  return 0;
}
