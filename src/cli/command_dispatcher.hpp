#pragma once

#include "commands/command_base.hpp"
#include <memory>
#include <unordered_map>
#include <string>

namespace tsuki::cli {

class CommandDispatcher {
public:
    void registerCommand(const std::string& name, std::unique_ptr<Command> command);
    int dispatch(int argc, char* argv[]);

private:
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
    const char* program_name_;
};

} // namespace tsuki::cli