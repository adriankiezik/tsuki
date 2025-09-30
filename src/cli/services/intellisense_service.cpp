#include "intellisense_service.hpp"
#include "definitions_generator.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

namespace tsuki::cli {

bool IntelliSenseService::setup(const std::string& project_dir, bool skip) {
    if (skip) {
        return true;
    }

    if (!generateDefinitions(project_dir)) {
        std::cout << "Warning: Failed to generate IntelliSense definitions" << std::endl;
        std::cout << "You can manually run: tsuki generate-definitions" << std::endl;
        return true; // Don't fail project creation
    }

    return createLuarcConfig(project_dir);
}

bool IntelliSenseService::generateDefinitions(const std::string& project_dir) {
    try {
        DefinitionsGenerator generator;
        std::string output_path = project_dir + "/definitions.lua";
        return generator.saveToFile(output_path);
    } catch (const std::exception& e) {
        spdlog::error("Failed to generate definitions: {}", e.what());
        return false;
    }
}

bool IntelliSenseService::createLuarcConfig(const std::string& project_dir) {
    std::string config_content = R"({
    "Lua.runtime.version": "Lua 5.1",
    "Lua.diagnostics.globals": ["tsuki", "graphics", "window", "keyboard", "mouse", "debug"],
    "Lua.workspace.checkThirdParty": false,
    "Lua.workspace.library": ["definitions.lua"]
})";

    std::string config_path = project_dir + "/.luarc.json";
    std::ofstream config_file(config_path);
    if (!config_file) {
        std::cout << "Warning: Failed to create .luarc.json" << std::endl;
        return true;
    }

    config_file << config_content;
    config_file.close();

    if (!std::filesystem::exists(config_path)) {
        std::cout << "Warning: .luarc.json file was not created" << std::endl;
    }

    return true;
}


} // namespace tsuki::cli