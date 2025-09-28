#pragma once

#include <fmt/core.h>
#include <fmt/color.h>
#include <string>

namespace tsuki {

class DebugPrinter {
public:
    // Pretty printing functions
    static void printHeader(const std::string& title, fmt::color color = fmt::color::cyan);
    static void printSeparator(const std::string& c = "", int length = 50);
    static void printKeyValue(const std::string& key, const std::string& value,
                            fmt::color keyColor = fmt::color::yellow,
                            fmt::color valueColor = fmt::color::white);
    static void printError(const std::string& message);
    static void printSuccess(const std::string& message);
    static void printWarning(const std::string& message);
    static void printInfo(const std::string& message);
    static void printStackTrace(const std::string& trace);
    static std::string cleanSourcePath(const std::string& source);
    static std::string formatFunctionName(const std::string& name, const std::string& namewhat);
};

} // namespace tsuki