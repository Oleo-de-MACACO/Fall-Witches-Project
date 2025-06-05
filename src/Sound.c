#include "../include/Sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raylib.h"

// --- Variáveis Globais Externas de Volume (definidas em main.c) ---
extern float masterVolume;
extern float mainMenuMusicVolume;
extern float gameplayMusicVolume;
extern float battleMusicVolume;
extern float cutsceneMusicVolume;
extern float ambientNatureVolume;
extern float ambientCityVolume;
extern float ambientCaveVolume;
extern float sfxVolume;

// --- Armazenamento Interno para Recursos de Áudio ---
static Music musicTracks[MUSIC_CATEGORY_COUNT][MAX_SOUND_FILES_PER_CATEGORY];
static int numMusicTracks[MUSIC_CATEGORY_COUNT] = {0};

static Sound soundEffects[MAX_SOUND_FILES_PER_CATEGORY];
static int numSoundEffects = 0;

static Music onDemandMusicTrack;
static bool  onDemandMusicLoaded = false;

// --- Estado da Música Atualmente Tocando (Inicialização Explícita) ---
static Music currentPlayingMusic = {
    .stream = {
        .buffer = NULL,
        .processor = NULL, // Explicitamente NULL
        .sampleRate = 0,
        .sampleSize = 0,
        .channels = 0
    },
    .frameCount = 0,     // Explicitamente 0
    .looping = false,
    .ctxType = 0,        // Ou MUSIC_AUDIO_NONE se definido por Raylib
    .ctxData = NULL
};
static MusicCategory currentPlayingCategory = MUSIC_CATEGORY_COUNT;
static int currentPlayingTrackIndex = -1;
static bool isCurrentMusicPaused = false;

// Função auxiliar para obter o volume efetivo
static float GetEffectiveVolumeForCategory(MusicCategory category) {
    float categoryVolume = gameplayMusicVolume; // Padrão
    switch (category) {
        case MUSIC_CATEGORY_MAINMENU: categoryVolume = mainMenuMusicVolume; break;
        case MUSIC_CATEGORY_GAME:     categoryVolume = gameplayMusicVolume; break;
        case MUSIC_CATEGORY_BATTLE:   categoryVolume = battleMusicVolume;   break;
        case MUSIC_CATEGORY_CUTSCENE: categoryVolume = cutsceneMusicVolume; break;
        case MUSIC_CATEGORY_AMBIENT_NATURE: categoryVolume = ambientNatureVolume; break;
        case MUSIC_CATEGORY_AMBIENT_CITY:   categoryVolume = ambientCityVolume;   break;
        case MUSIC_CATEGORY_AMBIENT_CAVE:   categoryVolume = ambientCaveVolume;   break;
        default: break; 
    }
    return categoryVolume * masterVolume;
}


static void LoadMusicForCategoryInternal(const char* basePath, const char* categoryFolder, MusicCategory category) {
    if (category >= MUSIC_CATEGORY_COUNT) return;
    numMusicTracks[category] = 0;
    char directoryPath[512]; sprintf(directoryPath, "%s/%s/", basePath, categoryFolder);
    if (!DirectoryExists(directoryPath)) { /* Log warning */ return; }

    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".ogg;.mp3;.wav;.flac", false);
    for (unsigned int i = 0; i < files.count && numMusicTracks[category] < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;
        musicTracks[category][numMusicTracks[category]] = LoadMusicStream(files.paths[i]);
        if (musicTracks[category][numMusicTracks[category]].stream.buffer != NULL) {
            numMusicTracks[category]++;
        } else {
            TraceLog(LOG_WARNING, "Falha ao carregar musica: %s para cat %d", files.paths[i], category);
            // Inicialização explícita em caso de falha no carregamento
            musicTracks[category][numMusicTracks[category]] = (Music){
                .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
                .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL
            };
        }
    }
    UnloadDirectoryFiles(files);
}

static void LoadSfxFromFolder(const char* basePath, const char* sfxFolder) { 
    numSoundEffects = 0; char directoryPath[512]; sprintf(directoryPath, "%s/%s/", basePath, sfxFolder);
    if (!DirectoryExists(directoryPath)) { return; }
    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".wav;.ogg;.mp3;.flac", false);
    for (unsigned int i = 0; i < files.count && numSoundEffects < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;
        soundEffects[numSoundEffects] = LoadSound(files.paths[i]);
        if (soundEffects[numSoundEffects].frameCount > 0) { SetSoundVolume(soundEffects[numSoundEffects], sfxVolume * masterVolume); numSoundEffects++; }
        else { TraceLog(LOG_WARNING, "Falha ao carregar SFX: %s", files.paths[i]); }
    } UnloadDirectoryFiles(files);
}

void LoadGameAudio(const char* audioBasePath) {
    if (!IsAudioDeviceReady() || !audioBasePath) return;
    srand((unsigned int)time(NULL));
    LoadMusicForCategoryInternal(audioBasePath, "MainMenu", MUSIC_CATEGORY_MAINMENU);
    LoadMusicForCategoryInternal(audioBasePath, "Game", MUSIC_CATEGORY_GAME);
    LoadMusicForCategoryInternal(audioBasePath, "Battle", MUSIC_CATEGORY_BATTLE);
    LoadMusicForCategoryInternal(audioBasePath, "Cutscene", MUSIC_CATEGORY_CUTSCENE);
    LoadMusicForCategoryInternal(audioBasePath, "NatureSounds", MUSIC_CATEGORY_AMBIENT_NATURE);
    LoadMusicForCategoryInternal(audioBasePath, "CitySounds", MUSIC_CATEGORY_AMBIENT_CITY);
    LoadMusicForCategoryInternal(audioBasePath, "CaveSounds", MUSIC_CATEGORY_AMBIENT_CAVE);
    LoadSfxFromFolder(audioBasePath, "SFX");

    // Inicialização explícita para currentPlayingMusic e onDemandMusicTrack
    currentPlayingMusic = (Music){ .stream = {.buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0}, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
    onDemandMusicTrack = (Music){ .stream = {.buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0}, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
    isCurrentMusicPaused = false; onDemandMusicLoaded = false;
    TraceLog(LOG_INFO, "Audio do jogo carregado.");
}

void UnloadGameAudio(void) { 
    StopCurrentMusic();
    for (int cat = 0; cat < MUSIC_CATEGORY_COUNT; cat++) {
        for (int i = 0; i < numMusicTracks[cat]; i++) { if (musicTracks[cat][i].stream.buffer != NULL) UnloadMusicStream(musicTracks[cat][i]); }
        numMusicTracks[cat] = 0;
    }
    for (int i = 0; i < numSoundEffects; i++) { if(soundEffects[i].frameCount > 0) UnloadSound(soundEffects[i]); }
    numSoundEffects = 0;
    if (onDemandMusicLoaded && onDemandMusicTrack.stream.buffer != NULL) { UnloadMusicStream(onDemandMusicTrack); onDemandMusicLoaded = false; }
    TraceLog(LOG_INFO, "Audio do jogo descarregado.");
}

void UpdateAudioStreams(void) { 
    if (currentPlayingMusic.stream.buffer != NULL && !isCurrentMusicPaused) {
        UpdateMusicStream(currentPlayingMusic); float length = GetMusicTimeLength(currentPlayingMusic); float played = GetMusicTimePlayed(currentPlayingMusic);
        if (currentPlayingMusic.looping && length > 0.0f && played >= length - 0.05f ) { SeekMusicStream(currentPlayingMusic, 0.0f); }
        else if (!currentPlayingMusic.looping && length > 0.0f && played >= length) { StopCurrentMusic(); }
    }
}

void StopCurrentMusic(void) {
    if (currentPlayingMusic.stream.buffer != NULL ) {
        StopMusicStream(currentPlayingMusic);
        if (onDemandMusicLoaded && currentPlayingMusic.stream.buffer == onDemandMusicTrack.stream.buffer) { // Melhor checagem
             UnloadMusicStream(onDemandMusicTrack);
             onDemandMusicLoaded = false;
        }
    }
    // Inicialização explícita ao parar
    currentPlayingMusic = (Music){
        .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
        .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL
    };
    onDemandMusicTrack = (Music){ // Reseta também onDemandMusicTrack para o caso de ser a mesma
        .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
        .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL
    };
    currentPlayingTrackIndex = -1;
    currentPlayingCategory = MUSIC_CATEGORY_COUNT;
    isCurrentMusicPaused = false;
}

void PauseCurrentMusic(void) {   if (currentPlayingMusic.stream.buffer != NULL && IsMusicStreamPlaying(currentPlayingMusic)) { PauseMusicStream(currentPlayingMusic); isCurrentMusicPaused = true; }}
void ResumeCurrentMusic(void) {  if (currentPlayingMusic.stream.buffer != NULL && isCurrentMusicPaused) { ResumeMusicStream(currentPlayingMusic); isCurrentMusicPaused = false; }}
bool IsSoundModuleMusicPlaying(void) { return (currentPlayingMusic.stream.buffer != NULL && IsMusicStreamPlaying(currentPlayingMusic) && !isCurrentMusicPaused); }

void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop) {
    if (category >= MUSIC_CATEGORY_COUNT || trackIndex < 0 || trackIndex >= numMusicTracks[category]) { return; }
    StopCurrentMusic();
    currentPlayingMusic = musicTracks[category][trackIndex];
    currentPlayingMusic.looping = loop;
    if (currentPlayingMusic.stream.buffer != NULL) {
        PlayMusicStream(currentPlayingMusic);
        SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(category));
        currentPlayingCategory = category; currentPlayingTrackIndex = trackIndex; isCurrentMusicPaused = false;
    } else {
        TraceLog(LOG_WARNING, "Tentativa de tocar stream invalido: Cat %d, Faixa %d", category, trackIndex);
        // Inicialização explícita em caso de falha
        currentPlayingMusic = (Music){ .stream = { .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 }, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
    }
}

void PlayRandomMusicFromCategory(MusicCategory category, bool loop) {  if (category >= MUSIC_CATEGORY_COUNT || numMusicTracks[category] == 0) { return; } PlayMusicTrack(category, GetRandomValue(0, numMusicTracks[category] - 1), loop); }

void UpdateCurrentlyPlayingMusicVolume(void) { if (currentPlayingMusic.stream.buffer != NULL) { if (currentPlayingCategory < MUSIC_CATEGORY_COUNT) { SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(currentPlayingCategory)); } } }
void PlaySfx(int sfxIndex) {  if (sfxIndex >= 0 && sfxIndex < numSoundEffects && soundEffects[sfxIndex].frameCount > 0 ) { PlaySound(soundEffects[sfxIndex]); } }
void ApplySfxVolume(void) {  for (int i = 0; i < numSoundEffects; i++) { if (soundEffects[i].frameCount > 0) { SetSoundVolume(soundEffects[i], sfxVolume * masterVolume); } } }

bool PlayMusicFile(const char* filePathInAssetsAudio, bool loop, MusicCategory assumedCategoryForVolume) { 
    StopCurrentMusic(); char fullPath[512]; sprintf(fullPath, "assets/audio/%s", filePathInAssetsAudio);
    if (!FileExists(fullPath)) { return false; }
    onDemandMusicTrack = LoadMusicStream(fullPath);
    if (onDemandMusicTrack.stream.buffer == NULL) { onDemandMusicLoaded = false; return false; }
    onDemandMusicLoaded = true; currentPlayingMusic = onDemandMusicTrack; currentPlayingMusic.looping = loop;
    PlayMusicStream(currentPlayingMusic); SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(assumedCategoryForVolume));
    currentPlayingCategory = assumedCategoryForVolume; currentPlayingTrackIndex = -2; isCurrentMusicPaused = false;
    return true;
}
bool PlaySfxFile(const char* filePathInAssetsAudio) {
    char fullPath[512]; sprintf(fullPath, "assets/audio/%s", filePathInAssetsAudio);
    if (!FileExists(fullPath)) { return false; } Sound sfx = LoadSound(fullPath);
    if (sfx.frameCount == 0) { return false; }
    SetSoundVolume(sfx, sfxVolume * masterVolume); PlaySound(sfx); UnloadSound(sfx); return true;
}

int GetMusicTrackCount(MusicCategory category) {  if (category >= MUSIC_CATEGORY_COUNT) return 0; return numMusicTracks[category];}
int GetSfxCount(void) { return numSoundEffects; }