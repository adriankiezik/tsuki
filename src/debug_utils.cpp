#include "tsuki/debug_utils.hpp"
#include <fmt/format.h>
#include <sstream>
#include <algorithm>

namespace tsuki {

// Unicode characters as constants to avoid encoding warnings
constexpr const char* BOX_TOP_LEFT = "\u256D";
constexpr const char* BOX_TOP_RIGHT = "\u256E";
constexpr const char* BOX_BOTTOM_LEFT = "\u2570";
constexpr const char* BOX_BOTTOM_RIGHT = "\u256F";
constexpr const char* BOX_VERTICAL = "\u2502";
constexpr const char* BOX_HORIZONTAL = "\u2500";
constexpr const char* TREE_BRANCH = "\u251C\u2500 ";
constexpr const char* TREE_VERTICAL = "\u2502  ";
constexpr const char* ICON_ERROR = "\u2717";
constexpr const char* ICON_SUCCESS = "\u2713";
constexpr const char* ICON_WARNING = "\u26A0";
constexpr const char* ICON_INFO = "\u2139";
constexpr const char* ICON_STACK = "\U0001F4CB";
constexpr const char* DEFAULT_SEPARATOR = "\u2500";

void DebugPrinter::printHeader(const std::string& title, fmt::color color) {
    // Much more compact header - just one line with icon
    fmt::print(fg(color) | fmt::emphasis::bold, "{} {}\n", title.find("ERROR") != std::string::npos ? ICON_ERROR : ICON_INFO, title);
}

void DebugPrinter::printSeparator(const std::string& c, int length) {
    std::string separator;
    const std::string& sep_char = c.empty() ? DEFAULT_SEPARATOR : c;
    for (int i = 0; i < length; ++i) {
        separator += sep_char;
    }
    fmt::print(fg(fmt::color::dim_gray), "{}\n", separator);
}

void DebugPrinter::printKeyValue(const std::string& key, const std::string& value,
                                fmt::color keyColor, fmt::color valueColor) {
    fmt::print(fg(keyColor), "{}:", key);
    fmt::print(" ");
    fmt::print(fg(valueColor), "{}\n", value);
}

void DebugPrinter::printError(const std::string& message) {
    fmt::print(fg(fmt::color::light_coral) | fmt::emphasis::bold, "{} {}\n", ICON_ERROR, message);
}

void DebugPrinter::printSuccess(const std::string& message) {
    fmt::print("  ");
    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "{} {}\n", ICON_SUCCESS, message);
}

void DebugPrinter::printWarning(const std::string& message) {
    fmt::print("  ");
    fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, "{} {}\n", ICON_WARNING, message);
}

void DebugPrinter::printInfo(const std::string& message) {
    fmt::print("  ");
    fmt::print(fg(fmt::color::blue) | fmt::emphasis::bold, "{} {}\n", ICON_INFO, message);
}

void DebugPrinter::printStackTrace(const std::string& trace) {
    std::stringstream ss(trace);
    std::string line;
    bool first = true;
    bool in_traceback = false;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        // Clean up the line
        std::string cleaned_line = line;
        while (!cleaned_line.empty() && (cleaned_line.front() == ' ' || cleaned_line.front() == '\t')) {
            cleaned_line.erase(0, 1);
        }

        // Check if this is the traceback header
        if (cleaned_line.find("stack traceback:") != std::string::npos) {
            in_traceback = true;
            fmt::print(fg(fmt::color::dim_gray), "{}\n", cleaned_line);
            continue;
        }

        // Show all lines when we're in traceback section
        if (in_traceback && !cleaned_line.empty()) {
            if (first) {
                fmt::print(fg(fmt::color::dim_gray), "  ");
                first = false;
            } else {
                fmt::print(fg(fmt::color::dim_gray), "  ");
            }

            // Color code: Lua files in white, C functions in cyan, others in gray
            if (cleaned_line.find(".lua:") != std::string::npos) {
                fmt::print(fg(fmt::color::white), "{}\n", cleaned_line);
            } else if (cleaned_line.find("[C]:") != std::string::npos) {
                fmt::print(fg(fmt::color::cyan), "{}\n", cleaned_line);
            } else {
                fmt::print(fg(fmt::color::dim_gray), "{}\n", cleaned_line);
            }
        }
        // Show lines that come before the traceback header (the actual error message)
        else if (!in_traceback && !cleaned_line.empty()) {
            // This is the actual error message, show it prominently
            fmt::print(fg(fmt::color::light_coral), "{}\n", cleaned_line);
        }
    }

    // Add newline after complete error message and stack trace
    fmt::print("\n");
}

std::string DebugPrinter::cleanSourcePath(const std::string& source) {
    std::string cleaned = source;

    // Remove the @ prefix that Lua adds
    if (!cleaned.empty() && cleaned[0] == '@') {
        cleaned = cleaned.substr(1);
    }

    // Remove common prefixes to make paths shorter
    if (cleaned.find("./") == 0) {
        cleaned = cleaned.substr(2);
    }

    // Just show filename if path is too long
    size_t lastSlash = cleaned.find_last_of("/\\");
    if (lastSlash != std::string::npos && cleaned.length() > 30) {
        cleaned = "..." + cleaned.substr(lastSlash);
    }

    return cleaned;
}

std::string DebugPrinter::formatFunctionName(const std::string& name, const std::string& namewhat) {
    if (name == "anonymous" || name.empty()) {
        return "anonymous function";
    }

    if (!namewhat.empty()) {
        return fmt::format("{} ({})", name, namewhat);
    }

    return name;
}

} // namespace tsuki