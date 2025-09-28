#include "tsuki/graphics.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Define M_PI for Windows MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace tsuki {

// Image implementation
Image::Image(const std::string& filename, SDL_Renderer* renderer) {
    load(filename, renderer);
}

Image::~Image() {
    unload();
}

Image::Image(Image&& other) noexcept
    : texture_(other.texture_), width_(other.width_), height_(other.height_) {
    other.texture_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        unload();
        texture_ = other.texture_;
        width_ = other.width_;
        height_ = other.height_;
        other.texture_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

bool Image::load(const std::string& filename, SDL_Renderer* renderer) {
    unload();

    if (!renderer) {
        return false;
    }

    // Load image using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4); // Force RGBA

    if (!data) {
        return false;
    }

    width_ = width;
    height_ = height;

    // Create SDL surface from loaded data
    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, data, width * 4);
    if (!surface) {
        stbi_image_free(data);
        return false;
    }

    // Create texture from surface
    texture_ = SDL_CreateTextureFromSurface(renderer, surface);

    // Clean up
    SDL_DestroySurface(surface);
    stbi_image_free(data);

    return texture_ != nullptr;
}

void Image::unload() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    width_ = 0;
    height_ = 0;
}

int Image::getWidth() const {
    return width_;
}

int Image::getHeight() const {
    return height_;
}

// Graphics implementation
bool Graphics::init(SDL_Renderer* renderer) {
    renderer_ = renderer;
    current_color_ = Color::white();

    if (renderer_) {
        // Try to initialize a default system font
        initializeDefaultFont();
    }

    return renderer_ != nullptr;
}

void Graphics::shutdown() {
    renderer_ = nullptr;
}

void Graphics::clear() {
    clear(Color::black());
}

void Graphics::clear(const Color& color) {
    if (!renderer_) return;

    SDL_SetRenderDrawColor(renderer_,
        static_cast<Uint8>(color.r * 255),
        static_cast<Uint8>(color.g * 255),
        static_cast<Uint8>(color.b * 255),
        static_cast<Uint8>(color.a * 255));
    SDL_RenderClear(renderer_);
}

void Graphics::present() {
    if (renderer_) {
        SDL_RenderPresent(renderer_);
    }
}

void Graphics::setColor(const Color& color) {
    current_color_ = color;
    if (renderer_) {
        SDL_SetRenderDrawColor(renderer_,
            static_cast<Uint8>(color.r * 255),
            static_cast<Uint8>(color.g * 255),
            static_cast<Uint8>(color.b * 255),
            static_cast<Uint8>(color.a * 255));
    }
}

void Graphics::rectangle(DrawMode mode, float x, float y, float width, float height) {
    if (!renderer_) return;

    setColor(current_color_);

    SDL_FRect rect = {x, y, width, height};

    if (mode == DrawMode::Fill) {
        SDL_RenderFillRect(renderer_, &rect);
    } else {
        SDL_RenderRect(renderer_, &rect);
    }
}

void Graphics::circle(DrawMode mode, float x, float y, float radius, int segments) {
    if (!renderer_) return;

    setColor(current_color_);

    if (mode == DrawMode::Fill) {
        // Draw filled circle using triangles (simplified)
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float(i) / segments) * 2.0f * M_PI;
            float angle2 = (float(i + 1) / segments) * 2.0f * M_PI;

            float x1 = x + radius * std::cos(angle1);
            float y1 = y + radius * std::sin(angle1);
            float x2 = x + radius * std::cos(angle2);
            float y2 = y + radius * std::sin(angle2);

            // Draw triangle as lines for now (simpler approach)
            SDL_RenderLine(renderer_, x, y, x1, y1);
            SDL_RenderLine(renderer_, x1, y1, x2, y2);
            SDL_RenderLine(renderer_, x2, y2, x, y);
        }
    } else {
        // Draw circle outline
        std::vector<SDL_FPoint> points;
        for (int i = 0; i <= segments; ++i) {
            float angle = (float(i) / segments) * 2.0f * M_PI;
            points.push_back({
                x + radius * std::cos(angle),
                y + radius * std::sin(angle)
            });
        }
        SDL_RenderLines(renderer_, points.data(), points.size());
    }
}

void Graphics::ellipse(DrawMode mode, float x, float y, float rx, float ry, int segments) {
    if (!renderer_) return;

    setColor(current_color_);

    if (mode == DrawMode::Fill) {
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float(i) / segments) * 2.0f * M_PI;
            float angle2 = (float(i + 1) / segments) * 2.0f * M_PI;

            float x1 = x + rx * std::cos(angle1);
            float y1 = y + ry * std::sin(angle1);
            float x2 = x + rx * std::cos(angle2);
            float y2 = y + ry * std::sin(angle2);

            // Draw triangle as lines for now
            SDL_RenderLine(renderer_, x, y, x1, y1);
            SDL_RenderLine(renderer_, x1, y1, x2, y2);
            SDL_RenderLine(renderer_, x2, y2, x, y);
        }
    } else {
        std::vector<SDL_FPoint> points;
        for (int i = 0; i <= segments; ++i) {
            float angle = (float(i) / segments) * 2.0f * M_PI;
            points.push_back({
                x + rx * std::cos(angle),
                y + ry * std::sin(angle)
            });
        }
        SDL_RenderLines(renderer_, points.data(), points.size());
    }
}

void Graphics::line(float x1, float y1, float x2, float y2) {
    if (!renderer_) return;

    setColor(current_color_);
    SDL_RenderLine(renderer_, x1, y1, x2, y2);
}

void Graphics::polygon(DrawMode mode, const std::vector<float>& points) {
    if (!renderer_ || points.size() < 6) return; // Need at least 3 points (6 coordinates)

    setColor(current_color_);

    std::vector<SDL_FPoint> sdl_points;
    for (size_t i = 0; i < points.size(); i += 2) {
        sdl_points.push_back({points[i], points[i + 1]});
    }

    if (mode == DrawMode::Fill) {
        // Simple triangle fan fill using lines (simplified)
        for (size_t i = 1; i < sdl_points.size() - 1; ++i) {
            SDL_RenderLine(renderer_, sdl_points[0].x, sdl_points[0].y, sdl_points[i].x, sdl_points[i].y);
            SDL_RenderLine(renderer_, sdl_points[i].x, sdl_points[i].y, sdl_points[i + 1].x, sdl_points[i + 1].y);
            SDL_RenderLine(renderer_, sdl_points[i + 1].x, sdl_points[i + 1].y, sdl_points[0].x, sdl_points[0].y);
        }
    } else {
        sdl_points.push_back(sdl_points[0]); // Close the polygon
        SDL_RenderLines(renderer_, sdl_points.data(), sdl_points.size());
    }
}

void Graphics::arc(DrawMode mode, float x, float y, float radius, float angle1, float angle2, int segments) {
    if (!renderer_) return;

    setColor(current_color_);

    float angle_range = angle2 - angle1;
    float abs_angle_range = std::fabs(angle_range);
    int calculated_segments = static_cast<int>(segments * abs_angle_range / (2.0f * M_PI));
    int actual_segments = std::max(1, calculated_segments);

    if (mode == DrawMode::Fill) {
        for (int i = 0; i < actual_segments; ++i) {
            float a1 = angle1 + (float(i) / actual_segments) * angle_range;
            float a2 = angle1 + (float(i + 1) / actual_segments) * angle_range;

            float x1 = x + radius * std::cos(a1);
            float y1 = y + radius * std::sin(a1);
            float x2 = x + radius * std::cos(a2);
            float y2 = y + radius * std::sin(a2);

            // Draw triangle as lines
            SDL_RenderLine(renderer_, x, y, x1, y1);
            SDL_RenderLine(renderer_, x1, y1, x2, y2);
            SDL_RenderLine(renderer_, x2, y2, x, y);
        }
    } else {
        std::vector<SDL_FPoint> points;
        for (int i = 0; i <= actual_segments; ++i) {
            float angle = angle1 + (float(i) / actual_segments) * angle_range;
            points.push_back({
                x + radius * std::cos(angle),
                y + radius * std::sin(angle)
            });
        }
        SDL_RenderLines(renderer_, points.data(), points.size());
    }
}

void Graphics::point(float x, float y) {
    if (!renderer_) return;

    setColor(current_color_);
    SDL_RenderPoint(renderer_, x, y);
}

void Graphics::points(const std::vector<float>& points) {
    if (!renderer_) return;

    setColor(current_color_);

    for (size_t i = 0; i < points.size(); i += 2) {
        SDL_RenderPoint(renderer_, points[i], points[i + 1]);
    }
}

void Graphics::draw(const Image& image, float x, float y) {
    draw(image, x, y, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
}

void Graphics::draw(const Image& image, float x, float y, float rotation, float sx, float sy, float ox, float oy) {
    if (!renderer_ || !image.isValid()) return;

    SDL_FRect dest = {x - ox * sx, y - oy * sy, image.getWidth() * sx, image.getHeight() * sy};
    SDL_FPoint center = {ox * sx, oy * sy};

    SDL_RenderTextureRotated(renderer_, image.getTexture(), nullptr, &dest, double(rotation * 180.0f / M_PI), &center, SDL_FLIP_NONE);
}

void Graphics::print(const std::string& text, float x, float y) {
    if (!renderer_ || text.empty()) {
        return;
    }

    // If we have a font loaded, use the proper font system
    if (current_font_) {
        // Render text using the Font system
        Uint8 r = static_cast<Uint8>(current_color_.r * 255);
        Uint8 g = static_cast<Uint8>(current_color_.g * 255);
        Uint8 b = static_cast<Uint8>(current_color_.b * 255);
        Uint8 a = static_cast<Uint8>(current_color_.a * 255);

        SDL_Texture* textTexture = current_font_->renderText(renderer_, text, r, g, b, a);
        if (!textTexture) {
            // If custom font fails, fall back to SDL debug font
            SDL_SetRenderDrawColor(renderer_, r, g, b, a);
            SDL_RenderDebugText(renderer_, x, y, text.c_str());
            return;
        }

        // Get texture dimensions
        float textWidth, textHeight;
        SDL_GetTextureSize(textTexture, &textWidth, &textHeight);


        // Set destination rectangle
        SDL_FRect destRect = {x, y, textWidth, textHeight};

        // Render the texture
        SDL_RenderTexture(renderer_, textTexture, nullptr, &destRect);

        // Clean up
        SDL_DestroyTexture(textTexture);
    } else {
        // Use SDL3's built-in debug font as default - never use fallback text
        Uint8 r = static_cast<Uint8>(current_color_.r * 255);
        Uint8 g = static_cast<Uint8>(current_color_.g * 255);
        Uint8 b = static_cast<Uint8>(current_color_.b * 255);
        Uint8 a = static_cast<Uint8>(current_color_.a * 255);

        SDL_SetRenderDrawColor(renderer_, r, g, b, a);
        SDL_RenderDebugText(renderer_, x, y, text.c_str());
    }
}

// Font management functions
bool Graphics::loadFont(const std::string& name, const std::string& filename, float size) {
    auto font = std::make_unique<Font>();
    if (!font->loadFromFile(filename, size)) {
        return false;
    }

    fonts_[name] = std::move(font);
    return true;
}

bool Graphics::setFont(const std::string& name) {
    auto it = fonts_.find(name);
    if (it != fonts_.end()) {
        current_font_ = it->second.get();
        return true;
    }
    return false;
}

void Graphics::setDefaultFont() {
    current_font_ = nullptr;
    // Try to set to a default font if one exists
    auto it = fonts_.find("default");
    if (it != fonts_.end()) {
        current_font_ = it->second.get();
    }
}

bool Graphics::initializeDefaultFont() {
    // Try to load a system font as default
    std::vector<std::string> systemFontPaths = {
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",           // Noto Sans (most common)
        "/usr/share/fonts/TTF/DejaVuSans.ttf",                 // DejaVu Sans
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",     // DejaVu Sans (Ubuntu)
        "/usr/share/fonts/TTF/LiberationSans-Regular.ttf",     // Liberation Sans
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf", // Liberation Sans (alt path)
        "/System/Library/Fonts/Arial.ttf",                     // macOS
        "C:\\Windows\\Fonts\\arial.ttf"                        // Windows
    };

    for (const auto& fontPath : systemFontPaths) {
        if (loadFont("default", fontPath, 32.0f)) {  // Larger size for better readability
            setFont("default");
            return true;
        }
    }

    // If no system font found, just use SDL debug font
    return false;
}

// Image management functions
bool Graphics::loadImage(const std::string& name, const std::string& filename) {
    if (!renderer_) {
        return false;
    }

    auto image = std::make_unique<Image>();
    if (!image->load(filename, renderer_)) {
        return false;
    }

    images_[name] = std::move(image);
    return true;
}

bool Graphics::unloadImage(const std::string& name) {
    auto it = images_.find(name);
    if (it != images_.end()) {
        images_.erase(it);
        return true;
    }
    return false;
}

Image* Graphics::getImage(const std::string& name) {
    auto it = images_.find(name);
    if (it != images_.end()) {
        return it->second.get();
    }
    return nullptr;
}

// String-based draw methods
void Graphics::draw(const std::string& imageName, float x, float y) {
    Image* image = getImage(imageName);
    if (image) {
        draw(*image, x, y);
    }
}

void Graphics::draw(const std::string& imageName, float x, float y, float rotation, float sx, float sy, float ox, float oy) {
    Image* image = getImage(imageName);
    if (image) {
        draw(*image, x, y, rotation, sx, sy, ox, oy);
    }
}

std::pair<int, int> Graphics::getTextSize(const std::string& text) {
    if (!current_font_) {
        // Use SDL3 debug font sizing (8x8 pixels per character)
        const int charWidth = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
        const int charHeight = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

        int width = static_cast<int>(text.length()) * charWidth;
        int height = charHeight;

        return {width, height};
    }

    int width, height;
    current_font_->getTextSize(text, &width, &height);
    return {width, height};
}

// Additional print overloads
void Graphics::print(const std::string& text, float x, float y, HorizontalAlign halign) {
    if (!renderer_) return;

    auto pos = calculateAlignedPosition(text, x, y, halign, VerticalAlign::Top);
    print(text, pos.first, pos.second);
}

void Graphics::print(const std::string& text, float x, float y, HorizontalAlign halign, VerticalAlign valign) {
    if (!renderer_) return;

    auto pos = calculateAlignedPosition(text, x, y, halign, valign);
    print(text, pos.first, pos.second);
}

void Graphics::print(const std::string& text, float x, float y, const std::string& align) {
    if (!renderer_) return;

    auto alignment = parseAlignment(align);
    auto pos = calculateAlignedPosition(text, x, y, alignment.first, alignment.second);
    print(text, pos.first, pos.second);
}

void Graphics::printAligned(const std::string& text, float x, float y, float width, float height, const std::string& align) {
    auto alignment = parseAlignment(align);
    printAligned(text, x, y, width, height, alignment.first, alignment.second);
}

void Graphics::printAligned(const std::string& text, float x, float y, float width, float height, HorizontalAlign halign, VerticalAlign valign) {
    if (!renderer_) return;

    auto pos = calculateAlignedPosition(text, x, y, width, height, halign, valign);
    print(text, pos.first, pos.second);
}

void Graphics::printf(const std::string& text, float x, float y, float limit, const std::string& align) {
    // Formatted text rendering placeholder
    (void)text; (void)x; (void)y; (void)limit; (void)align; // Suppress unused parameter warnings
}

void Graphics::push() {
    transform_stack_.push_back(current_transform_);
}

void Graphics::pop() {
    if (!transform_stack_.empty()) {
        current_transform_ = transform_stack_.back();
        transform_stack_.pop_back();
    }
}

void Graphics::translate(float x, float y) {
    current_transform_.tx += x;
    current_transform_.ty += y;
}

void Graphics::rotate(float angle) {
    current_transform_.rotation += angle;
}

void Graphics::scale(float sx, float sy) {
    current_transform_.sx *= sx;
    current_transform_.sy *= sy;
}

void Graphics::applyTransform() {
    // Transform application would be implemented here
}

// Text alignment helper functions
std::pair<HorizontalAlign, VerticalAlign> Graphics::parseAlignment(const std::string& align) {
    HorizontalAlign halign = HorizontalAlign::Left;
    VerticalAlign valign = VerticalAlign::Top;

    if (align.find("center") != std::string::npos) {
        halign = HorizontalAlign::Center;
    } else if (align.find("right") != std::string::npos) {
        halign = HorizontalAlign::Right;
    }

    if (align.find("middle") != std::string::npos) {
        valign = VerticalAlign::Middle;
    } else if (align.find("bottom") != std::string::npos) {
        valign = VerticalAlign::Bottom;
    }

    return {halign, valign};
}

std::pair<float, float> Graphics::calculateAlignedPosition(const std::string& text, float x, float y,
                                                          HorizontalAlign halign, VerticalAlign valign) {
    auto size = getTextSize(text);  // This now works with fallback
    float alignedX = x;
    float alignedY = y;

    switch (halign) {
        case HorizontalAlign::Center:
            alignedX = x - size.first / 2.0f;
            break;
        case HorizontalAlign::Right:
            alignedX = x - size.first;
            break;
        case HorizontalAlign::Left:
        default:
            break;
    }

    switch (valign) {
        case VerticalAlign::Middle:
            alignedY = y - size.second / 2.0f;
            break;
        case VerticalAlign::Bottom:
            alignedY = y - size.second;
            break;
        case VerticalAlign::Top:
        default:
            break;
    }

    return {alignedX, alignedY};
}

std::pair<float, float> Graphics::calculateAlignedPosition(const std::string& text, float x, float y, float width, float height,
                                                          HorizontalAlign halign, VerticalAlign valign) {
    auto size = getTextSize(text);  // This now works with fallback
    float alignedX = x;
    float alignedY = y;

    switch (halign) {
        case HorizontalAlign::Center:
            alignedX = x + (width - size.first) / 2.0f;
            break;
        case HorizontalAlign::Right:
            alignedX = x + width - size.first;
            break;
        case HorizontalAlign::Left:
        default:
            break;
    }

    switch (valign) {
        case VerticalAlign::Middle:
            alignedY = y + (height - size.second) / 2.0f;
            break;
        case VerticalAlign::Bottom:
            alignedY = y + height - size.second;
            break;
        case VerticalAlign::Top:
        default:
            break;
    }

    return {alignedX, alignedY};
}


} // namespace tsuki