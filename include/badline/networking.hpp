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

#include <string>
#include <vector>

#ifdef MCR_UNIX
#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#elif defined(MCR_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#error "UNSUPPORTED PLATFORM"
#endif

namespace ne {
enum EndpointCreationFlags : std::size_t {
  ENDPOINT_CREATION_FLAGS_USAGE_SERVER = 1,
  ENDPOINT_CREATION_FLAGS_USAGE_CLIENT = 1 << 1,
  ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM = 1 << 2,
  ENDPOINT_CREATION_FLAGS_VERBOSE = 1 << 3
};

enum BLOM_Type : unsigned char {
  BLOM_TYPE_SERVER_ANNOUNCEMENT,
};

std::string to_string(BLOM_Type const);

constexpr char const BLOM_HEADER_IDENTIFIER = 77;
constexpr char const BLOM_HEADER_VERSION = 1;
constexpr char const BLOM_MAX_ID_SIZE = 8;
constexpr std::size_t const BLOM_MAX_PACKET_SIZE = 512;

struct BLOM_UDP_Header {
  unsigned char identifier = BLOM_HEADER_IDENTIFIER;
  unsigned char version = BLOM_HEADER_VERSION;
  unsigned char number = 0; // Used for retransmission
  unsigned char type = 0;
};

struct BLOM_ServerAnnouncement {
  BLOM_UDP_Header header{};
  in_addr address{};
};

struct Endpoint;

bool create(Endpoint **const, std::string const &ifc, int const port,
            std::size_t const flags);
void destroy(Endpoint *const);

bool setActiveInterface(Endpoint *const, std::string const &ifc);
std::string const &getActiveInterface(Endpoint *const);

std::vector<in_addr> getInterfaceAddressesIPv4(std::string const &ifc);
std::string getLoopbackInterface();

#ifdef MCR_UNIX
std::vector<in6_addr> getInterfaceAddressesIPv6(std::string const &interface);
#endif

void blomAnnounce(Endpoint *const handle);
bool blomConnect(Endpoint *const handle);

void blomSend(Endpoint *const handle, void *const data, int const size);
void blomReceive(Endpoint *const handle, void *const data, int *const size);
}
