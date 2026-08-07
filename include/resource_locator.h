#ifndef HOLDEM_RESOURCE_LOCATOR_H
#define HOLDEM_RESOURCE_LOCATOR_H

#include <filesystem>
#include <string>

namespace holdem::resources {

// Resolve a project resource relative to the runtime resource root.
// The returned path is suitable for std::ifstream on desktop and Android.
std::filesystem::path locate(const std::string& relativePath);

} // namespace holdem::resources

#endif
