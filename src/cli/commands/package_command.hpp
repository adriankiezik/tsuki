#pragma once

#include "command_base.hpp"

namespace tsuki::cli {

class PackageCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "--package"; }
    std::string getDescription() const override { return "Create a .tsuki package file"; }
};

} // namespace tsuki::cli