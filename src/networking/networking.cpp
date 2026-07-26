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

#include "./networking.hpp"
#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#define MCRLOG(endp, str)                                                      \
  do {                                                                         \
    assert(endp);                                                              \
    if (endp->flags & ENDPOINT_CREATION_FLAGS_VERBOSE)                         \
      std::cout << "INFO: " << __func__ << ": " << str << std::endl;           \
  } while (0);

namespace ne {

void storeUDP(BLOM_ServerEndpoint *const handle, void *data,
              std::size_t const size) {
  assert(handle);
  assert(handle->udpDataIndex < BLOM_ServerEndpoint::MAX_BACKUP);
  assert(size <= BLOM_MAX_PACKET_SIZE);

  memcpy(handle->udpOutDataStorage +
             handle->udpDataIndex * BLOM_MAX_PACKET_SIZE,
         data, size);

  handle->udpOutDataSizes[handle->udpDataIndex] = size;

  handle->udpDataIndex =
      (handle->udpDataIndex + 1) % BLOM_ServerEndpoint::MAX_BACKUP;
}

void blomSend(Endpoint *const handle, void *const data, int const size) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  assert(data);
  assert(size);
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
  assert(handle->activeInterface.size() &&
         "An active interface must be selected");

  if (handle->flags & ENDPOINT_CREATION_FLAGS_USAGE_SERVER) {
    MCRLOG(handle, "Server is sending data over UDP...");
    auto serv = &handle->blom.server;
    sockaddr_in recipients{};
    recipients.sin_family = AF_INET;
    recipients.sin_port = htons(handle->port);
    recipients.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);

    assert(sendto(serv->udpSocket, data, size, 0, (sockaddr *)&recipients,
                  sizeof(recipients)) != -1);

    storeUDP(serv, data, size);
    MCRLOG(handle, "Server finished sending data over UDP");

  } else {
    MCRLOG(handle, "Client is sending data over TCP...");
    auto client = &handle->blom.client;
    assert(send(client->tcpSocket, data, size, 0) != -1);
    MCRLOG(handle, "Client finished sending data over TCP");
  }
}

void blomReceive(Endpoint *const handle, void *const data, int *const size) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
  assert(handle->activeInterface.size() &&
         "An active interface must be selected");
  assert(data);
  assert(size);

  if (handle->flags & ENDPOINT_CREATION_FLAGS_USAGE_SERVER) {
    auto serv = &handle->blom.server;

    auto &fd = serv->tcpConnections[serv->tcpActiveClient];
    if (fd == -1) {
      sockaddr_in addr{};
      auto asz = sizeof(addr);
      fd = accept(serv->tcpSocket, (sockaddr *)&addr, (socklen_t *)&asz);
      if (fd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        serv->tcpActiveClient = (serv->tcpActiveClient + 1) %
                                BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS;
        *size = 0;
        return;
      }
      assert(fd != -1);
      fcntl(fd, F_SETFL, O_NONBLOCK);
    }

    auto dst =
        serv->tcpInDataStorage + serv->tcpActiveClient * BLOM_MAX_PACKET_SIZE;
    int bytes = 0, total = 0;
    do {
      bytes = recv(fd, dst, BLOM_MAX_PACKET_SIZE, 0);
      if (bytes > 0)
        total += bytes;
    } while (bytes > 0 && total <= BLOM_MAX_PACKET_SIZE);

    auto sz = serv->tcpInDataSizes + serv->tcpActiveClient * sizeof(int);
    *sz = total;

    if (!bytes) {
      close(fd);
      fd = -1;
    }

    memcpy(data, dst, total);
    *size = total;
    serv->tcpActiveClient =
        (serv->tcpActiveClient + 1) % BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS;
  } else {
    auto client = &handle->blom.client;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(handle->port);
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    auto const addrSz = sizeof(addr);

    int result = recvfrom(client->udpSocket, client->udpInDataStorage,
                          BLOM_MAX_PACKET_SIZE, MSG_DONTWAIT, (sockaddr *)&addr,
                          (socklen_t *)&addrSz);

    if (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      *size = 0;
      return;
    }
    assert((result != -1) && "Failed to receive UDP packet");

    memcpy(data, client->udpInDataStorage, result);
    *size = result;
  }
}

bool blomConnect(Endpoint *const handle) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  auto client = &handle->blom.client;

  assert(handle->activeInterface.size() &&
         "An active interface must be selected");
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_USAGE_CLIENT);
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
  assert(client->udpSocket != -1);

  sockaddr_in udpAddr{};
  udpAddr.sin_family = AF_INET;
  udpAddr.sin_port = htons(handle->port);
  udpAddr.sin_addr.s_addr = htons(INADDR_ANY);
  auto const addrSz = sizeof(udpAddr);

  int result = recvfrom(client->udpSocket, client->udpInDataStorage,
                        BLOM_MAX_PACKET_SIZE, MSG_DONTWAIT,
                        (sockaddr *)&udpAddr, (socklen_t *)&addrSz);

  if (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return false;
  assert((result != -1) && "Failed to receive UDP packet");

  if (result != sizeof(BLOM_ServerAnnouncement))
    return false;

  BLOM_ServerAnnouncement info{};
  memcpy(&info, client->udpInDataStorage, result);

  if (info.header.identifier != BLOM_HEADER_IDENTIFIER)
    return false;

  if (info.header.version != BLOM_HEADER_VERSION)
    return false;

  if (info.header.type != BLOM_TYPE_SERVER_ANNOUNCEMENT)
    return false;

  client->udpDataIndex =
      (info.header.number + 1) % BLOM_ServerEndpoint::MAX_BACKUP;

  sockaddr_in tcpAddr{};
  tcpAddr.sin_family = AF_INET;
  tcpAddr.sin_port = htons(handle->port);
  tcpAddr.sin_addr = info.address;

  result = connect(client->tcpSocket, (sockaddr *)&tcpAddr, sizeof(tcpAddr));
  if (result == -1)
    perror("");
  assert(result > -1);

  client->tcpDestination = info.address;
  return true;
}

void blomAnnounce(Endpoint *const handle) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  auto serv = &handle->blom.server;

  assert(handle->activeInterface.size() &&
         "An active interface must be selected");
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_USAGE_SERVER);
  assert(handle->flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM);
  assert(serv->udpSocket != -1);

  sockaddr_in recipients{};
  recipients.sin_family = AF_INET;
  recipients.sin_port = htons(handle->port);
  recipients.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);

  BLOM_ServerAnnouncement data;
  data.header.identifier = BLOM_HEADER_IDENTIFIER;
  data.header.version = BLOM_HEADER_VERSION;
  data.header.number = serv->udpDataIndex;
  data.header.type = BLOM_TYPE_SERVER_ANNOUNCEMENT;
  data.address = handle->activeAddress;

  MCRLOG(handle, "Server is announcing its presence over UDP...");
  assert(sendto(serv->udpSocket, &data, sizeof(data), 0,
                (sockaddr *)&recipients, sizeof(recipients)) != -1);

  storeUDP(serv, &data, sizeof(data));
  MCRLOG(handle, "Server finished announcing its presence over UDP");
}

template <typename T>
void allocAndClear(T &ptr, std::size_t const size, int const init = 0) {
  ptr = (T)malloc(size);
  assert(ptr);
  memset(ptr, init, size);
}

void initialize_BLOM_server(Endpoint *const handle) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  assert(handle->activeInterface.size() &&
         "An active interface must be selected");

  auto serv = &handle->blom.server;

  MCRLOG(handle, "Server is initializing UDP backend...");
  serv->udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
  assert(serv->udpSocket != -1);

  assert(setsockopt(serv->udpSocket, IPPROTO_IP, IP_MULTICAST_IF,
                    &handle->activeAddress,
                    sizeof(handle->activeAddress)) != -1);

  MCRLOG(handle, "Server is allocating memory for UDP backend...");
  allocAndClear(serv->udpOutDataStorage,
                BLOM_ServerEndpoint::MAX_BACKUP * BLOM_MAX_PACKET_SIZE, 0);

  allocAndClear(serv->udpOutDataSizes,
                BLOM_ServerEndpoint::MAX_BACKUP * sizeof(int), -1);

  serv->udpDataIndex = 0;
  MCRLOG(handle, "Server finished initializing UDP backend");

  MCRLOG(handle, "Server is initializing TCP backend...");
  serv->tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
  assert(serv->tcpSocket != -1);

  sockaddr_in tcpAddr{};
  tcpAddr.sin_family = AF_INET;
  tcpAddr.sin_port = htons(handle->port);
  tcpAddr.sin_addr = handle->activeAddress;
  assert(bind(serv->tcpSocket, (sockaddr *)&tcpAddr, sizeof(tcpAddr)) != -1);
  MCRLOG(handle, "Server bound TCP socket");

  assert(listen(serv->tcpSocket, BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS) !=
         -1);
  MCRLOG(handle, "Server is listening on TCP socket");

  MCRLOG(handle, "Server is allocating memory for TCP backend...");
  allocAndClear(serv->tcpConnections,
                BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS * sizeof(int), -1);

  allocAndClear(serv->tcpInDataStorage,
                BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS * BLOM_MAX_PACKET_SIZE,
                0);

  allocAndClear(serv->tcpInDataSizes,
                BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS * sizeof(int), -1);

  fcntl(serv->tcpSocket, F_SETFL, O_NONBLOCK);
  MCRLOG(handle, "Server finished initializing TCP backend");
}

void initialize_BLOM_client(Endpoint *const handle, int const port) {
  assert(handle && "The endpoint handle cannot be a nullptr");
  assert(handle->activeInterface.size() &&
         "An active interface must be selected");

  auto client = &handle->blom.client;

  // Initialize UDP backend
  client->udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
  assert(client->udpSocket != -1);

  sockaddr_in info{};
  info.sin_family = AF_INET;
  info.sin_port = htons(port);
  info.sin_addr.s_addr = htonl(INADDR_ANY);
  auto const infoSz = sizeof(info);

  auto result = bind(client->udpSocket, (sockaddr *)&info, infoSz);
  if (result == -1)
    perror("initialize_BLOM_client");
  assert(result != -1);

  ip_mreq ip{};
  ip.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
  ip.imr_interface.s_addr = handle->activeAddress.s_addr;
  assert(setsockopt(client->udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ip,
                    sizeof(ip)) != -1);

  client->udpInDataSize = -1;
  client->udpDataIndex = 0;

  // Initialize TCP backend
  client->tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
  assert(client->tcpSocket != -1);
}

bool create(Endpoint **const handle, std::string const &interface,
            int const port, std::size_t const flags) {
  assert(handle);

  bool const isServer = flags & ENDPOINT_CREATION_FLAGS_USAGE_SERVER;
  bool const isClient = flags & ENDPOINT_CREATION_FLAGS_USAGE_CLIENT;
  assert((isServer && !isClient) || (!isServer && isClient));

  bool const isBLOM = flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM;
  assert(isBLOM && "A valid protocol must be specified");

  auto guard = std::unique_ptr<Endpoint, void (*)(Endpoint *const)>{
      new Endpoint{}, destroy};
  if (!guard)
    return false;
  guard->flags = flags;

  assert((port > 1024) && "Port number too low");
  guard->port = port;

  if (!setActiveInterface(guard.get(), interface)) {
    return false;
  }

  if (isServer) {
    if (isBLOM)
      initialize_BLOM_server(guard.get());
  } else {
    if (isBLOM)
      initialize_BLOM_client(guard.get(), port);
  }

  *handle = guard.release();
  return true;
}

void destroy(Endpoint *const handle) {
  assert(handle);

  if (handle->flags & ENDPOINT_CREATION_FLAGS_PROTOCOL_BLOM) {
    if (handle->flags & ENDPOINT_CREATION_FLAGS_USAGE_SERVER) {
      auto serv = &handle->blom.server;
      free(serv->udpOutDataStorage);
      free(serv->udpOutDataSizes);

      for (int idx = 0; idx < BLOM_ServerEndpoint::MAX_TCP_CONNECTIONS; ++idx)
        if (auto fd = serv->tcpConnections[idx]; fd > -1)
          close(fd);
      free(serv->tcpConnections);

      free(serv->tcpInDataStorage);
      free(serv->tcpInDataSizes);

      if (serv->udpSocket > -1)
        close(serv->udpSocket);
      if (serv->tcpSocket > -1)
        close(serv->tcpSocket);
    } else { // CLIENT
      auto client = &handle->blom.client;
      if (client->udpSocket > -1)
        close(client->udpSocket);
      if (client->tcpSocket > -1)
        close(client->tcpSocket);
    }
  }

  delete handle;
}

std::string to_string(BLOM_Type const type) {
  switch (type) {
  case BLOM_TYPE_SERVER_ANNOUNCEMENT:
    return "BLOM_TYPE_SERVER_ANNOUNCEMENT";
  default:
  }

  return "";
}
} // namespace ne
