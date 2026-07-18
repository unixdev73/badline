#include <badline/networking.hpp>
#include <cassert>
#include <iostream>

int main(int argc, char **argv) {
  assert(argc == 2 && "The demo requires an interface name");

  std::string const iface = argv[1];
  assert(iface.size() > 1 && "The interface name must be valid");

  auto const ip4 = ne::getInterfaceAddressesIPv4(iface);
  auto const ip6 = ne::getInterfaceAddressesIPv6(iface);

  if (ip4.empty() && ip6.empty()) {
    std::cout << "No addresses to show for interface: " << iface << std::endl;
    return 0;
  }

  std::cout << "Listing addresses of interface " << iface << ": " << std::endl;
  for (auto const &e : ip4)
    std::cout << "\t" << inet_ntoa(e) << std::endl;

  for (auto const &e : ip6) {
    char str[INET6_ADDRSTRLEN];

    if (!inet_ntop(AF_INET6, &e.__u6_addr, str, sizeof(str))) {
      std::cerr << "Failed to convert IPv6 address to string" << std::endl;
      return 1;
    }

    std::cout << '\t' << str << std::endl;
  }
}
