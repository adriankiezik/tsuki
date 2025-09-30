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
    std::cout << tsuki::PROJECT_NAME << " v" << tsuki::VERSION << "\n \n";

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
}

int VersionCommand::execute(int argc, char* argv[]) {
    (void)argc; (void)argv;
    std::cout << tsuki::PROJECT_NAME << " v" << tsuki::VERSION << std::endl;
    return 0;
}

}