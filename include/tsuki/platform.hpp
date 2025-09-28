#pragma once

#include <string>
#include <cstdio>

namespace tsuki {

/**
 * Cross-platform utility functions to abstract away platform-specific code.
 * This module provides clean interfaces for common platform-dependent operations
 * without requiring developers to handle #ifdef blocks throughout the codebase.
 */
class Platform {
public:
    // Platform identification
    enum class OS {
        Windows,
        MacOS,
        Linux,
        Unknown
    };

    /**
     * Get the current operating system.
     * @return OS enum value representing the current platform
     */
    static OS getCurrentOS();

    /**
     * Get the current platform as a string (for use with engine binaries).
     * @return "windows", "macos", "linux", or "unknown"
     */
    static std::string getCurrentPlatform();

    // Environment variables
    /**
     * Set an environment variable in a cross-platform way.
     * @param name Variable name
     * @param value Variable value
     * @return true if successful, false otherwise
     */
    static bool setEnvironmentVariable(const std::string& name, const std::string& value);

    /**
     * Unset an environment variable in a cross-platform way.
     * @param name Variable name
     * @return true if successful, false otherwise
     */
    static bool unsetEnvironmentVariable(const std::string& name);

    // Process execution
    /**
     * Open a process for reading in a cross-platform way.
     * @param command Command to execute
     * @return FILE* pointer or nullptr on failure
     */
    static FILE* openProcess(const std::string& command);

    /**
     * Close a process opened with openProcess().
     * @param pipe FILE* pointer from openProcess()
     * @return Exit code of the process
     */
    static int closeProcess(FILE* pipe);

    // Path handling
    /**
     * Check if a path contains any platform-specific path separator.
     * @param path Path string to check
     * @return true if path contains / or \ (on Windows)
     */
    static bool hasPathSeparator(const std::string& path);

    /**
     * Get the preferred path separator for the current platform.
     * @return "/" on Unix systems, "\" on Windows
     */
    static char getPathSeparator();

    /**
     * Normalize path separators to the current platform's preferred separator.
     * @param path Input path
     * @return Path with normalized separators
     */
    static std::string normalizePath(const std::string& path);

    // File operations
    /**
     * Make a file executable (Unix only, no-op on Windows).
     * @param filePath Path to the file
     * @return true if successful or not needed (Windows), false on error
     */
    static bool makeExecutable(const std::string& filePath);

    /**
     * Get the appropriate executable name for the current platform.
     * @param baseName Base name without extension
     * @return baseName + ".exe" on Windows, baseName on Unix
     */
    static std::string getExecutableName(const std::string& baseName);

    /**
     * Get the appropriate executable name for a specific platform.
     * @param baseName Base name without extension
     * @param targetPlatform Target platform ("windows", "macos", "linux")
     * @return baseName + ".exe" for Windows, baseName for Unix
     */
    static std::string getExecutableName(const std::string& baseName, const std::string& targetPlatform);

    // Process discovery
    /**
     * Find an executable in the system PATH.
     * @param executableName Name of executable to find
     * @return Full path to executable, or empty string if not found
     */
    static std::string findExecutableInPath(const std::string& executableName);

private:
    // Helper functions
    static std::string executeCommand(const std::string& command);
    static std::string trim(const std::string& str);
};

} // namespace tsuki