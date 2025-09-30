#define MINIAUDIO_IMPLEMENTATION
#include "tsuki/audio.hpp"
#include <spdlog/spdlog.h>

namespace tsuki {

// Sound implementation
Sound::Sound(const std::string& filename) {
    load(filename);
}

Sound::~Sound() {
    unload();
}

Sound::Sound(Sound&& other) noexcept
    : decoder_(std::move(other.decoder_)),
      filename_(std::move(other.filename_)),
      is_loaded_(other.is_loaded_) {
    other.is_loaded_ = false;
}

Sound& Sound::operator=(Sound&& other) noexcept {
    if (this != &other) {
        unload();
        decoder_ = std::move(other.decoder_);
        filename_ = std::move(other.filename_);
        is_loaded_ = other.is_loaded_;
        other.is_loaded_ = false;
    }
    return *this;
}

bool Sound::load(const std::string& filename) {
    unload();

    decoder_ = std::make_unique<ma_decoder>();
    ma_result result = ma_decoder_init_file(filename.c_str(), nullptr, decoder_.get());

    if (result != MA_SUCCESS) {
        spdlog::error("Failed to load sound: {}", filename);
        decoder_.reset();
        return false;
    }

    filename_ = filename;
    is_loaded_ = true;
    return true;
}

void Sound::unload() {
    if (is_loaded_ && decoder_) {
        ma_decoder_uninit(decoder_.get());
        decoder_.reset();
        is_loaded_ = false;
    }
}

// Music implementation
Music::Music(const std::string& filename) {
    load(filename);
}

Music::~Music() {
    unload();
}

Music::Music(Music&& other) noexcept
    : decoder_(std::move(other.decoder_)),
      filename_(std::move(other.filename_)),
      is_loaded_(other.is_loaded_) {
    other.is_loaded_ = false;
}

Music& Music::operator=(Music&& other) noexcept {
    if (this != &other) {
        unload();
        decoder_ = std::move(other.decoder_);
        filename_ = std::move(other.filename_);
        is_loaded_ = other.is_loaded_;
        other.is_loaded_ = false;
    }
    return *this;
}

bool Music::load(const std::string& filename) {
    unload();

    decoder_ = std::make_unique<ma_decoder>();
    ma_result result = ma_decoder_init_file(filename.c_str(), nullptr, decoder_.get());

    if (result != MA_SUCCESS) {
        spdlog::error("Failed to load music: {}", filename);
        decoder_.reset();
        return false;
    }

    filename_ = filename;
    is_loaded_ = true;
    return true;
}

void Music::unload() {
    if (is_loaded_ && decoder_) {
        ma_decoder_uninit(decoder_.get());
        decoder_.reset();
        is_loaded_ = false;
    }
}

// Audio implementation
Audio::Audio() = default;

Audio::~Audio() {
    shutdown();
}

bool Audio::init() {
    if (initialized_) {
        return true;
    }

    engine_ = std::make_unique<ma_engine>();
    ma_result result = ma_engine_init(nullptr, engine_.get());

    if (result != MA_SUCCESS) {
        spdlog::error("Failed to initialize audio engine");
        engine_.reset();
        return false;
    }

    initialized_ = true;
    spdlog::info("Audio engine initialized with miniaudio");
    return true;
}

void Audio::shutdown() {
    if (!initialized_) {
        return;
    }

    stopMusic();
    stop();

    if (engine_) {
        ma_engine_uninit(engine_.get());
        engine_.reset();
    }

    initialized_ = false;
}

void Audio::play(const Sound& sound, int loops) {
    if (!initialized_ || !sound.isValid()) {
        return;
    }

    // Stop current sound if any
    if (sound_loaded_) {
        ma_sound_uninit(&current_sound_);
        sound_loaded_ = false;
    }

    // Initialize and play new sound
    ma_result result = ma_sound_init_from_file(
        engine_.get(),
        sound.getFilename().c_str(),
        0,
        nullptr,
        nullptr,
        &current_sound_
    );

    if (result != MA_SUCCESS) {
        spdlog::error("Failed to play sound: {}", sound.getFilename());
        return;
    }

    // Set looping
    ma_sound_set_looping(&current_sound_, loops != 0);

    // Set volume
    ma_sound_set_volume(&current_sound_, volume_);

    // Start playback
    ma_sound_start(&current_sound_);
    sound_loaded_ = true;
}

void Audio::pause() {
    if (initialized_ && sound_loaded_) {
        ma_sound_stop(&current_sound_);
    }
}

void Audio::resume() {
    if (initialized_ && sound_loaded_) {
        ma_sound_start(&current_sound_);
    }
}

void Audio::stop() {
    if (initialized_ && sound_loaded_) {
        ma_sound_stop(&current_sound_);
        ma_sound_uninit(&current_sound_);
        sound_loaded_ = false;
    }
}

void Audio::playMusic(const Music& music, int loops) {
    if (!initialized_ || !music.isValid()) {
        return;
    }

    // Stop current music if any
    if (music_loaded_) {
        ma_sound_uninit(&current_music_);
        music_loaded_ = false;
    }

    // Initialize and play new music
    ma_result result = ma_sound_init_from_file(
        engine_.get(),
        music.getFilename().c_str(),
        MA_SOUND_FLAG_STREAM,  // Stream music files for better memory usage
        nullptr,
        nullptr,
        &current_music_
    );

    if (result != MA_SUCCESS) {
        spdlog::error("Failed to play music: {}", music.getFilename());
        return;
    }

    // Set looping (-1 means infinite loop)
    ma_sound_set_looping(&current_music_, loops != 0);

    // Set volume
    ma_sound_set_volume(&current_music_, music_volume_);

    // Start playback
    ma_sound_start(&current_music_);
    music_loaded_ = true;
    music_paused_ = false;
}

void Audio::pauseMusic() {
    if (initialized_ && music_loaded_ && !music_paused_) {
        ma_sound_stop(&current_music_);
        music_paused_ = true;
    }
}

void Audio::resumeMusic() {
    if (initialized_ && music_loaded_ && music_paused_) {
        ma_sound_start(&current_music_);
        music_paused_ = false;
    }
}

void Audio::stopMusic() {
    if (initialized_ && music_loaded_) {
        ma_sound_stop(&current_music_);
        ma_sound_uninit(&current_music_);
        music_loaded_ = false;
        music_paused_ = false;
    }
}

void Audio::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (initialized_ && sound_loaded_) {
        ma_sound_set_volume(&current_sound_, volume_);
    }
}

float Audio::getVolume() const {
    return volume_;
}

void Audio::setMusicVolume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (initialized_ && music_loaded_) {
        ma_sound_set_volume(&current_music_, music_volume_);
    }
}

float Audio::getMusicVolume() const {
    return music_volume_;
}

bool Audio::isPlaying() const {
    if (!initialized_ || !sound_loaded_) {
        return false;
    }
    return ma_sound_is_playing(&current_sound_);
}

bool Audio::isMusicPlaying() const {
    if (!initialized_ || !music_loaded_) {
        return false;
    }
    return ma_sound_is_playing(&current_music_) && !music_paused_;
}

bool Audio::isMusicPaused() const {
    return music_paused_;
}

} // namespace tsuki