#include "tsuki/timer.hpp"
#include <thread>

namespace tsuki {

Timer::Timer()
    : start_time_(Clock::now())
    , last_frame_time_(start_time_)
    , delta_time_(0.0)
    , fps_(0)
    , average_delta_(0.0)
    , fps_sample_index_(0)
    , fps_sum_(0.0) {

    for (int i = 0; i < FPS_SAMPLE_SIZE; ++i) {
        fps_samples_[i] = 0.0;
    }
}

void Timer::update() {
    auto current_time = Clock::now();
    auto delta_duration = current_time - last_frame_time_;
    delta_time_ = std::chrono::duration<double>(delta_duration).count();
    last_frame_time_ = current_time;

    updateFPS();
}

double Timer::getTime() const {
    auto current_time = Clock::now();
    auto duration = current_time - start_time_;
    return std::chrono::duration<double>(duration).count();
}

void Timer::sleep(double seconds) {
    auto duration = std::chrono::duration<double>(seconds);
    std::this_thread::sleep_for(duration);
}

double Timer::step(double dt) {
    delta_time_ = dt;
    return dt;
}

void Timer::step() {
    // Single step for debugging
}

void Timer::updateFPS() {
    // Remove old sample from sum
    fps_sum_ -= fps_samples_[fps_sample_index_];

    // Add new sample
    fps_samples_[fps_sample_index_] = delta_time_;
    fps_sum_ += delta_time_;

    // Move to next sample
    fps_sample_index_ = (fps_sample_index_ + 1) % FPS_SAMPLE_SIZE;

    // Calculate average
    average_delta_ = fps_sum_ / FPS_SAMPLE_SIZE;

    // Calculate FPS (avoid division by zero)
    if (average_delta_ > 0.0) {
        fps_ = static_cast<int>(1.0 / average_delta_);
    } else {
        fps_ = 0;
    }
}

} // namespace tsuki