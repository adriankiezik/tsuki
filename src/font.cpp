#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "tsuki/font.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

namespace tsuki {

Font::Font() : stbFont_(nullptr), size_(20.0f), scale_(1.0f) {
}

Font::~Font() {
    cleanup();
}

Font::Font(Font&& other) noexcept
    : fontData_(std::move(other.fontData_)),
      stbFont_(other.stbFont_),
      size_(other.size_),
      scale_(other.scale_) {
    other.stbFont_ = nullptr;
    other.size_ = 0.0f;
    other.scale_ = 0.0f;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        cleanup();
        fontData_ = std::move(other.fontData_);
        stbFont_ = other.stbFont_;
        size_ = other.size_;
        scale_ = other.scale_;

        other.stbFont_ = nullptr;
        other.size_ = 0.0f;
        other.scale_ = 0.0f;
    }
    return *this;
}

void Font::cleanup() {
    if (stbFont_) {
        delete static_cast<stbtt_fontinfo*>(stbFont_);
        stbFont_ = nullptr;
    }
    fontData_.clear();
    size_ = 0.0f;
    scale_ = 0.0f;
}

bool Font::loadFromFile(const std::string& filename, float size) {
    cleanup();

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fontData_.resize(fileSize);
    if (!file.read(reinterpret_cast<char*>(fontData_.data()), fileSize)) {
        cleanup();
        return false;
    }

    size_ = size;
    return initializeFont();
}

bool Font::loadFromMemory(const unsigned char* data, size_t size, float fontSize) {
    cleanup();

    fontData_.assign(data, data + size);
    size_ = fontSize;
    return initializeFont();
}

bool Font::initializeFont() {
    if (fontData_.empty()) {
        return false;
    }

    stbFont_ = new stbtt_fontinfo();
    auto* fontInfo = static_cast<stbtt_fontinfo*>(stbFont_);

    if (!stbtt_InitFont(fontInfo, fontData_.data(), 0)) {
        cleanup();
        return false;
    }

    scale_ = stbtt_ScaleForPixelHeight(fontInfo, size_);
    return true;
}

void Font::getTextSize(const std::string& text, int* width, int* height) const {
    if (!isLoaded() || text.empty()) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }

    auto* fontInfo = static_cast<stbtt_fontinfo*>(stbFont_);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, &lineGap);

    if (height) {
        *height = static_cast<int>((ascent - descent) * scale_);
    }

    if (width) {
        float totalWidth = 0.0f;
        for (size_t i = 0; i < text.length(); ++i) {
            int advance, leftSideBearing;
            stbtt_GetCodepointHMetrics(fontInfo, text[i], &advance, &leftSideBearing);
            totalWidth += advance * scale_;

            // Add kerning if not the last character
            if (i < text.length() - 1) {
                int kern = stbtt_GetCodepointKernAdvance(fontInfo, text[i], text[i + 1]);
                totalWidth += kern * scale_;
            }
        }
        *width = static_cast<int>(totalWidth);
    }
}

SDL_Texture* Font::renderText(SDL_Renderer* renderer, const std::string& text,
                             Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
    (void)a; // Alpha channel not yet implemented
    if (!isLoaded() || text.empty() || !renderer) {
        return nullptr;
    }


    int textWidth, textHeight;
    getTextSize(text, &textWidth, &textHeight);

    if (textWidth <= 0 || textHeight <= 0) {
        return nullptr;
    }

    // Create surface for the text
    SDL_Surface* surface = SDL_CreateSurface(textWidth, textHeight, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        return nullptr;
    }


    // Clear surface
    SDL_FillSurfaceRect(surface, nullptr, 0x00000000); // Transparent black

    // Get surface pixels
    void* pixels = surface->pixels;
    int pitch = surface->pitch;

    auto* fontInfo = static_cast<stbtt_fontinfo*>(stbFont_);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, &lineGap);

    int baseline = static_cast<int>(ascent * scale_);
    float x = 0.0f;

    // Render each character
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];

        int advance, leftSideBearing;
        stbtt_GetCodepointHMetrics(fontInfo, c, &advance, &leftSideBearing);

        // Get character bitmap with proper scaling
        int width, height, xOffset, yOffset;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(fontInfo, scale_, scale_, c,
                                                        &width, &height, &xOffset, &yOffset);


        if (bitmap) {
            // Copy bitmap to texture
            int startX = static_cast<int>(x) + static_cast<int>(leftSideBearing * scale_) + xOffset;
            int startY = baseline + yOffset;

            for (int row = 0; row < height; ++row) {
                for (int col = 0; col < width; ++col) {
                    int texX = startX + col;
                    int texY = startY + row;

                    if (texX >= 0 && texX < textWidth && texY >= 0 && texY < textHeight) {
                        unsigned char alpha = bitmap[row * width + col];
                        if (alpha > 0) {
                            Uint32* pixel = static_cast<Uint32*>(pixels) + texY * (pitch / 4) + texX;
                            // Premultiply alpha for better blending
                            Uint8 premult_r = (r * alpha) / 255;
                            Uint8 premult_g = (g * alpha) / 255;
                            Uint8 premult_b = (b * alpha) / 255;
                            // Use RGBA8888 format (R=MSB, A=LSB)
                            *pixel = (premult_r << 24) | (premult_g << 16) | (premult_b << 8) | alpha;
                        }
                    }
                }
            }

            stbtt_FreeBitmap(bitmap, nullptr);
        }

        x += advance * scale_;

        // Add kerning
        if (i < text.length() - 1) {
            int kern = stbtt_GetCodepointKernAdvance(fontInfo, c, text[i + 1]);
            x += kern * scale_;
        }
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!texture) {
        return nullptr;
    }


    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

} // namespace tsuki