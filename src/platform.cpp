#include "tsuki/platform.hpp"
#include <filesystem>
#include <algorithm>
#include <cctype>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
#endif

namespace tsuki {

Platform::OS Platform::getCurrentOS() {
#ifdef __APPLE__
    return OS::MacOS;
#elif defined(_WIN32)
    return OS::Windows;
#elif defined(__linux__)
    return OS::Linux;
#else
    return OS::Unknown;
#endif
}

std::string Platform::getCurrentPlatform() {
    switch (getCurrentOS()) {
        case OS::Windows: return "windows";
        case OS::MacOS: return "macos";
        case OS::Linux: return "linux";
        default: return "unknown";
    }
}

bool Platform::setEnvironmentVariable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name.c_str(), value.c_str()) != 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

bool Platform::unsetEnvironmentVariable(const std::string& name) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name.c_str(), nullptr) != 0;
#else
    return unsetenv(name.c_str()) == 0;
#endif
}

FILE* Platform::openProcess(const std::string& command) {
#ifdef _WIN32
    return _popen(command.c_str(), "r");
#else
    return popen(command.c_str(), "r");
#endif
}

int Platform::closeProcess(FILE* pipe) {
    if (!pipe) return -1;

#ifdef _WIN32
    return _pclose(pipe);
#else
    return pclose(pipe);
#endif
}

bool Platform::hasPathSeparator(const std::string& path) {
    if (path.find('/') != std::string::npos) {
        return true;
    }
#ifdef _WIN32
    return path.find('\\') != std::string::npos;
#else
    return false;
#endif
}

char Platform::getPathSeparator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::string Platform::normalizePath(const std::string& path) {
    std::string result = path;
    char preferred = getPathSeparator();

#ifdef _WIN32
    // On Windows, convert forward slashes to backslashes
    std::replace(result.begin(), result.end(), '/', '\\');
#else
    // On Unix, convert backslashes to forward slashes (rare but possible)
    std::replace(result.begin(), result.end(), '\\', '/');
#endif

    return result;
}

bool Platform::makeExecutable(const std::string& filePath) {
#ifdef _WIN32
    // Windows doesn't need explicit executable permissions
    return true;
#else
    try {
        std::filesystem::permissions(filePath,
            std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
            std::filesystem::perms::others_read | std::filesystem::perms::others_exec);
        return true;
    } catch (const std::exception&) {
        return false;
    }
#endif
}

std::string Platform::getExecutableName(const std::string& baseName) {
    return getExecutableName(baseName, getCurrentPlatform());
}

std::string Platform::getExecutableName(const std::string& baseName, const std::string& targetPlatform) {
    if (targetPlatform == "windows") {
        return baseName + ".exe";
    }
    return baseName;
}

std::string Platform::findExecutableInPath(const std::string& executableName) {
    std::string command;

#ifdef _WIN32
    // Windows: use 'where' command
    command = "where " + executableName;
    if (executableName.find(".exe") == std::string::npos) {
        command = "where " + executableName + ".exe";
    }
#else
    // Unix/Linux/macOS: use 'which' command
    command = "which " + executableName;
#endif

    std::string result = executeCommand(command);
    result = trim(result);

    // Verify the file actually exists
    if (!result.empty() && std::filesystem::exists(result)) {
        return result;
    }

    return "";
}

std::string Platform::executeCommand(const std::string& command) {
    FILE* pipe = openProcess(command);
    if (!pipe) {
        return "";
    }

    std::string result;
    char buffer[512];

    if (fgets(buffer, sizeof(buffer), pipe)) {
        result = buffer;
    }

    closeProcess(pipe);
    return result;
}

std::string Platform::trim(const std::string& str) {
    std::string result = str;

    // Remove trailing newlines and carriage returns
    while (!result.empty() &&
           (result.back() == '\n' || result.back() == '\r' ||
            result.back() == ' ' || result.back() == '\t')) {
        result.pop_back();
    }

    // Remove leading whitespace
    size_t start = 0;
    while (start < result.length() &&
           (result[start] == ' ' || result[start] == '\t')) {
        start++;
    }

    return result.substr(start);
}

} // namespace tsuki