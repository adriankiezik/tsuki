#include "command_dispatcher.hpp"
#include "commands/run_command.hpp"

namespace tsuki::cli {

void CommandDispatcher::registerCommand(const std::string& name, std::unique_ptr<Command> command) {
    commands_[name] = std::move(command);
}

int CommandDispatcher::dispatch(int argc, char* argv[]) {
    program_name_ = argv[0];

    // No arguments - try to run fused executable
    if (argc == 1) {
        auto run_cmd = std::make_unique<RunCommand>();
        return run_cmd->execute(argc, argv);
    }

    // Check for command
    std::string command_name = argv[1];

    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        return it->second->execute(argc, argv);
    }

    // Not a registered command, treat as game path
    auto run_cmd = std::make_unique<RunCommand>();
    return run_cmd->execute(argc, argv);
}

} // namespace tsuki::cli