#include "tsuki/graphics.hpp"
#include <algorithm>
#include <cmath>

namespace tsuki {

// Image implementation
Image::Image(const std::string& filename) {
    load(filename);
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

bool Image::load(const std::string& filename) {
    unload();

#ifdef TSUKI_HAS_SDL_IMAGE
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        return false;
    }

    width_ = surface->w;
    height_ = surface->h;

    // Note: We need a renderer to create texture, this will be handled in Graphics::draw
    SDL_DestroySurface(surface);
    return false; // Placeholder for now - needs renderer
#else
    (void)filename;
    return false;
#endif
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
    int actual_segments = std::max(1, int(segments * std::abs(angle_range) / (2.0f * M_PI)));

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

    SDL_RenderTextureRotated(renderer_, image.getTexture(), nullptr, &dest, rotation * 180.0f / M_PI, &center, SDL_FLIP_NONE);
}

void Graphics::print(const std::string& text, float x, float y) {
    // Text rendering would require SDL_ttf implementation
    // For now, this is a placeholder
    (void)text; (void)x; (void)y; // Suppress unused parameter warnings
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

} // namespace tsuki