#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "miniaudio.h"

namespace tsuki {

// Forward declarations
class AudioEngine;

class Sound {
public:
    Sound() = default;
    explicit Sound(const std::string& filename);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    Sound(Sound&& other) noexcept;
    Sound& operator=(Sound&& other) noexcept;

    bool load(const std::string& filename);
    void unload();
    bool isValid() const { return is_loaded_; }

    ma_decoder* getDecoder() { return decoder_.get(); }
    const std::string& getFilename() const { return filename_; }

private:
    std::unique_ptr<ma_decoder> decoder_;
    std::string filename_;
    bool is_loaded_ = false;
};

class Music {
public:
    Music() = default;
    explicit Music(const std::string& filename);
    ~Music();

    Music(const Music&) = delete;
    Music& operator=(const Music&) = delete;
    Music(Music&& other) noexcept;
    Music& operator=(Music&& other) noexcept;

    bool load(const std::string& filename);
    void unload();
    bool isValid() const { return is_loaded_; }

    ma_decoder* getDecoder() { return decoder_.get(); }
    const std::string& getFilename() const { return filename_; }

private:
    std::unique_ptr<ma_decoder> decoder_;
    std::string filename_;
    bool is_loaded_ = false;
};

class Audio {
public:
    Audio();
    ~Audio();

    bool init();
    void shutdown();

    // Sound effects
    void play(const Sound& sound, int loops = 0);
    void pause();
    void resume();
    void stop();

    // Background music
    void playMusic(const Music& music, int loops = -1);
    void pauseMusic();
    void resumeMusic();
    void stopMusic();

    // Volume control (0.0 - 1.0)
    void setVolume(float volume);
    float getVolume() const;

    void setMusicVolume(float volume);
    float getMusicVolume() const;

    // State queries
    bool isPlaying() const;
    bool isMusicPlaying() const;
    bool isMusicPaused() const;

private:
    std::unique_ptr<ma_engine> engine_;
    ma_sound current_sound_;
    ma_sound current_music_;

    float volume_ = 1.0f;
    float music_volume_ = 1.0f;
    bool initialized_ = false;
    bool sound_loaded_ = false;
    bool music_loaded_ = false;
    bool music_paused_ = false;
};

} // namespace tsuki