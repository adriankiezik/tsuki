#include "project_service.hpp"
#include "../utils/cli_utils.hpp"
#include <filesystem>
#include <iostream>

namespace tsuki::cli {

int ProjectService::createProject(const ProjectOptions& options) {
    // Validate project name
    if (options.name.empty()) {
        std::cerr << "Error: Project name cannot be empty" << std::endl;
        return 1;
    }

    if (!isValidProjectName(options.name)) {
        std::cerr << "Error: Invalid project name '" << options.name << "'" << std::endl;
        std::cerr << "Project names cannot contain: / \\ : * ? \" < > |" << std::endl;
        std::cerr << "Project names cannot be reserved Windows names (CON, PRN, AUX, etc.)" << std::endl;
        return 1;
    }

    // Check if directory already exists
    if (std::filesystem::exists(options.name)) {
        std::cerr << "Error: Directory '" << options.name << "' already exists!" << std::endl;
        return 1;
    }

    // Use RAII guard for automatic cleanup on failure
    ProjectGuard guard(options.name);

    // Create project directory
    try {
        std::filesystem::create_directory(options.name);
    } catch (const std::exception& e) {
        std::cerr << "Error: Cannot create directory '" << options.name << "': " << e.what() << std::endl;
        return 1;
    }

    // Create main.lua
    if (!template_service_.createMainLua(options.name, options.name, options.template_type)) {
        std::cerr << "Error: Failed to create main.lua" << std::endl;
        return 1;
    }

    // Set up IntelliSense
    if (!intellisense_service_.setup(options.name, options.skip_intellisense)) {
        std::cout << "Warning: IntelliSense setup incomplete" << std::endl;
    }

    // Success! Commit the guard so it doesn't cleanup
    guard.commit();

    std::cout << "\n✅ Project '" << options.name << "' created successfully!" << std::endl;
    std::cout << "\nNext steps:" << std::endl;
    std::cout << "  cd " << options.name << std::endl;
    std::cout << "  tsuki ." << std::endl;
    std::cout << "\nFor VSCode IntelliSense:" << std::endl;
    std::cout << "  1. Install the 'Lua' extension by sumneko" << std::endl;
    std::cout << "  2. Open the project folder in VSCode" << std::endl;
    std::cout << "  3. IntelliSense should work automatically!" << std::endl;

    return 0;
}

} // namespace tsuki::cli