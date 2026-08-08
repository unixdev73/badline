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

namespace ne {
std::string getLoopbackInterface() {
  std::string out;
  ifaddrs *info{};

  if (getifaddrs(&info))
    return {};

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (std::string(entry->ifa_name).find("lo") == 0 && entry->ifa_addr &&
        entry->ifa_addr->sa_family == AF_INET) {
      out = entry->ifa_name;
      break;
    }
  }

  freeifaddrs(info);
  return out;
}

std::vector<in_addr> getInterfaceAddressesIPv4(std::string const &interface) {
  assert(interface.size() > 1);
  std::vector<in_addr> out{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return {};

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (interface == entry->ifa_name && entry->ifa_addr &&
        entry->ifa_addr->sa_family == AF_INET)
      out.push_back(((sockaddr_in *)entry->ifa_addr)->sin_addr);
  }

  freeifaddrs(info);

  return out;
}

std::vector<in6_addr> getInterfaceAddressesIPv6(std::string const &interface) {
  assert(interface.size() > 1);
  std::vector<in6_addr> out{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return {};

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (interface == entry->ifa_name && entry->ifa_addr &&
        entry->ifa_addr->sa_family == AF_INET6)
      // figure this part out...
      out.push_back(((sockaddr_in6 *)entry->ifa_addr)->sin6_addr);
  }

  freeifaddrs(info);

  return out;
}

bool setActiveInterface(Endpoint *const handle, std::string const &interface) {
  assert(handle && interface.size() > 1);

  std::string name{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return false;

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (interface == entry->ifa_name) {
      name = entry->ifa_name;
      break;
    }
  }

  freeifaddrs(info);

  if (!name.size())
    return false;

  handle->activeInterface = name;

  auto const addresses = getInterfaceAddressesIPv4(handle->activeInterface);
  assert(addresses.size() && "The active interface must have an IPv4 address");
  handle->activeAddress = addresses.front();
  return true;
}

std::string const &getActiveInterface(Endpoint *const handle) {
  assert(handle);

  return handle->activeInterface;
}
} // namespace ne
