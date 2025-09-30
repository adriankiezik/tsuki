#include "new_command.hpp"
#include <iostream>

namespace tsuki::cli {

int NewCommand::execute(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " new <project_name> [--template starter|minimal] [--no-intellisense]" << std::endl;
        return 1;
    }

    ProjectOptions options;
    options.name = argv[2];
    options.template_type = TemplateType::Starter;
    options.skip_intellisense = false;

    // Parse additional arguments
    for (int i = 3; i < argc; i++) {
        std::string current_arg = argv[i];
        if (current_arg == "--template" && i + 1 < argc) {
            std::string template_str = argv[++i];
            if (template_str == "starter") {
                options.template_type = TemplateType::Starter;
            } else if (template_str == "minimal") {
                options.template_type = TemplateType::Minimal;
            } else {
                std::cerr << "Error: Invalid template '" << template_str << "'. Valid options: starter, minimal" << std::endl;
                return 1;
            }
        } else if (current_arg == "--no-intellisense") {
            options.skip_intellisense = true;
        } else {
            std::cerr << "Error: Unknown option '" << current_arg << "'" << std::endl;
            return 1;
        }
    }

    return project_service_.createProject(options);
}

} // namespace tsuki::cli