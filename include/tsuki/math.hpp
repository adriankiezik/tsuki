#pragma once

#include <cmath>
#include <random>

namespace tsuki {

class Math {
public:
    Math();
    ~Math() = default;

    // Constants
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double TWO_PI = 2.0 * PI;
    static constexpr double HALF_PI = PI * 0.5;

    // Random number generation
    void randomseed(unsigned int seed);
    double random();
    double random(double max);
    double random(double min, double max);
    int randomInt(int min, int max);

    // Trigonometric functions (working with radians)
    static double sin(double x) { return std::sin(x); }
    static double cos(double x) { return std::cos(x); }
    static double tan(double x) { return std::tan(x); }
    static double asin(double x) { return std::asin(x); }
    static double acos(double x) { return std::acos(x); }
    static double atan(double x) { return std::atan(x); }
    static double atan2(double y, double x) { return std::atan2(y, x); }

    // Utility functions
    static double abs(double x) { return std::abs(x); }
    static double floor(double x) { return std::floor(x); }
    static double ceil(double x) { return std::ceil(x); }
    static double round(double x) { return std::round(x); }
    static double sqrt(double x) { return std::sqrt(x); }
    static double pow(double base, double exp) { return std::pow(base, exp); }
    static double log(double x) { return std::log(x); }
    static double log10(double x) { return std::log10(x); }
    static double exp(double x) { return std::exp(x); }

    // Interpolation
    static double lerp(double a, double b, double t) { return a + t * (b - a); }
    static double clamp(double value, double min, double max);
    static double min(double a, double b) { return std::min(a, b); }
    static double max(double a, double b) { return std::max(a, b); }

    // Angle conversion
    static double radians(double degrees) { return degrees * PI / 180.0; }
    static double degrees(double radians) { return radians * 180.0 / PI; }

    // Distance and vector functions
    static double distance(double x1, double y1, double x2, double y2);
    static double length(double x, double y) { return sqrt(x * x + y * y); }
    static double dot(double x1, double y1, double x2, double y2) { return x1 * x2 + y1 * y2; }

    // Normalization
    static std::pair<double, double> normalize(double x, double y);

private:
    std::mt19937 generator_;
    std::uniform_real_distribution<double> distribution_;
};

} // namespace tsuki