#pragma once

#include "command_base.hpp"
#include "../services/project_service.hpp"

namespace tsuki::cli {

class NewCommand : public Command {
public:
    int execute(int argc, char* argv[]) override;
    std::string getName() const override { return "new"; }
    std::string getDescription() const override { return "Create a new game project"; }

private:
    ProjectService project_service_;
};

} // namespace tsuki::cli