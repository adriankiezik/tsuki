#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <memory>

namespace tsuki {

class Font {
public:
    Font();
    ~Font();

    // Non-copyable but moveable
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    // Load font from file or memory
    bool loadFromFile(const std::string& filename, float size = 16.0f);
    bool loadFromMemory(const unsigned char* data, size_t size, float fontSize = 16.0f);

    // Get font properties
    float getSize() const { return size_; }
    bool isLoaded() const { return !fontData_.empty() && stbFont_ != nullptr; }

    // Text measurement
    void getTextSize(const std::string& text, int* width, int* height) const;

    // Render text to SDL texture
    SDL_Texture* renderText(SDL_Renderer* renderer, const std::string& text,
                           Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255) const;

private:
    std::vector<unsigned char> fontData_;
    void* stbFont_; // stbtt_fontinfo*
    float size_;
    float scale_;

    void cleanup();
    bool initializeFont();
};

} // namespace tsuki