#pragma once

#include "command_base.hpp"

namespace tsuki::cli {

class RunCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "run"; }
    std::string getDescription() const override { return "Run a game"; }

private:
    int runGame(const std::string& game_path);
    int runFusedExecutable(const char* argv0);
};

} // namespace tsuki::cli