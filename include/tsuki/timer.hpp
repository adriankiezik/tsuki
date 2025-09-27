#pragma once

#include <chrono>

namespace tsuki {

class Timer {
public:
    Timer();
    ~Timer() = default;

    void update();

    // Delta time (time since last frame)
    double getDelta() const { return delta_time_; }

    // Total time since engine start
    double getTime() const;

    // Frame rate
    int getFPS() const { return fps_; }
    double getAverageDelta() const { return average_delta_; }

    // Sleep
    void sleep(double seconds);

    // Step mode (for debugging)
    void step();
    double step(double dt);

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint start_time_;
    TimePoint last_frame_time_;
    double delta_time_;
    int fps_;
    double average_delta_;

    // FPS calculation
    static constexpr int FPS_SAMPLE_SIZE = 60;
    double fps_samples_[FPS_SAMPLE_SIZE];
    int fps_sample_index_;
    double fps_sum_;

    void updateFPS();
};

} // namespace tsuki