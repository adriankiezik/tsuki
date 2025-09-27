#include "tsuki/math.hpp"
#include <random>
#include <chrono>

namespace tsuki {

Math::Math() : generator_(std::chrono::steady_clock::now().time_since_epoch().count()),
               distribution_(0.0, 1.0) {
}

void Math::randomseed(unsigned int seed) {
    generator_.seed(seed);
}

double Math::random() {
    return distribution_(generator_);
}

double Math::random(double max) {
    return random() * max;
}

double Math::random(double min, double max) {
    return min + random() * (max - min);
}

int Math::randomInt(int min, int max) {
    std::uniform_int_distribution<int> int_dist(min, max);
    return int_dist(generator_);
}

double Math::clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

double Math::distance(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

std::pair<double, double> Math::normalize(double x, double y) {
    double len = length(x, y);
    if (len == 0.0) {
        return {0.0, 0.0};
    }
    return {x / len, y / len};
}

} // namespace tsuki