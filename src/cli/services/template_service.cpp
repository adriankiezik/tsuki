#include "template_service.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace tsuki::cli {

bool TemplateService::createMainLua(const std::string& project_dir,
                                    const std::string& project_name,
                                    TemplateType type) {
    std::string main_lua_path = project_dir + "/main.lua";

    if (type == TemplateType::Starter) {
        if (copyStarterTemplate(main_lua_path)) {
            std::cout << "✓ Created main.lua from starter template" << std::endl;
            return true;
        }
        std::cout << "Warning: Starter template not found, using minimal template" << std::endl;
    }

    // Create minimal template
    std::ofstream main_file(main_lua_path);
    if (!main_file) {
        std::cerr << "Error: Cannot create main.lua" << std::endl;
        return false;
    }

    main_file << getMinimalTemplate(project_name);
    std::cout << "✓ Created main.lua from minimal template" << std::endl;
    return true;
}

std::string TemplateService::getMinimalTemplate(const std::string& project_name) {
    return R"(function tsuki.load()
    -- Called once when the game starts
    window.setTitle(")" + project_name + R"(")
end

function tsuki.update(dt)
    -- Called every frame for game logic
end

function tsuki.draw()
    -- Called every frame for rendering
    graphics.clear(0.1, 0.1, 0.2, 1.0)

    graphics.setColor(1.0, 1.0, 1.0, 1.0)
    graphics.print("Welcome to Tsuki!", window.getWidth()/2, window.getHeight()/2 - 20, "center")
    graphics.print("Edit main.lua to start building your game", window.getWidth()/2, window.getHeight()/2 + 20, "center")
end
)";
}

bool TemplateService::copyStarterTemplate(const std::string& dest_path) {
    std::string starter_path = "examples/starter/main.lua";
    if (!std::filesystem::exists(starter_path)) {
        return false;
    }

    try {
        std::filesystem::copy_file(starter_path, dest_path,
                                   std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying starter template: " << e.what() << std::endl;
        return false;
    }
}

} // namespace tsuki::cli