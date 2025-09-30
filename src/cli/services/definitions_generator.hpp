#pragma once

#include <string>
#include <vector>
#include <map>

namespace tsuki::cli {

struct MethodInfo {
    std::string name;
    std::string params;      // Parameter signature
    std::string return_type; // Return type
};

struct ClassInfo {
    std::string name;
    std::vector<MethodInfo> methods;
    std::string description;
};

class DefinitionsGenerator {
public:
    // Generate definitions by introspecting a live Lua state
    std::string generate();

    // Save generated definitions to a file
    bool saveToFile(const std::string& path);

private:
    std::map<std::string, ClassInfo> classes_;

    void introspectLuaState();
    std::string formatDefinitions() const;
    void getMethodSignature(const std::string& class_name, const std::string& method_name,
                           std::string& params, std::string& return_type) const;
};

} // namespace tsuki::cli