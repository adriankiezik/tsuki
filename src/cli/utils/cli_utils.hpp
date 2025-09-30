#pragma once

#include <string>
#include <vector>
#include <optional>

namespace tsuki::cli {

// RAII guard for temporary directories
class TempDirectoryGuard {
public:
    explicit TempDirectoryGuard(const std::string& prefix = "tsuki");
    ~TempDirectoryGuard();

    // Disable copy, allow move
    TempDirectoryGuard(const TempDirectoryGuard&) = delete;
    TempDirectoryGuard& operator=(const TempDirectoryGuard&) = delete;
    TempDirectoryGuard(TempDirectoryGuard&&) noexcept;
    TempDirectoryGuard& operator=(TempDirectoryGuard&&) noexcept;

    const std::string& path() const { return path_; }
    void keep() { should_cleanup_ = false; }

private:
    std::string path_;
    bool should_cleanup_;
};

// RAII guard for project creation (cleanup on failure)
class ProjectGuard {
public:
    explicit ProjectGuard(std::string project_dir);
    ~ProjectGuard();

    ProjectGuard(const ProjectGuard&) = delete;
    ProjectGuard& operator=(const ProjectGuard&) = delete;

    void commit() { committed_ = true; }
    const std::string& path() const { return project_dir_; }

private:
    std::string project_dir_;
    bool committed_;
};

// Input validation
bool isValidProjectName(const std::string& name);
bool isValidPlatform(const std::string& platform);
bool isValidArchitecture(const std::string& arch);

// Path utilities
std::string getPlatformExecutableDir();
std::string resolveEnginePath(const char* argv0);

// Safe system operations
int safeCurlDownload(const std::string& url, const std::string& output_path, bool headers_only = false);

// String utilities
bool endsWith(const std::string& str, const std::string& suffix);
std::string autoAppendExtension(const std::string& path, const std::string& extension);

} // namespace tsuki::cli