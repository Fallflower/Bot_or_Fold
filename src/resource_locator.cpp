#include "resource_locator.h"

#include <system_error>
#include <vector>

namespace holdem::resources {

std::filesystem::path locate(const std::string& relativePath) {
    namespace fs = std::filesystem;
    const fs::path requested(relativePath);
    if (requested.empty()) {
        return {};
    }

    std::error_code error;
    std::vector<fs::path> candidates;
    if (requested.is_absolute()) {
        candidates.push_back(requested);
    } else {
        candidates.push_back(requested);
        candidates.push_back(fs::current_path(error) / requested);
        error.clear();
        candidates.push_back(fs::current_path(error) / "resources" / requested);
    }

    for (const fs::path& candidate : candidates) {
        error.clear();
        if (fs::is_regular_file(candidate, error) && !error) {
            return fs::absolute(candidate, error);
        }
    }

    // Return the canonical runtime-relative spelling for a useful open/error
    // path even before a resource has been copied into the package.
    return requested;
}

} // namespace holdem::resources
