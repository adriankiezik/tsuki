#include "fuse_command.hpp"
#include "../utils/cli_utils.hpp"
#include <tsuki/platform.hpp>
#include <tsuki/packaging.hpp>
#include <iostream>
#include <vector>

namespace tsuki::cli {

int FuseCommand::execute(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " --fuse <game.tsuki> <output_executable> [--target windows|linux|macos] [--arch x64|x86]" << std::endl;
        return 1;
    }

    std::string tsuki_file = argv[2];
    std::string output_exe = argv[3];
    std::string target_platform = tsuki::Platform::getCurrentPlatform();
    std::string target_arch = "x64";

    // Parse additional arguments
    for (int i = 4; i < argc; i++) {
        std::string current_arg = argv[i];
        if (current_arg == "--target" && i + 1 < argc) {
            target_platform = argv[++i];
        } else if (current_arg == "--arch" && i + 1 < argc) {
            target_arch = argv[++i];
        }
    }

    // Validate inputs
    if (!isValidPlatform(target_platform)) {
        std::cerr << "Error: Unsupported target platform '" << target_platform
                  << "'. Supported: linux, windows, macos" << std::endl;
        return 1;
    }

    if (!isValidArchitecture(target_arch)) {
        std::cerr << "Error: Unsupported architecture '" << target_arch
                  << "'. Supported: x64, x86" << std::endl;
        return 1;
    }

    std::cout << "Creating standalone executable for " << target_platform
              << " (" << target_arch << ")" << std::endl;

    std::string engine_path = resolveEnginePath(argv[0]);

    if (tsuki::Packaging::createStandaloneExecutable(engine_path, tsuki_file,
                                                     output_exe, target_platform, target_arch)) {
        std::cout << "Successfully created standalone executable: " << output_exe << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to create standalone executable" << std::endl;
        return 1;
    }
}

int FuseAllCommand::execute(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " --fuse-all <game.tsuki> <output_prefix>" << std::endl;
        return 1;
    }

    std::string tsuki_file = argv[2];
    std::string output_prefix = argv[3];

    std::cout << "Creating standalone executables for all platforms..." << std::endl;

    std::string engine_path = resolveEnginePath(argv[0]);

    // Define all platforms and their output names
    std::vector<std::pair<std::string, std::string>> platforms = {
        {"linux", output_prefix + "-linux"},
        {"windows", output_prefix + "-windows.exe"},
        {"macos", output_prefix + "-macos"}
    };

    bool all_successful = true;
    int successful_count = 0;

    for (const auto& [platform, output_name] : platforms) {
        std::cout << "\n=== Creating " << platform << " executable ===" << std::endl;

        if (tsuki::Packaging::createStandaloneExecutable(engine_path, tsuki_file,
                                                         output_name, platform, "x64")) {
            std::cout << "✓ Successfully created: " << output_name << std::endl;
            successful_count++;
        } else {
            std::cerr << "✗ Failed to create " << platform << " executable" << std::endl;
            all_successful = false;
        }
    }

    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Successfully created " << successful_count << " out of "
              << platforms.size() << " executables." << std::endl;

    if (all_successful) {
        std::cout << "All platform executables created successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Some executables failed to create. See errors above." << std::endl;
        return 1;
    }
}

} // namespace tsuki::cli