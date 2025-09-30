#pragma once

#include "template_service.hpp"
#include "intellisense_service.hpp"
#include <string>

namespace tsuki::cli {

struct ProjectOptions {
    std::string name;
    TemplateType template_type = TemplateType::Starter;
    bool skip_intellisense = false;
};

class ProjectService {
public:
    int createProject(const ProjectOptions& options);

private:
    TemplateService template_service_;
    IntelliSenseService intellisense_service_;
};

} // namespace tsuki::cli