#pragma once

#include "command_base.hpp"

namespace tsuki::cli {

class FuseCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "--fuse"; }
    std::string getDescription() const override { return "Create a standalone executable"; }
};

class FuseAllCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "--fuse-all"; }
    std::string getDescription() const override { return "Create executables for all platforms"; }
};

} // namespace tsuki::cli