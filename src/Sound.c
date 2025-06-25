#include "../include/Sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raylib.h"

/**
 * @brief Estrutura interna para associar um objeto Music da Raylib com seu nome de arquivo.
 * Isso é necessário para encontrar músicas pelo nome, como solicitado pelo sistema de eventos.
 */
typedef struct {
    Music music;
    char fileName[MAX_PATH_LENGTH];
} MusicTrack;

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
static MusicTrack musicTracks[MUSIC_CATEGORY_COUNT][MAX_SOUND_FILES_PER_CATEGORY];
static int numMusicTracks[MUSIC_CATEGORY_COUNT] = {0};

static Sound soundEffects[MAX_SOUND_FILES_PER_CATEGORY];
static int numSoundEffects = 0;

static Music onDemandMusicTrack;
static bool  onDemandMusicLoaded = false;

// --- Estado da Música Atualmente Tocando ---
// *** CORRIGIDO: Inicializador explícito e completo para a struct Music para evitar warnings ***
static Music currentPlayingMusic = { .stream = {0}, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
static MusicCategory currentPlayingCategory = MUSIC_CATEGORY_COUNT;
static int currentPlayingTrackIndex = -1;
static bool isCurrentMusicPaused = false;


/**
 * @brief Obtém o volume efetivo para uma categoria de música, aplicando o volume mestre.
 * @param category A categoria da música.
 * @return O volume final (0.0f a 1.0f) a ser aplicado.
 */
static float GetEffectiveVolumeForCategory(MusicCategory category) {
    float categoryVolume = gameplayMusicVolume;
    switch (category) {
        case MUSIC_CATEGORY_MAINMENU:       categoryVolume = mainMenuMusicVolume; break;
        case MUSIC_CATEGORY_GAME:           categoryVolume = gameplayMusicVolume; break;
        case MUSIC_CATEGORY_BATTLE:         categoryVolume = battleMusicVolume;   break;
        case MUSIC_CATEGORY_CUTSCENE:       categoryVolume = cutsceneMusicVolume; break;
        case MUSIC_CATEGORY_AMBIENT_NATURE: categoryVolume = ambientNatureVolume; break;
        case MUSIC_CATEGORY_AMBIENT_CITY:   categoryVolume = ambientCityVolume;   break;
        case MUSIC_CATEGORY_AMBIENT_CAVE:   categoryVolume = ambientCaveVolume;   break;
        default: break;
    }
    return categoryVolume * masterVolume;
}

/**
 * @brief Carrega arquivos de música de uma pasta específica para uma categoria.
 * @param basePath O caminho base da pasta de áudio (ex: "assets/audio").
 * @param categoryFolder O nome da subpasta (ex: "MainMenu", "NatureSounds").
 * @param category O enum MusicCategory correspondente.
 */
static void LoadMusicForCategoryInternal(const char* basePath, const char* categoryFolder, MusicCategory category) {
    if (category >= MUSIC_CATEGORY_COUNT) return;
    numMusicTracks[category] = 0;

    char directoryPath[512];
    sprintf(directoryPath, "%s/%s/", basePath, categoryFolder);

    if (!DirectoryExists(directoryPath)) {
        TraceLog(LOG_WARNING, "Diretorio de musicas para categoria %d nao encontrado: %s", category, directoryPath);
        return;
    }

    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".ogg;.mp3;.wav;.flac", false);
    for (unsigned int i = 0; i < files.count && numMusicTracks[category] < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;

        musicTracks[category][numMusicTracks[category]].music = LoadMusicStream(files.paths[i]);
        if (musicTracks[category][numMusicTracks[category]].music.stream.buffer != NULL) {
            strncpy(musicTracks[category][numMusicTracks[category]].fileName, GetFileName(files.paths[i]), MAX_PATH_LENGTH - 1);
            musicTracks[category][numMusicTracks[category]].fileName[MAX_PATH_LENGTH - 1] = '\0';
            numMusicTracks[category]++;
        }
    }
    UnloadDirectoryFiles(files);
}

static void LoadSfxFromFolder(const char* basePath, const char* sfxFolder) {
    numSoundEffects = 0;
    char directoryPath[512]; sprintf(directoryPath, "%s/%s/", basePath, sfxFolder);
    if (!DirectoryExists(directoryPath)) { return; }
    FilePathList files = LoadDirectoryFilesEx(directoryPath, ".wav;.ogg;.mp3;.flac", false);
    for (unsigned int i = 0; i < files.count && numSoundEffects < MAX_SOUND_FILES_PER_CATEGORY; i++) {
        if (!IsPathFile(files.paths[i])) continue;
        soundEffects[numSoundEffects] = LoadSound(files.paths[i]);
        if (soundEffects[numSoundEffects].frameCount > 0) { numSoundEffects++; }
    }
    UnloadDirectoryFiles(files);
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
    TraceLog(LOG_INFO, "Audio do jogo carregado.");
}

void UnloadGameAudio(void) {
    StopCurrentMusic();
    for (int cat = 0; cat < MUSIC_CATEGORY_COUNT; cat++) {
        for (int i = 0; i < numMusicTracks[cat]; i++) {
            if (musicTracks[cat][i].music.stream.buffer != NULL) UnloadMusicStream(musicTracks[cat][i].music);
        }
        numMusicTracks[cat] = 0;
    }
    for (int i = 0; i < numSoundEffects; i++) { if(soundEffects[i].frameCount > 0) UnloadSound(soundEffects[i]); }
    numSoundEffects = 0;
    TraceLog(LOG_INFO, "Audio do jogo descarregado.");
}

void UpdateAudioStreams(void) {
    if (currentPlayingMusic.stream.buffer != NULL && !isCurrentMusicPaused) {
        UpdateMusicStream(currentPlayingMusic);
        float length = GetMusicTimeLength(currentPlayingMusic); float played = GetMusicTimePlayed(currentPlayingMusic);
        if (currentPlayingMusic.looping && played >= length) {
            SeekMusicStream(currentPlayingMusic, 0.0f);
        }
    }
}

void StopCurrentMusic(void) {
    if (currentPlayingMusic.stream.buffer != NULL ) { StopMusicStream(currentPlayingMusic); }
    currentPlayingMusic = (Music){ .stream = {0}, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
    currentPlayingTrackIndex = -1;
    currentPlayingCategory = MUSIC_CATEGORY_COUNT; isCurrentMusicPaused = false;
}
void PauseCurrentMusic(void) { if (currentPlayingMusic.stream.buffer != NULL && IsMusicStreamPlaying(currentPlayingMusic)) { PauseMusicStream(currentPlayingMusic); isCurrentMusicPaused = true; } }
void ResumeCurrentMusic(void) { if (currentPlayingMusic.stream.buffer != NULL && isCurrentMusicPaused) { ResumeMusicStream(currentPlayingMusic); isCurrentMusicPaused = false; } }
bool IsSoundModuleMusicPlaying(void) { return (currentPlayingMusic.stream.buffer != NULL && IsMusicStreamPlaying(currentPlayingMusic) && !isCurrentMusicPaused); }
void UpdateCurrentlyPlayingMusicVolume(void) { if (currentPlayingMusic.stream.buffer != NULL && currentPlayingCategory < MUSIC_CATEGORY_COUNT) { SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(currentPlayingCategory)); } }
void ApplySfxVolume(void) { for (int i = 0; i < numSoundEffects; i++) { if (soundEffects[i].frameCount > 0) { SetSoundVolume(soundEffects[i], sfxVolume * masterVolume); } } }
void PlaySfx(int sfxIndex) { if (sfxIndex >= 0 && sfxIndex < numSoundEffects && soundEffects[sfxIndex].frameCount > 0 ) { SetSoundVolume(soundEffects[sfxIndex], sfxVolume * masterVolume); PlaySound(soundEffects[sfxIndex]); } }
int GetSfxCount(void) { return numSoundEffects; }
int GetMusicTrackCount(MusicCategory category) { if (category >= MUSIC_CATEGORY_COUNT) return 0; return numMusicTracks[category]; }

void PlayMusicTrack(MusicCategory category, int trackIndex, bool loop) {
    if (category >= MUSIC_CATEGORY_COUNT || trackIndex < 0 || trackIndex >= numMusicTracks[category]) return;
    StopCurrentMusic();
    currentPlayingMusic = musicTracks[category][trackIndex].music;
    currentPlayingMusic.looping = loop;
    if (currentPlayingMusic.stream.buffer != NULL) {
        PlayMusicStream(currentPlayingMusic);
        SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(category));
        currentPlayingCategory = category; currentPlayingTrackIndex = trackIndex; isCurrentMusicPaused = false;
    } else {
        currentPlayingMusic = (Music){ .stream = {0}, .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL };
    }
}

void PlayRandomMusicFromCategory(MusicCategory category, bool loop) {
    if (category >= MUSIC_CATEGORY_COUNT || numMusicTracks[category] == 0) return;
    PlayMusicTrack(category, GetRandomValue(0, numMusicTracks[category] - 1), loop);
}

int Sound_GetMusicIndexByName(MusicCategory category, const char* songName) {
    if (category >= MUSIC_CATEGORY_COUNT || !songName || strlen(songName) == 0) return -1;
    for (int i = 0; i < numMusicTracks[category]; i++) {
        if (strcmp(musicTracks[category][i].fileName, songName) == 0) {
            return i;
        }
    }
    TraceLog(LOG_WARNING, "Sound_GetMusicIndexByName: Musica '%s' nao encontrada na categoria %d.", songName, category);
    return -1;
}

bool PlayMusicFile(const char* filePathInAssetsAudio, bool loop, MusicCategory assumedCategoryForVolume) {
    StopCurrentMusic();
    char fullPath[512]; sprintf(fullPath, "assets/audio/%s", filePathInAssetsAudio);
    if (!FileExists(fullPath)) { TraceLog(LOG_ERROR, "PlayMusicFile: Arquivo nao encontrado: %s", fullPath); return false; }

    onDemandMusicTrack = LoadMusicStream(fullPath);
    if (onDemandMusicTrack.stream.buffer == NULL) { onDemandMusicLoaded = false; return false; }
    onDemandMusicLoaded = true; currentPlayingMusic = onDemandMusicTrack; currentPlayingMusic.looping = loop;
    PlayMusicStream(currentPlayingMusic);
    SetMusicVolume(currentPlayingMusic, GetEffectiveVolumeForCategory(assumedCategoryForVolume));
    currentPlayingCategory = assumedCategoryForVolume; currentPlayingTrackIndex = -2;
    isCurrentMusicPaused = false;
    return true;
}

bool PlaySfxFile(const char* filePathInAssetsAudio) {
    char fullPath[512]; sprintf(fullPath, "assets/audio/%s", filePathInAssetsAudio);
    if (!FileExists(fullPath)) { return false; }
    Sound sfx = LoadSound(fullPath); if (sfx.frameCount == 0) { return false; }
    SetSoundVolume(sfx, sfxVolume * masterVolume); PlaySound(sfx); UnloadSound(sfx);
    return true;
}