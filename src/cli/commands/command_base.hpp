#pragma once

#include <string>
#include <vector>

namespace tsuki::cli {

class Command {
public:
    virtual ~Command() = default;
    virtual int execute(int argc, char* argv[]) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
};

} // namespace tsuki::cli