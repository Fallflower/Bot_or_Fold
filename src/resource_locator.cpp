#include "resource_locator.h"

#include <array>
#include <cstdint>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <unistd.h>
#endif

namespace {

std::filesystem::path executableDirectory() {
#if defined(_WIN32)
    std::vector<wchar_t> buffer(512);
    for (;;) {
        const DWORD length = GetModuleFileNameW(
            nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            return {};
        }
        if (length < buffer.size() - 1) {
            return std::filesystem::path(buffer.data(), buffer.data() + length).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size == 0) {
        return {};
    }
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }
    std::error_code error;
    const std::filesystem::path executable =
        std::filesystem::weakly_canonical(buffer.data(), error);
    return error ? std::filesystem::path(buffer.data()).parent_path()
                 : executable.parent_path();
#elif defined(__linux__) && !defined(__ANDROID__)
    std::array<char, 4096> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) {
        return {};
    }
    return std::filesystem::path(
        std::string(buffer.data(), static_cast<std::size_t>(length))).parent_path();
#else
    return {};
#endif
}

} // namespace

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
        const fs::path executableDir = executableDirectory();
        if (!executableDir.empty()) {
            candidates.push_back(executableDir / requested);
            candidates.push_back(executableDir / "resources" / requested);
        }
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
