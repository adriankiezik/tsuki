#pragma once

#include "command_base.hpp"

namespace tsuki::cli {

class HelpCommand : public Command {
public:
    explicit HelpCommand(const char* program_name);
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "--help"; }
    std::string getDescription() const override { return "Show help information"; }

private:
    void printUsage();
    std::string program_name_;
};

class VersionCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "--version"; }
    std::string getDescription() const override { return "Show version information"; }
};

} // namespace tsuki::cli