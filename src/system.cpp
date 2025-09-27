#include "tsuki/system.hpp"
#include <SDL3/SDL.h>

namespace tsuki {

OS System::getOS() const {
    return detectOS();
}

std::string System::getOSString() const {
    const char* platform = SDL_GetPlatform();
    return platform ? platform : "Unknown";
}

std::string System::getProcessorCount() const {
    return std::to_string(SDL_GetNumLogicalCPUCores());
}

std::string System::getVersion() const {
    return "Tsuki Engine 1.0.0";
}

int System::getPowerInfo() const {
    int seconds, percent;
    SDL_PowerState state = SDL_GetPowerInfo(&seconds, &percent);
    return (state == SDL_POWERSTATE_NO_BATTERY) ? -1 : percent;
}

bool System::isPowerSaveMode() const {
    return false; // Not easily detectable across platforms
}

std::string System::getClipboardText() const {
    char* text = SDL_GetClipboardText();
    std::string result = text ? text : "";
    if (text) {
        SDL_free(text);
    }
    return result;
}

void System::setClipboardText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

bool System::hasClipboardText() const {
    return SDL_HasClipboardText();
}

void System::vibrate(double seconds) {
    (void)seconds; // Suppress unused parameter warning
    // Vibration is platform-specific and requires additional implementation
}

bool System::openURL(const std::string& url) {
    return SDL_OpenURL(url.c_str()) == 0;
}

OS System::detectOS() const {
    const char* platform = SDL_GetPlatform();
    if (!platform) return OS::Unknown;

    std::string platformStr(platform);
    if (platformStr == "Windows") return OS::Windows;
    if (platformStr == "Mac OS X") return OS::macOS;
    if (platformStr == "Linux") return OS::Linux;
    if (platformStr == "iOS") return OS::iOS;
    if (platformStr == "Android") return OS::Android;

    return OS::Unknown;
}

} // namespace tsuki