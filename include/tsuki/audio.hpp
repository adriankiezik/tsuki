#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>

#ifdef TSUKI_HAS_SDL_MIXER
#include <SDL3_mixer/SDL_mixer.h>
#endif

namespace tsuki {

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

    bool isValid() const {
#ifdef TSUKI_HAS_SDL_MIXER
        return chunk_ != nullptr;
#else
        return false;
#endif
    }

#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Chunk* getChunk() const { return chunk_; }
#endif

private:
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Chunk* chunk_ = nullptr;
#else
    void* chunk_ = nullptr; // Placeholder when SDL_mixer is not available
#endif
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

    bool isValid() const {
#ifdef TSUKI_HAS_SDL_MIXER
        return music_ != nullptr;
#else
        return false;
#endif
    }

#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Music* getMusic() const { return music_; }
#endif

private:
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Music* music_ = nullptr;
#else
    void* music_ = nullptr; // Placeholder when SDL_mixer is not available
#endif
};

class Audio {
public:
    Audio() = default;
    ~Audio() = default;

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
    float volume_ = 1.0f;
    float music_volume_ = 1.0f;
};

} // namespace tsuki