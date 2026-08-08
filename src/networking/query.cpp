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

#ifdef MCR_WINDOWS
#include <iphlpapi.h>
#endif

namespace ne {
#ifdef MCR_UNIX
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

std::vector<in_addr> getInterfaceAddressesIPv4(std::string const &ifc) {
  assert(ifc.size() > 1);
  std::vector<in_addr> out{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return {};

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (ifc == entry->ifa_name && entry->ifa_addr &&
        entry->ifa_addr->sa_family == AF_INET)
      out.push_back(((sockaddr_in *)entry->ifa_addr)->sin_addr);
  }

  freeifaddrs(info);

  return out;
}

std::vector<in6_addr> getInterfaceAddressesIPv6(std::string const &ifc) {
  assert(ifc.size() > 1);
  std::vector<in6_addr> out{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return {};

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (ifc == entry->ifa_name && entry->ifa_addr &&
        entry->ifa_addr->sa_family == AF_INET6)
      // figure this part out...
      out.push_back(((sockaddr_in6 *)entry->ifa_addr)->sin6_addr);
  }

  freeifaddrs(info);

  return out;
}

bool setActiveInterface(Endpoint *const handle, std::string const &ifc) {
  assert(handle && ifc.size() > 1);

  std::string name{};
  ifaddrs *info{};

  if (getifaddrs(&info))
    return false;

  for (auto entry = info; entry; entry = entry->ifa_next) {
    if (ifc == entry->ifa_name) {
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

#elif defined(MCR_WINDOWS)
std::string wideToUtf8(wchar_t const *str) {
  if (!str)
    return {};

  int size =
      WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);

  if (size <= 1)
    return {};

  std::string out(size - 1, '\0');

  WideCharToMultiByte(CP_UTF8, 0, str, -1, out.data(), size, nullptr, nullptr);

  return out;
}

std::vector<unsigned char> getAdaptersBuffer() {
  ULONG size = 0;

  if (GetAdaptersAddresses(AF_INET,
                           GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                               GAA_FLAG_SKIP_DNS_SERVER,
                           nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW) {
    return {};
  }

  std::vector<unsigned char> buffer(size);

  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

  if (GetAdaptersAddresses(AF_INET,
                           GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                               GAA_FLAG_SKIP_DNS_SERVER,
                           nullptr, adapters, &size) != NO_ERROR) {
    return {};
  }

  return buffer;
}

std::string getLoopbackInterface() {
  auto buffer = getAdaptersBuffer();
  if (buffer.empty())
    return {};

  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

  for (auto *adapter = adapters; adapter; adapter = adapter->Next) {
    if (!adapter->AdapterName)
      continue;

    for (auto *address = adapter->FirstUnicastAddress; address;
         address = address->Next) {

      if (!address->Address.lpSockaddr ||
          address->Address.lpSockaddr->sa_family != AF_INET) {
        continue;
      }

      auto *addr = reinterpret_cast<sockaddr_in *>(address->Address.lpSockaddr);

      if (addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
        return wideToUtf8(adapter->FriendlyName);
    }
  }

  return {};
}

std::vector<in_addr> getInterfaceAddressesIPv4(std::string const &ifc) {
  assert(ifc.size() > 1);

  auto buffer = getAdaptersBuffer();
  if (buffer.empty())
    return {};

  std::vector<in_addr> out{};

  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

  for (auto *adapter = adapters; adapter; adapter = adapter->Next) {
    if (!adapter->AdapterName || ifc != wideToUtf8(adapter->FriendlyName)) {
      continue;
    }

    for (auto *address = adapter->FirstUnicastAddress; address;
         address = address->Next) {

      if (!address->Address.lpSockaddr ||
          address->Address.lpSockaddr->sa_family != AF_INET) {
        continue;
      }

      auto *addr = reinterpret_cast<sockaddr_in *>(address->Address.lpSockaddr);

      out.push_back(addr->sin_addr);
    }

    break;
  }

  return out;
}

bool setActiveInterface(Endpoint *const handle, std::string const &ifc) {
  assert(handle && ifc.size() > 1);

  auto buffer = getAdaptersBuffer();
  if (buffer.empty())
    return false;

  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

  bool found = false;

  for (auto *adapter = adapters; adapter; adapter = adapter->Next) {
    if (adapter->AdapterName && ifc == wideToUtf8(adapter->FriendlyName)) {
      found = true;
      break;
    }
  }

  if (!found)
    return false;

  handle->activeInterface = ifc;

  auto const addresses = getInterfaceAddressesIPv4(handle->activeInterface);

  assert(!addresses.empty() &&
         "The active interface must have an IPv4 address");

  if (addresses.empty())
    return false;

  handle->activeAddress = addresses.front();

  return true;
}
#endif

std::string const &getActiveInterface(Endpoint *const handle) {
  assert(handle);

  return handle->activeInterface;
}
} // namespace ne
