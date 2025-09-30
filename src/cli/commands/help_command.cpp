#include "help_command.hpp"
#include <tsuki/version.hpp>
#include <iostream>

namespace tsuki::cli {

HelpCommand::HelpCommand(const char* program_name)
    : program_name_(program_name) {}

int HelpCommand::execute(int argc, char* argv[]) {
    (void)argc; (void)argv;
    printUsage();
    return 0;
}

void HelpCommand::printUsage() {
    std::cout << tsuki::PROJECT_NAME << " Game Engine v" << tsuki::VERSION << "\n";
    std::cout << "Usage:\n\n";

    std::cout << "  Creating projects:\n";
    std::cout << "    " << program_name_ << " new <name>                              Create new game project\n";
    std::cout << "    " << program_name_ << " new <name> --template minimal           Create minimal project\n";
    std::cout << "    " << program_name_ << " new <name> --no-intellisense            Skip IntelliSense setup\n\n";

    std::cout << "  Running games:\n";
    std::cout << "    " << program_name_ << " <game_directory>     Run a game from directory\n";
    std::cout << "    " << program_name_ << " <game.tsuki>        Run a .tsuki game file\n";
    std::cout << "    " << program_name_ << "                     Run if executable contains embedded game\n\n";

    std::cout << "  Packaging:\n";
    std::cout << "    " << program_name_ << " --package <dir> <output.tsuki>          Create .tsuki file from directory\n";
    std::cout << "    " << program_name_ << " --fuse <game.tsuki> <output>            Create standalone executable\n";
    std::cout << "    " << program_name_ << " --fuse <game.tsuki> <output> --target windows  Create Windows executable from Linux\n";
    std::cout << "    " << program_name_ << " --fuse-all <game.tsuki> <prefix>        Create executables for all platforms\n\n";

    std::cout << "  Other:\n";
    std::cout << "    " << program_name_ << " --help              Show this help\n";
    std::cout << "    " << program_name_ << " --version           Show version\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program_name_ << " new mygame                           # Create new project with starter template\n";
    std::cout << "  " << program_name_ << " new mygame --template minimal        # Create minimal project\n";
    std::cout << "  " << program_name_ << " mygame/                              # Run game from directory\n";
    std::cout << "  " << program_name_ << " mygame.tsuki                         # Run packaged game\n";
    std::cout << "  " << program_name_ << " mygame                               # Auto-detect mygame.tsuki\n";
    std::cout << "  " << program_name_ << " --package mygame/ mygame             # Create mygame.tsuki (auto-append extension)\n";
    std::cout << "  " << program_name_ << " --fuse mygame.tsuki mygame_linux     # Create Linux standalone game\n";
    std::cout << "  " << program_name_ << " --fuse mygame.tsuki mygame.exe --target windows  # Create Windows .exe from Linux\n";
    std::cout << "  " << program_name_ << " --fuse-all mygame.tsuki mygame       # Create mygame-linux, mygame-windows.exe, mygame-macos\n";
}

int VersionCommand::execute(int argc, char* argv[]) {
    (void)argc; (void)argv;
    std::cout << tsuki::PROJECT_NAME << " Game Engine v" << tsuki::VERSION << "\n";
    std::cout << "Built with C++23, SDL3, and Lua 5.4\n";
    std::cout << "Packaging support with libzip\n";
    return 0;
}

} // namespace tsuki::cli