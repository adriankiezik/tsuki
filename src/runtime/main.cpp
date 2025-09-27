#include <tsuki/tsuki.hpp>
#include <tsuki/packaging.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstring>
#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

void printUsage(const char* program_name) {
    std::cout << "Tsuki Game Engine v1.0.0\n";
    std::cout << "Usage:\n\n";

    std::cout << "  Running games:\n";
    std::cout << "    " << program_name << " <game_directory>     Run a game from directory\n";
    std::cout << "    " << program_name << " <game.tsuki>        Run a .tsuki game file\n";
    std::cout << "    " << program_name << "                     Run if executable contains embedded game\n\n";

    std::cout << "  Packaging:\n";
    std::cout << "    " << program_name << " --package <dir> <output.tsuki>          Create .tsuki file from directory\n";
    std::cout << "    " << program_name << " --fuse <game.tsuki> <output>            Create standalone executable\n";
    std::cout << "    " << program_name << " --fuse <game.tsuki> <output> --target windows  Create Windows executable from Linux\n\n";

    std::cout << "  Other:\n";
    std::cout << "    " << program_name << " --help              Show this help\n";
    std::cout << "    " << program_name << " --version           Show version\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " mygame/                              # Run game from directory\n";
    std::cout << "  " << program_name << " mygame.tsuki                         # Run packaged game\n";
    std::cout << "  " << program_name << " mygame                               # Auto-detect mygame.tsuki\n";
    std::cout << "  " << program_name << " --package mygame/ mygame             # Create mygame.tsuki (auto-append extension)\n";
    std::cout << "  " << program_name << " --fuse mygame.tsuki mygame_linux     # Create Linux standalone game\n";
    std::cout << "  " << program_name << " --fuse mygame.tsuki mygame.exe --target windows  # Create Windows .exe from Linux\n";
}

void printVersion() {
    std::cout << "Tsuki Game Engine v1.0.0\n";
    std::cout << "Built with C++23, SDL3, and Lua 5.4\n";
    std::cout << "Packaging support with libzip\n";
}

int runGame(const std::string& game_path) {
    // Initialize the engine
    auto& engine = tsuki::Engine::getInstance();
    if (!engine.init()) {
        std::cerr << "Failed to initialize Tsuki engine!" << std::endl;
        return 1;
    }

    std::cout << "Loading Tsuki game from: " << game_path << std::endl;

    // Run the Lua game
    engine.runLuaGame(game_path);

    std::cout << "Game finished!" << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    // No arguments - check if this is a fused executable
    if (argc == 1) {
        if (tsuki::Packaging::isFusedExecutable(argv[0])) {
            std::cout << "Detected embedded game in executable" << std::endl;

            // Extract to temporary directory
            std::string temp_dir = (std::filesystem::temp_directory_path() / ("tsuki_fused_" + std::to_string(getpid()))).string();
            if (!tsuki::Packaging::extractFromFusedExecutable(argv[0], temp_dir)) {
                std::cerr << "Failed to extract embedded game!" << std::endl;
                return 1;
            }

            int result = runGame(temp_dir);

            // Cleanup
            std::filesystem::remove_all(temp_dir);
            return result;
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    std::string arg = argv[1];

    // Handle command line options
    if (arg == "--help" || arg == "-h") {
        printUsage(argv[0]);
        return 0;
    }

    if (arg == "--version" || arg == "-v") {
        printVersion();
        return 0;
    }

    // Handle packaging command
    if (arg == "--package") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " --package <source_directory> <output.tsuki>" << std::endl;
            return 1;
        }

        std::string source_dir = argv[2];
        std::string output_file = argv[3];

        // Auto-append .tsuki extension if not present
        if (!output_file.ends_with(".tsuki")) {
            output_file += ".tsuki";
        }

        if (tsuki::Packaging::createTsukiFile(source_dir, output_file)) {
            std::cout << "Successfully created " << output_file << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to create .tsuki file" << std::endl;
            return 1;
        }
    }

    // Handle fusion command
    if (arg == "--fuse") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " --fuse <game.tsuki> <output_executable> [--target windows|linux] [--arch x64|x86]" << std::endl;
            return 1;
        }

        std::string tsuki_file = argv[2];
        std::string output_exe = argv[3];
        std::string target_platform = "linux";  // default to current platform
        std::string target_arch = "x64";        // default architecture

        // Parse additional arguments
        for (int i = 4; i < argc; i++) {
            std::string current_arg = argv[i];
            if (current_arg == "--target" && i + 1 < argc) {
                target_platform = argv[++i];
            } else if (current_arg == "--arch" && i + 1 < argc) {
                target_arch = argv[++i];
            }
        }

        // Validate target platform
        if (target_platform != "linux" && target_platform != "windows") {
            std::cerr << "Error: Unsupported target platform '" << target_platform << "'. Supported: linux, windows" << std::endl;
            return 1;
        }

        // Validate architecture
        if (target_arch != "x64" && target_arch != "x86") {
            std::cerr << "Error: Unsupported architecture '" << target_arch << "'. Supported: x64, x86" << std::endl;
            return 1;
        }

        std::cout << "Creating standalone executable for " << target_platform << " (" << target_arch << ")" << std::endl;

        if (tsuki::Packaging::createStandaloneExecutable(argv[0], tsuki_file, output_exe, target_platform, target_arch)) {
            std::cout << "Successfully created standalone executable: " << output_exe << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to create standalone executable" << std::endl;
            return 1;
        }
    }

    // Regular game running
    std::string game_path = arg;

    // Auto-detect .tsuki files: if path doesn't end with .tsuki, doesn't exist as directory, but path.tsuki exists, use that
    if (!game_path.ends_with(".tsuki") &&
        !std::filesystem::exists(game_path) &&
        std::filesystem::exists(game_path + ".tsuki")) {
        game_path += ".tsuki";
        std::cout << "Auto-detected .tsuki file: " << game_path << std::endl;
    }

    // Check if it's a .tsuki file
    if (game_path.ends_with(".tsuki")) {
        std::cout << "Loading .tsuki file: " << game_path << std::endl;

        // Create temporary directory for extraction
        std::string temp_dir = (std::filesystem::temp_directory_path() / ("tsuki_" + std::to_string(getpid()))).string();
        if (!tsuki::Packaging::extractTsukiFile(game_path, temp_dir)) {
            std::cerr << "Failed to extract .tsuki file" << std::endl;
            return 1;
        }

        int result = runGame(temp_dir);

        // Cleanup temporary directory
        std::filesystem::remove_all(temp_dir);
        return result;
    }

    // Check if the path exists
    if (!std::filesystem::exists(game_path)) {
        std::cerr << "Error: Game path '" << game_path << "' does not exist!" << std::endl;
        return 1;
    }

    // Check if it's a directory
    if (!std::filesystem::is_directory(game_path)) {
        std::cerr << "Error: '" << game_path << "' is not a directory!" << std::endl;
        return 1;
    }

    // Check if main.lua exists
    std::string main_lua = game_path + "/main.lua";
    if (!std::filesystem::exists(main_lua)) {
        std::cerr << "Error: No main.lua found in '" << game_path << "'!" << std::endl;
        std::cerr << "Tsuki games must have a main.lua file." << std::endl;
        return 1;
    }

    return runGame(game_path);
}