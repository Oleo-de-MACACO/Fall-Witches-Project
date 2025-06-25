#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <stdbool.h>

#define MAX_SOUND_FILES_PER_CATEGORY 50
#define MAX_PATH_LENGTH 128 // Comprimento máximo para nomes de arquivo

// Enumeração expandida para incluir categorias de som ambiente
typedef enum {
    MUSIC_CATEGORY_MAINMENU,
    MUSIC_CATEGORY_GAME,
    MUSIC_CATEGORY_BATTLE,
    MUSIC_CATEGORY_CUTSCENE,
    MUSIC_CATEGORY_AMBIENT_NATURE,
    MUSIC_CATEGORY_AMBIENT_CITY,
    MUSIC_CATEGORY_AMBIENT_CAVE,
    MUSIC_CATEGORY_COUNT // Mantém a contagem total de categorias de música
} MusicCategory;

// --- Variáveis Globais de Volume (definidas em main.c) ---
extern float masterVolume;
extern float mainMenuMusicVolume;
extern float gameplayMusicVolume;
extern float battleMusicVolume;
extern float cutsceneMusicVolume;
extern float ambientNatureVolume;
extern float ambientCityVolume;
extern float ambientCaveVolume;
extern float sfxVolume;

// --- Funções de Controle de Áudio ---
void LoadGameAudio(const char* audioBasePath);
void UnloadGameAudio(void);
void UpdateAudioStreams(void);

// --- Funções de Reprodução de Música ---
void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop);
void PlayRandomMusicFromCategory(MusicCategory category, bool loop);
void StopCurrentMusic(void);
void PauseCurrentMusic(void);
void ResumeCurrentMusic(void);
bool IsSoundModuleMusicPlaying(void);
void UpdateCurrentlyPlayingMusicVolume(void);

// --- Funções de Reprodução de Efeitos Sonoros (SFX) ---
void PlaySfx(int sfxIndex);
void ApplySfxVolume(void);

// --- Funções para Tocar Arquivos Específicos ---
bool PlayMusicFile(const char* filePathInAssetsAudio, bool loop, MusicCategory assumedCategoryForVolume);
bool PlaySfxFile(const char* filePathInAssetsAudio);

// --- Funções Getter ---
int GetMusicTrackCount(MusicCategory category);
int GetSfxCount(void);
int Sound_GetMusicIndexByName(MusicCategory category, const char* songName);

#endif // SOUND_MANAGER_H