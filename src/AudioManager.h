#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "miniaudio.h"

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    bool Init();
    void PlaySoundFile(const char* filename, bool loop = false);
    //void PlayStream(const char* url);
    void StopSound(); // Метод для остановки звука

    void SetVolume(float newVolume);    

    bool isPlaying();

    // Публичный доступ к уровню громкости
    float volume; // Уровень громкости

private:
    ma_engine engine; // Движок звука
    ma_sound currentSound; // Сохранение текущего звука
    bool isInitialized; // Флаг инициализации
};

extern AudioManager* snd;

// Функция для создания и инициализации AudioManager
void CreateAudioManager();

#endif // AUDIOMANAGER_H
