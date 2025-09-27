#pragma once

#include <string>
#include <vector>

namespace tsuki {

class Packaging {
public:
    // Create .tsuki file from directory
    static bool createTsukiFile(const std::string& source_dir, const std::string& output_file);

    // Extract .tsuki file to directory
    static bool extractTsukiFile(const std::string& tsuki_file, const std::string& output_dir);

    // Create standalone executable by fusing engine + .tsuki file
    static bool createStandaloneExecutable(const std::string& engine_path,
                                          const std::string& tsuki_file,
                                          const std::string& output_path);

    // Create cross-platform standalone executable
    static bool createStandaloneExecutable(const std::string& engine_path,
                                          const std::string& tsuki_file,
                                          const std::string& output_path,
                                          const std::string& target_platform,
                                          const std::string& target_arch);

    // Check if a file is a fused executable (contains embedded .tsuki)
    static bool isFusedExecutable(const std::string& executable_path);

    // Extract embedded .tsuki from fused executable
    static bool extractFromFusedExecutable(const std::string& executable_path,
                                          const std::string& output_dir);

private:
    static bool zipDirectory(const std::string& source_dir, const std::string& zip_file);
    static bool unzipFile(const std::string& zip_file, const std::string& output_dir);
    static std::vector<std::string> getDirectoryFiles(const std::string& directory, bool recursive = true);

    // Cross-platform support
    static std::string getCacheDirectory();
    static std::string getBinaryUrl(const std::string& platform, const std::string& arch);
    static std::string getCachedBinaryPath(const std::string& platform, const std::string& arch);
    static bool downloadBinary(const std::string& url, const std::string& output_path);
    static bool extractBinaryBundle(const std::string& bundle_path, const std::string& extract_dir);
    static std::string getEngineBinaryPath(const std::string& platform, const std::string& arch);
};

} // namespace tsuki