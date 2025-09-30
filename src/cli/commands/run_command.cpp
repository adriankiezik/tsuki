#include "run_command.hpp"
#include "../utils/cli_utils.hpp"
#include <tsuki/tsuki.hpp>
#include <tsuki/packaging.hpp>
#include <filesystem>
#include <iostream>

namespace tsuki::cli {

int RunCommand::execute(int argc, char* argv[]) {
    // No arguments - check if this is a fused executable
    if (argc == 1) {
        return runFusedExecutable(argv[0]);
    }

    std::string game_path = argv[1];

    // Auto-detect .tsuki files
    if (!endsWith(game_path, ".tsuki") &&
        !std::filesystem::exists(game_path) &&
        std::filesystem::exists(game_path + ".tsuki")) {
        game_path += ".tsuki";
        std::cout << "Auto-detected .tsuki file: " << game_path << std::endl;
    }

    // Check if it's a .tsuki file
    if (endsWith(game_path, ".tsuki")) {
        std::cout << "Loading .tsuki file: " << game_path << std::endl;

        TempDirectoryGuard temp_guard("tsuki");
        if (!tsuki::Packaging::extractTsukiFile(game_path, temp_guard.path())) {
            std::cerr << "Failed to extract .tsuki file" << std::endl;
            return 1;
        }

        return runGame(temp_guard.path());
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

int RunCommand::runGame(const std::string& game_path) {
    auto& engine = tsuki::Engine::getInstance();
    if (!engine.init()) {
        std::cerr << "Failed to initialize Tsuki!" << std::endl;
        return 1;
    }

    engine.runLuaGame(game_path);
    return 0;
}

int RunCommand::runFusedExecutable(const char* argv0) {
    if (tsuki::Packaging::isFusedExecutable(argv0)) {
        std::cout << "Detected embedded game in executable" << std::endl;

        TempDirectoryGuard temp_guard("tsuki_fused");
        if (!tsuki::Packaging::extractFromFusedExecutable(argv0, temp_guard.path())) {
            std::cerr << "Failed to extract embedded game!" << std::endl;
            return 1;
        }

        return runGame(temp_guard.path());
    }

    std::cerr << "No game specified. Use --help for usage information." << std::endl;
    return 1;
}

} // namespace tsuki::cli