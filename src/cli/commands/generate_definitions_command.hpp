#pragma once

#include "command_base.hpp"

namespace tsuki::cli {

class GenerateDefinitionsCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "generate-definitions"; }
    std::string getDescription() const override { return "Generate Lua type definitions for IntelliSense"; }
};

} // namespace tsuki::cli