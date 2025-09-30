#include "generate_definitions_command.hpp"
#include "../services/definitions_generator.hpp"
#include <iostream>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace tsuki::cli {

int GenerateDefinitionsCommand::execute(int argc, char* argv[]) {
    std::string output_path = "tsuki-definitions.lua";

    // Parse optional output path
    if (argc >= 3) {
        output_path = argv[2];
    }

    std::cout << "Generating Lua type definitions..." << std::endl;

    DefinitionsGenerator generator;
    if (!generator.saveToFile(output_path)) {
        std::cerr << "Error: Failed to generate definitions" << std::endl;
        return 1;
    }

    std::filesystem::path abs_path = std::filesystem::absolute(output_path);
    std::cout << "✓ Definitions generated: " << abs_path << std::endl;
    std::cout << "\nTo use IntelliSense, add to your .luarc.json:" << std::endl;
    std::cout << "{\n";
    std::cout << "    \"Lua.runtime.version\": \"Lua 5.4\",\n";
    std::cout << "    \"Lua.diagnostics.globals\": [\"tsuki\"],\n";
    std::cout << "    \"Lua.workspace.checkThirdParty\": false,\n";
    std::cout << "    \"Lua.workspace.library\": [\"" << std::filesystem::path(output_path).filename().string() << "\"]\n";
    std::cout << "}\n";

    return 0;
}

} // namespace tsuki::cli