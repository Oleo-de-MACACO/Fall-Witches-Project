#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "raylib.h"

#define MAX_SOUND_FILES_PER_CATEGORY 500

typedef enum {
    MUSIC_CATEGORY_MAINMENU,
    MUSIC_CATEGORY_GAME,
    MUSIC_CATEGORY_BATTLE,
    MUSIC_CATEGORY_CUTSCENE,
    MUSIC_CATEGORY_COUNT 
} MusicCategory;

// --- Global Audio Control Functions ---
void LoadGameAudio(const char* audioBasePath);
void UnloadGameAudio(void);
void UpdateAudioStreams(void); // MUST be called every frame

// --- Music Playback Functions ---
void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop);
void PlayRandomMusicFromCategory(MusicCategory category, bool loop);
void StopCurrentMusic(void);
void PauseCurrentMusic(void);
void ResumeCurrentMusic(void);
bool IsSoundModuleMusicPlaying(void); // << NOVA FUNÇÃO
void ApplyMusicVolume(void); 

// --- Sound Effect Playback Functions ---
void PlaySfx(int sfxIndex);
void ApplySfxVolume(void); 

// --- Getter Functions ---
int GetMusicTrackCount(MusicCategory category);
int GetSfxCount(void);

#endif // SOUND_MANAGER_H