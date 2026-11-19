#include "main.h"
#include "AudioManager.h"
#include <iostream>

AudioManager* snd = nullptr;

AudioManager::AudioManager() : volume(1.0f) { // Инициализация громкости по умолчанию
}

AudioManager::~AudioManager() {
    // Освобождение ресурсов
    ma_sound_uninit(&currentSound); // Освобождение ресурсов звука
    ma_engine_uninit(&engine);
}

bool AudioManager::Init() {
    traceLastFunc("bool AudioManager::Init();");

    // Проверка, инициализирован ли уже движок
    if (isInitialized) {
        Log("[Warning]: Движок звука уже инициализирован!");
        return true; // Возвращаем true, если инициализация уже выполнена
    }

    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        Log("[ErrorCode]: #1, Не удалось инициализировать движок звука!");
        return false; // Возвращаем false в случае ошибки
    }
    else {
        Log("{00DD00}AudioEngine() is init success");
        SetVolume(volume); // Установка начального уровня громкости
        isInitialized = true; // Устанавливаем флаг инициализации
        return true; // Возвращаем true при успешной инициализации
    }
}

// Функция для создания и инициализации AudioManager
void CreateAudioManager() {
    if (snd == nullptr) {
        snd = new AudioManager();
        if (!snd->Init()) {
            delete snd; // Освобождаем память в случае ошибки
            snd = nullptr;
        }
    }
}

void AudioManager::PlaySoundFile(const char* filename, bool loop) {
    traceLastFunc("void AudioManager::PlaySoundFile(const char* filename, bool loop);");
    ma_result result;    

    // Инициализация звука
    result = ma_sound_init_from_file(&engine, filename, 0, NULL, NULL, &currentSound);

    if (result != MA_SUCCESS) {
        Log("[ErrorCode]: #2, Не удалось инициализировать звук!");
        return; // Выход из функции, если инициализация не удалась
    }

    // Установка зацикливания, если это необходимо
    ma_sound_set_looping(&currentSound, loop);

    // Воспроизведение звука
    result = ma_sound_start(&currentSound);
    if (result != MA_SUCCESS) {
        Log("[ErrorCode]: #3, Не удалось воспроизвести звук!");
    }
}

bool AudioManager::isPlaying() {
    traceLastFunc("bool AudioManager::isPlaying();");
    bool playing = ma_sound_is_playing(&currentSound) == MA_TRUE;
    traceLastFunc(("bool AudioManager::isPlaying(); # " + std::to_string(playing)).c_str());

    return playing;
}

//void AudioManager::PlayStream(const char* url) {
//    traceLastFunc("void AudioManager::PlayStream(const char* url);");
//    ma_result result = ma_engine_play_sound(&engine, url, NULL);
//    if (result != MA_SUCCESS) {        
//        Log("[ErrorCode]: #4, Не удалось воспроизвести интернет-радио!");
//    }
//}

void AudioManager::SetVolume(float newVolume) {
    traceLastFunc("AudioManager::SetVolume(float newVolume);");
    volume = newVolume;
    ma_engine_set_volume(&engine, volume); // Установка громкости
}

void AudioManager::StopSound() {
    ma_sound_stop(&currentSound); // Остановка звука
    ma_sound_uninit(&currentSound); // Освобождение ресурсов
}
