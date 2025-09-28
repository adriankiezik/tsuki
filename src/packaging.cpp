#include "tsuki/packaging.hpp"
#include "tsuki/version.hpp"
#include <zip.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

namespace tsuki {

bool Packaging::createTsukiFile(const std::string& source_dir, const std::string& output_file) {
    std::cout << "Creating .tsuki file: " << output_file << " from " << source_dir << std::endl;

    if (!std::filesystem::exists(source_dir)) {
        std::cerr << "Error: Source directory does not exist: " << source_dir << std::endl;
        return false;
    }

    if (!std::filesystem::exists(source_dir + "/main.lua")) {
        std::cerr << "Error: main.lua not found in " << source_dir << std::endl;
        return false;
    }

    return zipDirectory(source_dir, output_file);
}

bool Packaging::extractTsukiFile(const std::string& tsuki_file, const std::string& output_dir) {
    std::cout << "Extracting .tsuki file: " << tsuki_file << " to " << output_dir << std::endl;

    if (!std::filesystem::exists(tsuki_file)) {
        std::cerr << "Error: .tsuki file does not exist: " << tsuki_file << std::endl;
        return false;
    }

    return unzipFile(tsuki_file, output_dir);
}

bool Packaging::createStandaloneExecutable(const std::string& engine_path,
                                          const std::string& tsuki_file,
                                          const std::string& output_path) {
    std::cout << "Creating standalone executable: " << output_path << std::endl;
    std::cout << "  Engine: " << engine_path << std::endl;
    std::cout << "  Game: " << tsuki_file << std::endl;

    // Read engine executable
    std::ifstream engine(engine_path, std::ios::binary);
    if (!engine) {
        std::cerr << "Error: Cannot read engine executable: " << engine_path << std::endl;
        return false;
    }

    // Read .tsuki file
    std::ifstream game(tsuki_file, std::ios::binary);
    if (!game) {
        std::cerr << "Error: Cannot read .tsuki file: " << tsuki_file << std::endl;
        return false;
    }

    // Create output file
    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        std::cerr << "Error: Cannot create output file: " << output_path << std::endl;
        return false;
    }

    // Read engine into memory
    std::string engine_content((std::istreambuf_iterator<char>(engine)), std::istreambuf_iterator<char>());
    
    // Read game into memory
    game.seekg(0, std::ios::end);
    uint64_t game_size = game.tellg();
    game.seekg(0, std::ios::beg);
    std::string game_content((std::istreambuf_iterator<char>(game)), std::istreambuf_iterator<char>());


    // Write engine
    output.write(engine_content.data(), engine_content.size());

    // Add separator
    const char separator[] = "---TSUKI-GAME-BOUNDARY---";
    output.write(separator, strlen(separator));

    // Write game size as binary (little-endian)
    output.write(reinterpret_cast<const char*>(&game_size), sizeof(game_size));

    // Write game content
    output.write(game_content.data(), game_content.size());

    engine.close();
    game.close();
    output.close();

    // Make executable on Unix systems
#ifndef _WIN32
    std::filesystem::permissions(output_path,
        std::filesystem::perms::owner_all |
        std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
        std::filesystem::perms::others_read | std::filesystem::perms::others_exec);
#endif

    std::cout << "Standalone executable created successfully!" << std::endl;
    return true;
}

bool Packaging::isFusedExecutable(const std::string& executable_path) {
    std::ifstream file(executable_path, std::ios::binary);
    if (!file) return false;

    // Search for the boundary marker
    const char separator[] = "---TSUKI-GAME-BOUNDARY---";
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content.find(separator) != std::string::npos;
}

bool Packaging::extractFromFusedExecutable(const std::string& executable_path,
                                          const std::string& output_dir) {
    std::ifstream file(executable_path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot read executable: " << executable_path << std::endl;
        return false;
    }

    // Read entire file
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Find boundary (search from end to avoid collision with engine's embedded strings)
    const char separator[] = "---TSUKI-GAME-BOUNDARY---";
    size_t boundary_pos = content.rfind(separator);
    
    // Additional safety: verify this is actually our boundary by checking the structure
    if (boundary_pos != std::string::npos) {
        size_t expected_game_start = boundary_pos + strlen(separator) + sizeof(uint64_t);
        // Verify we have enough bytes for the structure and check ZIP header
        if (expected_game_start + 4 <= content.size()) {
            // Check if what follows looks like a ZIP file (starts with "PK")
            if (content[expected_game_start] != 'P' || content[expected_game_start + 1] != 'K') {
                // This might be a false boundary - search for the previous occurrence
                size_t search_end = boundary_pos > 0 ? boundary_pos - 1 : 0;
                if (search_end > 0) {
                    std::string search_content = content.substr(0, search_end + 1);
                    boundary_pos = search_content.rfind(separator);
                }
            }
        }
    }
    if (boundary_pos == std::string::npos) {
        std::cerr << "Error: No embedded game found in executable" << std::endl;
        return false;
    }

    // Position after boundary marker and size
    size_t game_start = boundary_pos + strlen(separator) + sizeof(uint64_t);

    // Extract game size
    uint64_t game_size;
    memcpy(&game_size, content.data() + boundary_pos + strlen(separator), sizeof(game_size));

    // Extract game data
    if (game_start + game_size > content.size()) {
        std::cerr << "Error: Invalid game data in executable" << std::endl;
        return false;
    }

    // Create temporary .tsuki file
    std::string temp_tsuki = output_dir + "/temp.tsuki";
    std::filesystem::create_directories(output_dir);

    std::ofstream temp_file(temp_tsuki, std::ios::binary);
    if (!temp_file) {
        std::cerr << "Error: Cannot create temporary file: " << temp_tsuki << std::endl;
        return false;
    }

    temp_file.write(content.data() + game_start, game_size);
    temp_file.close();

    // Extract the .tsuki file
    bool result = unzipFile(temp_tsuki, output_dir);

    // Clean up temporary file
    std::filesystem::remove(temp_tsuki);

    return result;
}

bool Packaging::zipDirectory(const std::string& source_dir, const std::string& zip_file) {
    int error = 0;
    zip_t* archive = zip_open(zip_file.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!archive) {
        zip_error_t zip_error;
        zip_error_init_with_code(&zip_error, error);
        std::cerr << "Error creating ZIP file: " << zip_error_strerror(&zip_error) << std::endl;
        zip_error_fini(&zip_error);
        return false;
    }

    auto files = getDirectoryFiles(source_dir);
    for (const auto& file_path : files) {
        std::string relative_path = std::filesystem::relative(file_path, source_dir).string();

        // Skip hidden files and directories
        if (relative_path.find("/.") != std::string::npos || relative_path[0] == '.') {
            continue;
        }

        if (std::filesystem::is_regular_file(file_path)) {
            zip_source_t* source = zip_source_file(archive, file_path.c_str(), 0, 0);
            if (!source) {
                std::cerr << "Error adding file to ZIP: " << file_path << std::endl;
                zip_close(archive);
                return false;
            }

            if (zip_file_add(archive, relative_path.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                std::cerr << "Error adding file to ZIP: " << relative_path << std::endl;
                zip_source_free(source);
                zip_close(archive);
                return false;
            }
        }
    }

    if (zip_close(archive) < 0) {
        std::cerr << "Error closing ZIP file" << std::endl;
        return false;
    }

    std::cout << "Successfully created " << zip_file << std::endl;
    return true;
}

bool Packaging::unzipFile(const std::string& zip_file, const std::string& output_dir) {
    int error = 0;
    zip_t* archive = zip_open(zip_file.c_str(), ZIP_RDONLY, &error);
    if (!archive) {
        zip_error_t zip_error;
        zip_error_init_with_code(&zip_error, error);
        std::cerr << "Error opening ZIP file: " << zip_error_strerror(&zip_error) << std::endl;
        zip_error_fini(&zip_error);
        return false;
    }

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    if (num_entries < 0) {
        std::cerr << "Error getting ZIP entries" << std::endl;
        zip_close(archive);
        return false;
    }

    std::filesystem::create_directories(output_dir);

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(archive, i, 0);
        if (!name) {
            std::cerr << "Error getting entry name" << std::endl;
            continue;
        }

        std::string output_path = output_dir + "/" + name;

        // Create directory if needed
        std::filesystem::create_directories(std::filesystem::path(output_path).parent_path());

        // Skip directories
        if (name[strlen(name) - 1] == '/') {
            continue;
        }

        zip_file_t* file = zip_fopen_index(archive, i, 0);
        if (!file) {
            std::cerr << "Error opening ZIP entry: " << name << std::endl;
            continue;
        }

        std::ofstream output_file(output_path, std::ios::binary);
        if (!output_file) {
            std::cerr << "Error creating output file: " << output_path << std::endl;
            zip_fclose(file);
            continue;
        }

        char buffer[8192];
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            output_file.write(buffer, bytes_read);
        }

        output_file.close();
        zip_fclose(file);
    }

    zip_close(archive);
    std::cout << "Successfully extracted to " << output_dir << std::endl;
    return true;
}

std::vector<std::string> Packaging::getDirectoryFiles(const std::string& directory, bool recursive) {
    std::vector<std::string> files;

    if (!std::filesystem::exists(directory)) {
        return files;
    }

    if (recursive) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    }

    return files;
}

// Cross-platform standalone executable creation
bool Packaging::createStandaloneExecutable(const std::string& engine_path,
                                          const std::string& tsuki_file,
                                          const std::string& output_path,
                                          const std::string& target_platform,
                                          const std::string& target_arch) {

    // Use local engine for same-platform builds
    std::string current_platform;
#ifdef __APPLE__
    current_platform = "macos";
#elif defined(_WIN32)
    current_platform = "windows";
#else
    current_platform = "linux";
#endif

    if (target_platform == current_platform) {
        // Use local engine for same-platform builds
        return createStandaloneExecutable(engine_path, tsuki_file, output_path);
    }

    // For local development, you can disable cross-compilation
    const char* disable_cross = getenv("TSUKI_DISABLE_CROSS_COMPILATION");
    if (disable_cross && std::string(disable_cross) == "1") {
        std::cerr << "Cross-compilation disabled by TSUKI_DISABLE_CROSS_COMPILATION=1" << std::endl;
        std::cerr << "Building for current platform only (" << current_platform << ")" << std::endl;
        return false;
    }

    std::cout << "Preparing cross-platform build for " << target_platform << " (" << target_arch << ")" << std::endl;

    // Get or download the target platform engine
    std::string target_engine_path = getEngineBinaryPath(target_platform, target_arch);
    if (target_engine_path.empty()) {
        std::cerr << "Failed to obtain " << target_platform << " (" << target_arch << ") engine binary" << std::endl;
        std::cerr << "This could mean:" << std::endl;
        std::cerr << "  - No release exists for this platform/architecture combination" << std::endl;
        std::cerr << "  - Network connectivity issues" << std::endl;
        std::cerr << "  - Invalid platform/architecture specified" << std::endl;
        return false;
    }

    return createStandaloneExecutable(target_engine_path, tsuki_file, output_path);
}

std::string Packaging::getCacheDirectory() {
    const char* home = getenv("HOME");
    if (!home) {
        return "/tmp/tsuki_cache";
    }

    std::string cache_dir = std::string(home) + "/.cache/tsuki";
    std::filesystem::create_directories(cache_dir);
    return cache_dir;
}

bool Packaging::isDevelopmentBuild() {
    // This function is no longer used for blocking cross-compilation
    // Kept for potential future use
    return false;
}

std::string Packaging::getBinaryUrl(const std::string& platform, const std::string& arch) {
    // Allow cross-compilation for all versions - let download failures handle missing releases

    // This can be overridden by environment variable TSUKI_RELEASES_URL
    const char* custom_url = getenv("TSUKI_RELEASES_URL");

    std::string base_url;
    if (custom_url) {
        base_url = std::string(custom_url);
    } else {
        // Use specific release tag instead of /latest/ to avoid version mismatches
        base_url = "https://github.com/adriankiezik/tsuki/releases/download/v" + 
                   std::string(TSUKI_VERSION) + "/";
    }

    return base_url + "tsuki-v" + std::string(TSUKI_VERSION) + "-" + platform + "-" + arch + ".zip";
}

std::string Packaging::getCachedBinaryPath(const std::string& platform, const std::string& arch) {
    std::string cache_dir = getCacheDirectory();
    return cache_dir + "/tsuki-v" + std::string(TSUKI_VERSION) + "-" + platform + "-" + arch + ".zip";
}

bool Packaging::downloadBinary(const std::string& url, const std::string& output_path) {
    std::cout << "Downloading binary from: " << url << std::endl;
    std::cout << "Saving to: " << output_path << std::endl;

    // Create directory if it doesn't exist
    std::filesystem::create_directories(std::filesystem::path(output_path).parent_path());

    // Use curl to download the file
    std::string curl_cmd = "curl -L -o \"" + output_path + "\" \"" + url + "\"";
    int result = system(curl_cmd.c_str());

    if (result != 0) {
        std::cerr << "Failed to download binary. curl returned: " << result << std::endl;
        std::cerr << "Make sure curl is installed and you have internet connection." << std::endl;
        return false;
    }

    // Verify the file was downloaded
    if (!std::filesystem::exists(output_path)) {
        std::cerr << "Download failed: file not found at " << output_path << std::endl;
        return false;
    }

    std::cout << "Successfully downloaded binary" << std::endl;
    return true;
}

bool Packaging::extractBinaryBundle(const std::string& bundle_path, const std::string& extract_dir) {
    std::cout << "Extracting binary bundle to: " << extract_dir << std::endl;

    // Create extraction directory
    std::filesystem::create_directories(extract_dir);

    // Extract using the existing unzipFile method
    return unzipFile(bundle_path, extract_dir);
}

std::string Packaging::getEngineBinaryPath(const std::string& platform, const std::string& arch) {
    std::string cache_dir = getCacheDirectory();
    std::string bundle_path = getCachedBinaryPath(platform, arch);
    std::string extract_dir = cache_dir + "/extracted/" + platform + "-" + arch;
    std::string engine_path = extract_dir + "/tsuki";

    if (platform == "windows") {
        engine_path += ".exe";
    }

    // Check if we already have the extracted binary
    if (std::filesystem::exists(engine_path)) {
        std::cout << "Using cached " << platform << " engine: " << engine_path << std::endl;
        return engine_path;
    }

    // Check if we have the bundle cached
    if (!std::filesystem::exists(bundle_path)) {
        std::cout << "Downloading " << platform << " (" << arch << ") engine binary..." << std::endl;
        std::string url = getBinaryUrl(platform, arch);

        if (!downloadBinary(url, bundle_path)) {
            std::cerr << "Failed to download engine binary from: " << url << std::endl;
            std::cerr << "This could mean:" << std::endl;
            std::cerr << "  - No release exists for version " << TSUKI_VERSION << std::endl;
            std::cerr << "  - No " << platform << "-" << arch << " build available for this version" << std::endl;
            std::cerr << "  - Network connectivity issues" << std::endl;
            std::cerr << "Tip: Set TSUKI_DISABLE_CROSS_COMPILATION=1 to build for current platform only" << std::endl;
            return "";
        }
    }

    // Extract the bundle
    if (!extractBinaryBundle(bundle_path, extract_dir)) {
        std::cerr << "Failed to extract binary bundle" << std::endl;
        return "";
    }

    // Verify the engine binary exists
    if (!std::filesystem::exists(engine_path)) {
        std::cerr << "Engine binary not found in bundle: " << engine_path << std::endl;
        return "";
    }

    // Make executable on Unix systems
#ifndef _WIN32
    std::filesystem::permissions(engine_path,
        std::filesystem::perms::owner_all |
        std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
        std::filesystem::perms::others_read | std::filesystem::perms::others_exec);
#endif

    std::cout << "Successfully prepared " << platform << " engine: " << engine_path << std::endl;
    return engine_path;
}

} // namespace tsuki