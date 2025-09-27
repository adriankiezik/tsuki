#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <array>
#include <vector>

#ifdef TSUKI_HAS_SDL_IMAGE
#include <SDL3_image/SDL_image.h>
#endif

namespace tsuki {

enum class DrawMode {
    Fill,
    Line
};

struct Color {
    float r, g, b, a;

    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    static Color white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static Color black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static Color red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static Color green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static Color blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
};

class Image {
public:
    Image() = default;
    explicit Image(const std::string& filename);
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    bool load(const std::string& filename);
    void unload();

    int getWidth() const;
    int getHeight() const;
    bool isValid() const { return texture_ != nullptr; }

    SDL_Texture* getTexture() const { return texture_; }

private:
    SDL_Texture* texture_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

class Graphics {
public:
    Graphics() = default;
    ~Graphics() = default;

    bool init(SDL_Renderer* renderer);
    void shutdown();

    // State management
    void clear();
    void clear(const Color& color);
    void present();

    void setColor(const Color& color);
    Color getColor() const { return current_color_; }

    // Drawing functions (similar to LOVE API)
    void rectangle(DrawMode mode, float x, float y, float width, float height);
    void circle(DrawMode mode, float x, float y, float radius, int segments = 32);
    void ellipse(DrawMode mode, float x, float y, float rx, float ry, int segments = 32);
    void line(float x1, float y1, float x2, float y2);
    void polygon(DrawMode mode, const std::vector<float>& points);
    void arc(DrawMode mode, float x, float y, float radius, float angle1, float angle2, int segments = 32);
    void point(float x, float y);
    void points(const std::vector<float>& points);

    // Image drawing
    void draw(const Image& image, float x, float y);
    void draw(const Image& image, float x, float y, float rotation, float sx = 1.0f, float sy = 1.0f,
              float ox = 0.0f, float oy = 0.0f);

    // Text drawing
    void print(const std::string& text, float x, float y);
    void printf(const std::string& text, float x, float y, float limit, const std::string& align = "left");

    // Transformation
    void push();
    void pop();
    void translate(float x, float y);
    void rotate(float angle);
    void scale(float sx, float sy);

private:
    SDL_Renderer* renderer_ = nullptr;
    Color current_color_ = Color::white();

    struct Transform {
        float tx = 0.0f, ty = 0.0f;
        float rotation = 0.0f;
        float sx = 1.0f, sy = 1.0f;
    };

    std::vector<Transform> transform_stack_;
    Transform current_transform_;

    void applyTransform();
    void drawCirclePoints(float cx, float cy, float x, float y);
};

} // namespace tsuki