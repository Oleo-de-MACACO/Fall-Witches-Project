#include "../include/Sound.h"
#include <stdio.h>  
#include <stdlib.h> // For rand(), srand()
#include <string.h> // For strlen (used by IsPathFile, implicitly)
#include <time.h>   // For time() in srand()
#include "raylib.h" // For all Raylib functions

// Extern global volume settings (defined in main.c, controlled by Settings.c)
extern float musicVolume;
extern float sfxVolume;

// --- Storage for Audio Assets ---
static Music mainMenuSoundtracks[MAX_SOUND_FILES_PER_CATEGORY];
static int numMainMenuSoundtracks = 0;

static Music gameSoundtracks[MAX_SOUND_FILES_PER_CATEGORY];
static int numGameSoundtracks = 0;

static Music battleSoundtracks[MAX_SOUND_FILES_PER_CATEGORY];
static int numBattleSoundtracks = 0;

static Music cutsceneSoundtracks[MAX_SOUND_FILES_PER_CATEGORY];
static int numCutsceneSoundtracks = 0;

static Sound soundEffects[MAX_SOUND_FILES_PER_CATEGORY];
static int numSoundEffects = 0;

// --- State for Currently Playing Music ---
static Music currentPlayingMusic = { 
    .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
    .frameCount = 0, 
    .looping = false, 
    .ctxType = 0, 
    .ctxData = NULL 
};
static MusicCategory currentPlayingCategory = MUSIC_CATEGORY_COUNT; 
static int currentPlayingTrackIndex = -1;
static bool isCurrentMusicPaused = false;
// isCurrentMusicLooping is now part of currentPlayingMusic.looping

// Helper to get Music array and count pointer based on category
static Music* GetMusicArrayAndCount(MusicCategory category, int** count_ptr) {
    switch (category) {
        case MUSIC_CATEGORY_MAINMENU:  if(count_ptr) *count_ptr = &numMainMenuSoundtracks; return mainMenuSoundtracks;
        case MUSIC_CATEGORY_GAME:      if(count_ptr) *count_ptr = &numGameSoundtracks;    return gameSoundtracks;
        case MUSIC_CATEGORY_BATTLE:    if(count_ptr) *count_ptr = &numBattleSoundtracks;  return battleSoundtracks;
        case MUSIC_CATEGORY_CUTSCENE:  if(count_ptr) *count_ptr = &numCutsceneSoundtracks;return cutsceneSoundtracks;
        default:                       if(count_ptr) *count_ptr = NULL;                   return NULL;
    }
}

// Loads music for a specific category from a subfolder
static void LoadMusicCategory(const char* basePath, const char* categoryFolder, MusicCategory category) {
    int* targetCountPtr = NULL;
    Music* targetArray = GetMusicArrayAndCount(category, &targetCountPtr);

    if (!targetArray || !targetCountPtr || !basePath || !categoryFolder) {
        TraceLog(LOG_ERROR, "Sound.c: Parametros invalidos para LoadMusicCategory.");
        return;
    }
    *targetCountPtr = 0; 

    char directoryPath[512]; 
    sprintf(directoryPath, "%s/%s/", basePath, categoryFolder);

    if (!DirectoryExists(directoryPath)) {
        TraceLog(LOG_WARNING, "Diretorio de musicas nao encontrado: %s", directoryPath);
        return;
    }

    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".ogg;.mp3;.wav;.flac", false);
    TraceLog(LOG_INFO, "Carregando musicas de: %s (Encontrados %d arquivos)", directoryPath, files.count);

    for (unsigned int i = 0; i < files.count && *targetCountPtr < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;

        targetArray[*targetCountPtr] = LoadMusicStream(files.paths[i]);
        if (targetArray[*targetCountPtr].stream.buffer != NULL && targetArray[*targetCountPtr].frameCount > 0) {
            TraceLog(LOG_INFO, "Musica carregada: %s", files.paths[i]);
            (*targetCountPtr)++;
        } else {
            TraceLog(LOG_WARNING, "Falha ao carregar musica ou musica vazia: %s", files.paths[i]);
            UnloadMusicStream(targetArray[*targetCountPtr]); // Unload if failed partially
            targetArray[*targetCountPtr] = (Music){ 
                .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
                .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL 
            }; // Reset
        }
    }
    UnloadDirectoryFiles(files);
}

// Loads sound effects from a subfolder
static void LoadSfxCategory(const char* basePath, const char* categoryFolder) {
    if (!basePath || !categoryFolder) {
        TraceLog(LOG_ERROR, "Sound.c: Parametros invalidos para LoadSfxCategory.");
        return;
    }
    numSoundEffects = 0; 
    char directoryPath[512];
    sprintf(directoryPath, "%s/%s/", basePath, categoryFolder);

    if (!DirectoryExists(directoryPath)) {
        TraceLog(LOG_WARNING, "Diretorio de SFX nao encontrado: %s", directoryPath);
        return;
    }

    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".wav;.ogg;.mp3;.flac", false);
    TraceLog(LOG_INFO, "Carregando SFX de: %s (Encontrados %d arquivos)", directoryPath, files.count);

    for (unsigned int i = 0; i < files.count && numSoundEffects < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;

        soundEffects[numSoundEffects] = LoadSound(files.paths[i]);
        if (soundEffects[numSoundEffects].frameCount > 0) { 
            SetSoundVolume(soundEffects[numSoundEffects], sfxVolume); 
            numSoundEffects++;
        } else {
            TraceLog(LOG_WARNING, "Falha ao carregar SFX: %s", files.paths[i]);
        }
    }
    UnloadDirectoryFiles(files);
}

void LoadGameAudio(const char* audioBasePath) {
    if (!IsAudioDeviceReady()) { TraceLog(LOG_WARNING, "Dispositivo de audio nao pronto. Audio nao sera carregado."); return; }
    if (!audioBasePath) { TraceLog(LOG_ERROR, "audioBasePath e NULL em LoadGameAudio."); return; }

    srand((unsigned int)time(NULL)); 

    LoadMusicCategory(audioBasePath, "MainMenu", MUSIC_CATEGORY_MAINMENU);
    LoadMusicCategory(audioBasePath, "Game", MUSIC_CATEGORY_GAME);
    LoadMusicCategory(audioBasePath, "Battle", MUSIC_CATEGORY_BATTLE);
    LoadMusicCategory(audioBasePath, "Cutscene", MUSIC_CATEGORY_CUTSCENE);
    LoadSfxCategory(audioBasePath, "SFX"); 

    currentPlayingMusic = (Music){ 
        .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
        .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL 
    }; 
    isCurrentMusicPaused = false;
    TraceLog(LOG_INFO, "Audio do jogo carregado.");
}

void UnloadGameAudio(void) {
    StopCurrentMusic(); 

    for (int i = 0; i < numMainMenuSoundtracks; i++) if(mainMenuSoundtracks[i].stream.buffer != NULL) UnloadMusicStream(mainMenuSoundtracks[i]);
    for (int i = 0; i < numGameSoundtracks; i++)     if(gameSoundtracks[i].stream.buffer != NULL) UnloadMusicStream(gameSoundtracks[i]);
    for (int i = 0; i < numBattleSoundtracks; i++)   if(battleSoundtracks[i].stream.buffer != NULL) UnloadMusicStream(battleSoundtracks[i]);
    for (int i = 0; i < numCutsceneSoundtracks; i++) if(cutsceneSoundtracks[i].stream.buffer != NULL) UnloadMusicStream(cutsceneSoundtracks[i]);
    
    for (int i = 0; i < numSoundEffects; i++) {
        if(soundEffects[i].frameCount > 0) UnloadSound(soundEffects[i]);
    }
    
    numMainMenuSoundtracks = 0; numGameSoundtracks = 0; numBattleSoundtracks = 0; numCutsceneSoundtracks = 0;
    numSoundEffects = 0;
    TraceLog(LOG_INFO, "Audio do jogo descarregado.");
}

void UpdateAudioStreams(void) {
    if (currentPlayingMusic.stream.buffer != NULL && !isCurrentMusicPaused) {
        UpdateMusicStream(currentPlayingMusic);
        
        float length = GetMusicTimeLength(currentPlayingMusic);
        float played = GetMusicTimePlayed(currentPlayingMusic);

        if (currentPlayingMusic.looping && length > 0.0f && played >= length - 0.05f ) { 
            SeekMusicStream(currentPlayingMusic, 0.0f); 
        } else if (!currentPlayingMusic.looping && length > 0.0f && played >= length) { 
             StopCurrentMusic(); 
        }
    }
}

void StopCurrentMusic(void) {
    if (currentPlayingMusic.stream.buffer != NULL ) {
        StopMusicStream(currentPlayingMusic);
    }
    currentPlayingMusic = (Music){ 
        .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
        .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL 
    }; 
    currentPlayingTrackIndex = -1;
    currentPlayingCategory = MUSIC_CATEGORY_COUNT; 
    isCurrentMusicPaused = false;
}

void PauseCurrentMusic(void) {
    if (currentPlayingMusic.stream.buffer != NULL && IsMusicStreamPlaying(currentPlayingMusic)) {
        PauseMusicStream(currentPlayingMusic);
        isCurrentMusicPaused = true;
    }
}

void ResumeCurrentMusic(void) {
    if (currentPlayingMusic.stream.buffer != NULL && isCurrentMusicPaused) {
        ResumeMusicStream(currentPlayingMusic);
        isCurrentMusicPaused = false;
    }
}

bool IsSoundModuleMusicPlaying(void) {
    return (currentPlayingMusic.stream.buffer != NULL && 
            IsMusicStreamPlaying(currentPlayingMusic) && 
            !isCurrentMusicPaused);
}

void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop) {
    int* countPtr = NULL;
    Music* musicList = GetMusicArrayAndCount(category, &countPtr);
    int count = (countPtr != NULL) ? *countPtr : 0;

    if (musicList != NULL && trackIndex >= 0 && trackIndex < count) {
        StopCurrentMusic(); 
        currentPlayingMusic = musicList[trackIndex];
        currentPlayingMusic.looping = loop; 

        if (currentPlayingMusic.stream.buffer != NULL && currentPlayingMusic.frameCount > 0) {
            PlayMusicStream(currentPlayingMusic);
            SetMusicVolume(currentPlayingMusic, musicVolume); 
            currentPlayingCategory = category;
            currentPlayingTrackIndex = trackIndex;
            isCurrentMusicPaused = false;
        } else {
            TraceLog(LOG_WARNING, "Tentativa de tocar stream de musica invalida ou nao pronta: categoria %d, track %d", category, trackIndex);
            currentPlayingMusic = (Music){ .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 }, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
        }
    } else {
        TraceLog(LOG_WARNING, "Indice de track invalido %d para categoria %d (total: %d)", trackIndex, category, count);
    }
}

void PlayRandomMusicFromCategory(MusicCategory category, bool loop) {
    int* countPtr = NULL;
    GetMusicArrayAndCount(category, &countPtr);
    int count = (countPtr != NULL) ? *countPtr : 0;

    if (count > 0) {
        int randomIndex = GetRandomValue(0, count - 1);
        PlayMusicTrack(category, randomIndex, loop);
    } else {
        TraceLog(LOG_WARNING, "Nenhuma track na categoria %d para tocar aleatoriamente.", category);
    }
}

void ApplyMusicVolume(void) {
    if (currentPlayingMusic.stream.buffer != NULL) {
        SetMusicVolume(currentPlayingMusic, musicVolume);
    }
}

void PlaySfx(int sfxIndex) {
    if (sfxIndex >= 0 && sfxIndex < numSoundEffects && soundEffects[sfxIndex].frameCount > 0 ) {
        PlaySound(soundEffects[sfxIndex]); 
    } else {
        TraceLog(LOG_WARNING, "Indice SFX invalido ou som nao pronto: %d (total: %d)", sfxIndex, numSoundEffects);
    }
}

void ApplySfxVolume(void) {
    for (int i = 0; i < numSoundEffects; i++) {
        if (soundEffects[i].frameCount > 0) { 
            SetSoundVolume(soundEffects[i], sfxVolume);
        }
    }
}

int GetMusicTrackCount(MusicCategory category) {
    int* countPtr = NULL;
    GetMusicArrayAndCount(category, &countPtr);
    return (countPtr != NULL) ? *countPtr : 0;
}

int GetSfxCount(void) {
    return numSoundEffects;
}