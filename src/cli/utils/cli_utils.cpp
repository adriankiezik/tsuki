#include "cli_utils.hpp"
#include "http_client.hpp"
#include <tsuki/platform.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <set>
#include <cstdlib>

#ifdef __linux__
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(_WIN32)
    #include <windows.h>
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

namespace tsuki::cli {

// TempDirectoryGuard implementation
TempDirectoryGuard::TempDirectoryGuard(const std::string& prefix)
    : should_cleanup_(true) {
    path_ = (std::filesystem::temp_directory_path() /
             (prefix + "_" + std::to_string(getpid()))).string();
    std::filesystem::create_directories(path_);
}

TempDirectoryGuard::~TempDirectoryGuard() {
    if (should_cleanup_) {
        try {
            std::filesystem::remove_all(path_);
        } catch (...) {
            // Silently ignore cleanup errors
        }
    }
}

TempDirectoryGuard::TempDirectoryGuard(TempDirectoryGuard&& other) noexcept
    : path_(std::move(other.path_)), should_cleanup_(other.should_cleanup_) {
    other.should_cleanup_ = false;
}

TempDirectoryGuard& TempDirectoryGuard::operator=(TempDirectoryGuard&& other) noexcept {
    if (this != &other) {
        if (should_cleanup_) {
            try {
                std::filesystem::remove_all(path_);
            } catch (...) {}
        }
        path_ = std::move(other.path_);
        should_cleanup_ = other.should_cleanup_;
        other.should_cleanup_ = false;
    }
    return *this;
}

// ProjectGuard implementation
ProjectGuard::ProjectGuard(std::string project_dir)
    : project_dir_(std::move(project_dir)), committed_(false) {}

ProjectGuard::~ProjectGuard() {
    if (!committed_) {
        try {
            std::filesystem::remove_all(project_dir_);
        } catch (...) {
            // Silently ignore cleanup errors
        }
    }
}

// Input validation
bool isValidProjectName(const std::string& name) {
    if (name.empty() || name.length() > 255) {
        return false;
    }

    // Check for invalid characters
    const std::string invalid_chars = "/\\:*?\"<>|";
    if (name.find_first_of(invalid_chars) != std::string::npos) {
        return false;
    }

    // Check reserved names on Windows
    static const std::set<std::string> reserved = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    std::string upper_name = name;
    for (char& c : upper_name) c = std::toupper(c);

    return reserved.find(upper_name) == reserved.end();
}

bool isValidPlatform(const std::string& platform) {
    return platform == "linux" || platform == "windows" || platform == "macos";
}

bool isValidArchitecture(const std::string& arch) {
    return arch == "x64" || arch == "x86";
}

// Path utilities
std::string getPlatformExecutableDir() {
    try {
#ifdef __linux__
        return std::filesystem::canonical("/proc/self/exe").parent_path().string();
#elif defined(__APPLE__)
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            return std::filesystem::canonical(path).parent_path().string();
        }
#elif defined(_WIN32)
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path().string();
#endif
    } catch (...) {
        // Fallback to current directory
    }
    return std::filesystem::current_path().string();
}

std::string resolveEnginePath(const char* argv0) {
    std::string engine_path = argv0;
    if (!tsuki::Platform::hasPathSeparator(engine_path)) {
        std::string which_result = tsuki::Platform::findExecutableInPath(engine_path);
        if (!which_result.empty()) {
            engine_path = which_result;
        }
    }
    return engine_path;
}

// Safe system operations (deprecated - use HttpClient instead)
int safeCurlDownload(const std::string& url, const std::string& output_path, bool headers_only) {
    // This function is kept for backward compatibility but now uses HttpClient internally
    // For new code, use HttpClient directly
    (void)headers_only; // Ignored for now

    HttpClient client;
    bool success = client.downloadFile(url, output_path);
    return success ? 0 : 1;
}

// String utilities
bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string autoAppendExtension(const std::string& path, const std::string& extension) {
    if (!endsWith(path, extension)) {
        return path + extension;
    }
    return path;
}

} // namespace tsuki::cli