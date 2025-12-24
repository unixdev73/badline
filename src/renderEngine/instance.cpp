#include "internals.hpp"

namespace re {
Result storeMissingInstanceExts(std::vector<std::string> const *const requested,
                                std::vector<std::string> *const missing) {

  uint32_t availExtCount{};
  std::vector<VkExtensionProperties> avail{};

  vkEnumerateInstanceExtensionProperties(0, &availExtCount, 0);
  avail.resize(availExtCount);
  vkEnumerateInstanceExtensionProperties(0, &availExtCount, avail.data());

  std::vector<std::string> available{};
  for (auto const &e : avail)
    available.push_back(e.extensionName);

  *missing = subtract<std::string>(available, *requested);
  return Result::Success;
}
} // namespace re
