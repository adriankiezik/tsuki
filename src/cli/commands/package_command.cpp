#include "package_command.hpp"
#include "../utils/cli_utils.hpp"
#include <tsuki/packaging.hpp>
#include <iostream>

namespace tsuki::cli {

int PackageCommand::execute(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " --package <source_directory> <output.tsuki>" << std::endl;
        return 1;
    }

    std::string source_dir = argv[2];
    std::string output_file = autoAppendExtension(argv[3], ".tsuki");

    if (tsuki::Packaging::createTsukiFile(source_dir, output_file)) {
        std::cout << "Successfully created " << output_file << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to create .tsuki file" << std::endl;
        return 1;
    }
}

} // namespace tsuki::cli