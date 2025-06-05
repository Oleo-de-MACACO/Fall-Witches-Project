#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "raylib.h"

#define MAX_SOUND_FILES_PER_CATEGORY 50

typedef enum {
    MUSIC_CATEGORY_MAINMENU, MUSIC_CATEGORY_GAME, MUSIC_CATEGORY_BATTLE, MUSIC_CATEGORY_CUTSCENE,
    MUSIC_CATEGORY_AMBIENT_NATURE, MUSIC_CATEGORY_AMBIENT_CITY, MUSIC_CATEGORY_AMBIENT_CAVE,
    MUSIC_CATEGORY_COUNT
} MusicCategory;

// --- Variáveis Globais de Volume (definidas em main.c) ---
extern float masterVolume; // *** NOVO ***
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
void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop);
void PlayRandomMusicFromCategory(MusicCategory category, bool loop);
void StopCurrentMusic(void);
void PauseCurrentMusic(void);
void ResumeCurrentMusic(void);
bool IsSoundModuleMusicPlaying(void);
void UpdateCurrentlyPlayingMusicVolume(void);
void PlaySfx(int sfxIndex);
void ApplySfxVolume(void);
bool PlayMusicFile(const char* filePathInAssetsAudio, bool loop, MusicCategory assumedCategoryForVolume);
bool PlaySfxFile(const char* filePathInAssetsAudio);
int GetMusicTrackCount(MusicCategory category);
int GetSfxCount(void);

#endif // SOUND_MANAGER_H