#pragma once

#include <string>

namespace tsuki {

enum class OS {
    Windows,
    macOS,
    Linux,
    iOS,
    Android,
    Unknown
};

class System {
public:
    System() = default;
    ~System() = default;

    // Operating system
    OS getOS() const;
    std::string getOSString() const;

    // System information
    std::string getProcessorCount() const;
    std::string getVersion() const;

    // Power information
    int getPowerInfo() const; // Returns battery percentage, -1 if no battery
    bool isPowerSaveMode() const;

    // Clipboard
    std::string getClipboardText() const;
    void setClipboardText(const std::string& text);
    bool hasClipboardText() const;

    // Vibration (mobile platforms)
    void vibrate(double seconds = 0.5);

    // URLs
    bool openURL(const std::string& url);

private:
    OS detectOS() const;
};

} // namespace tsuki