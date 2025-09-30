#pragma once

#include <string>

namespace tsuki::cli {

enum class TemplateType {
    Minimal,
    Starter
};

class TemplateService {
public:
    bool createMainLua(const std::string& project_dir, const std::string& project_name,
                      TemplateType type);

private:
    std::string getMinimalTemplate(const std::string& project_name);
    bool copyStarterTemplate(const std::string& dest_path);
};

} // namespace tsuki::cli