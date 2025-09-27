#include "tsuki/audio.hpp"

namespace tsuki {

// Sound implementation
Sound::Sound(const std::string& filename) {
    load(filename);
}

Sound::~Sound() {
    unload();
}

Sound::Sound(Sound&& other) noexcept : chunk_(other.chunk_) {
    other.chunk_ = nullptr;
}

Sound& Sound::operator=(Sound&& other) noexcept {
    if (this != &other) {
        unload();
        chunk_ = other.chunk_;
        other.chunk_ = nullptr;
    }
    return *this;
}

bool Sound::load(const std::string& filename) {
    unload();
#ifdef TSUKI_HAS_SDL_MIXER
    chunk_ = Mix_LoadWAV(filename.c_str());
    return chunk_ != nullptr;
#else
    (void)filename; // Suppress unused parameter warning
    return false;
#endif
}

void Sound::unload() {
#ifdef TSUKI_HAS_SDL_MIXER
    if (chunk_) {
        Mix_FreeChunk(chunk_);
        chunk_ = nullptr;
    }
#endif
}

// Music implementation
Music::Music(const std::string& filename) {
    load(filename);
}

Music::~Music() {
    unload();
}

Music::Music(Music&& other) noexcept : music_(other.music_) {
    other.music_ = nullptr;
}

Music& Music::operator=(Music&& other) noexcept {
    if (this != &other) {
        unload();
        music_ = other.music_;
        other.music_ = nullptr;
    }
    return *this;
}

bool Music::load(const std::string& filename) {
    unload();
#ifdef TSUKI_HAS_SDL_MIXER
    music_ = Mix_LoadMUS(filename.c_str());
    return music_ != nullptr;
#else
    (void)filename; // Suppress unused parameter warning
    return false;
#endif
}

void Music::unload() {
#ifdef TSUKI_HAS_SDL_MIXER
    if (music_) {
        Mix_FreeMusic(music_);
        music_ = nullptr;
    }
#endif
}

// Audio implementation
bool Audio::init() {
    return true; // SDL_mixer is already initialized in Engine::init
}

void Audio::shutdown() {
    stopMusic();
    stop();
}

void Audio::play(const Sound& sound, int loops) {
#ifdef TSUKI_HAS_SDL_MIXER
    if (sound.isValid()) {
        Mix_PlayChannel(-1, sound.getChunk(), loops);
    }
#else
    (void)sound;
    (void)loops;
#endif
}

void Audio::pause() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Pause(-1);
#endif
}

void Audio::resume() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Resume(-1);
#endif
}

void Audio::stop() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_HaltChannel(-1);
#endif
}

void Audio::playMusic(const Music& music, int loops) {
#ifdef TSUKI_HAS_SDL_MIXER
    if (music.isValid()) {
        Mix_PlayMusic(music.getMusic(), loops);
    }
#else
    (void)music;
    (void)loops;
#endif
}

void Audio::pauseMusic() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_PauseMusic();
#endif
}

void Audio::resumeMusic() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_ResumeMusic();
#endif
}

void Audio::stopMusic() {
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_HaltMusic();
#endif
}

void Audio::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_Volume(-1, static_cast<int>(volume_ * MIX_MAX_VOLUME));
#endif
}

float Audio::getVolume() const {
    return volume_;
}

void Audio::setMusicVolume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
#ifdef TSUKI_HAS_SDL_MIXER
    Mix_VolumeMusic(static_cast<int>(music_volume_ * MIX_MAX_VOLUME));
#endif
}

float Audio::getMusicVolume() const {
    return music_volume_;
}

bool Audio::isPlaying() const {
#ifdef TSUKI_HAS_SDL_MIXER
    return Mix_Playing(-1) > 0;
#else
    return false;
#endif
}

bool Audio::isMusicPlaying() const {
#ifdef TSUKI_HAS_SDL_MIXER
    return Mix_PlayingMusic() != 0;
#else
    return false;
#endif
}

bool Audio::isMusicPaused() const {
#ifdef TSUKI_HAS_SDL_MIXER
    return Mix_PausedMusic() != 0;
#else
    return false;
#endif
}

} // namespace tsuki