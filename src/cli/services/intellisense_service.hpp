#pragma once

#include <string>

namespace tsuki::cli {

class IntelliSenseService {
public:
    bool setup(const std::string& project_dir, bool skip = false);

private:
    bool generateDefinitions(const std::string& project_dir);
    bool createLuarcConfig(const std::string& project_dir);
};

} // namespace tsuki::cli