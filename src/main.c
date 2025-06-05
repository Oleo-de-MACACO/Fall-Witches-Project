#include <stddef.h>
#include <stdbool.h>
#include "raylib.h"
#include <math.h>

// --- Inclusão dos Headers do Projeto ---
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Menu.h"
#include "../include/Inventory.h"
#include "../include/CharacterCreation.h"
#include "../include/PauseMenu.h"
#include "../include/LoadSaveUI.h"
#include "../include/Settings.h"
#include "../include/WalkCycle.h"
#include "../include/Sound.h"
#include "../include/WorldMap.h"
#include "../include/WorldLoading.h"

// --- Constantes Globais ---
const int virtualScreenWidth = 800;
const int virtualScreenHeight = 450;
const int WORLD_MAP_MIN_X = -10;
const int WORLD_MAP_MAX_X = 10;
const int WORLD_MAP_MIN_Y = -10;
const int WORLD_MAP_MAX_Y = 10;
const int gameSectionWidthMultiplier = 3; // Usado como fallback ou para lógica legada
const int gameSectionHeightMultiplier = 3; // Usado como fallback ou para lógica legada

// --- Variáveis Globais do Jogo ---
Player players[MAX_PLAYERS_SUPPORTED];
Music gamePlaylist[MAX_MUSIC_PLAYLIST_SIZE]; // Legado

GameState currentScreen;
GameModeType currentGameMode = GAME_MODE_UNINITIALIZED;
int currentActivePlayers = MAX_PLAYERS_SUPPORTED;
int introScreenFramesCounter = 0;

// --- Variáveis Globais de Volume (Definidas aqui) ---
float masterVolume = 1.0f;          // *** NOVO: Volume Principal/Mestre ***
float mainMenuMusicVolume = 0.7f;
float gameplayMusicVolume = 0.5f;
float battleMusicVolume = 0.5f;
float cutsceneMusicVolume = 0.6f;
float ambientNatureVolume = 0.4f;
float ambientCityVolume = 0.4f;
float ambientCaveVolume = 0.45f;
float sfxVolume = 0.5f;

bool isMusicPlaying = true; // Flag de conveniência, Sound.c gerencia o estado real
int currentPlaylistIndex = 0;
float musicPlayTimer = 0.0f;
float currentTrackDuration = 0.0f;

int g_currentMapX = 0;
int g_currentMapY = 0;
bool g_request_exit = false;

RenderTexture2D targetRenderTexture;
Camera2D gameCamera = {
    .offset = {0.0f, 0.0f}, .target = {0.0f, 0.0f}, .rotation = 0.0f, .zoom = 1.0f
};
WorldSection* currentActiveWorldSection = NULL;

Vector2 GetVirtualMousePosition(Vector2 actualMousePos) {
    Vector2 virtualMouse = { 0.0f, 0.0f };
    float scale = fminf((float)GetScreenWidth() / (float)virtualScreenWidth, (float)GetScreenHeight() / (float)virtualScreenHeight);
    float scaledRenderWidth = (float)virtualScreenWidth * scale;
    float scaledRenderHeight = (float)virtualScreenHeight * scale;
    float letterboxX = ((float)GetScreenWidth() - scaledRenderWidth) * 0.5f;
    float letterboxY = ((float)GetScreenHeight() - scaledRenderHeight) * 0.5f;
    virtualMouse.x = (actualMousePos.x - letterboxX) / scale;
    virtualMouse.y = (actualMousePos.y - letterboxY) / scale;
    return virtualMouse;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);
    InitWindow(virtualScreenWidth, virtualScreenHeight, "Fall Witches");
    if (!IsWindowReady()) { return 1; }
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    if(!IsAudioDeviceReady()){ TraceLog(LOG_WARNING, "Dispositivo de audio nao pode ser inicializado."); }

    InitGameResources(players, gamePlaylist);
    LoadGameAudio("assets/audio");

    UpdateCurrentlyPlayingMusicVolume(); // Aplica volumes iniciais baseados em master e categorias
    ApplySfxVolume();                   // Aplica volume SFX inicial (baseado em master e sfxVolume)

    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    if (targetRenderTexture.id == 0) {
        TraceLog(LOG_ERROR, "Falha ao carregar render texture.");
        if(IsAudioDeviceReady()) { CloseAudioDevice(); }
        CloseWindow(); return 1;
    }
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    gameCamera.offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
    gameCamera.target = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
    gameCamera.zoom = 1.0f;
    currentScreen = GAMESTATE_INTRO;
    introScreenFramesCounter = 0;
    currentActiveWorldSection = LoadWorldSection(g_currentMapX, g_currentMapY);
    if (currentActiveWorldSection == NULL || !currentActiveWorldSection->isLoaded) {
        TraceLog(LOG_ERROR, "Falha critica ao carregar secao inicial (0,0).");
    }

    if (GetMusicTrackCount(MUSIC_CATEGORY_MAINMENU) > 0) {
        PlayMusicTrack(MUSIC_CATEGORY_MAINMENU, 0, true); // Sound.c aplicará mainMenuMusicVolume * masterVolume
        isMusicPlaying = true;
    } else { isMusicPlaying = false; }
    SetTargetFPS(60);

    while (!WindowShouldClose() && !g_request_exit) {
        if(IsAudioDeviceReady()) UpdateAudioStreams();
        Vector2 mousePosition = GetMousePosition();
        Vector2 virtualMousePosition = GetVirtualMousePosition(mousePosition);

        // Exemplo de input para mudar volumes (idealmente em Settings)
        // Ajusta masterVolume como exemplo, os outros seguem o mesmo padrão em Settings.c
        if (IsKeyPressed(KEY_PAGE_DOWN)) {
            masterVolume -= 0.1f; if (masterVolume < 0.0f) masterVolume = 0.0f;
            UpdateCurrentlyPlayingMusicVolume(); ApplySfxVolume();
        }
        if (IsKeyPressed(KEY_PAGE_UP)) {
            masterVolume += 0.1f; if (masterVolume > 1.0f) masterVolume = 1.0f;
            UpdateCurrentlyPlayingMusicVolume(); ApplySfxVolume();
        }
        // Lógica de atualização de gameplayMusicVolume e sfxVolume com +/- removida daqui,
        // pois Settings.c será o local principal para essas mudanças.

        if (currentGameMode == GAME_MODE_SINGLE_PLAYER) { currentActivePlayers = 1; }
        else { currentActivePlayers = MAX_PLAYERS_SUPPORTED; }

        int previousMapX = g_currentMapX; int previousMapY = g_currentMapY;

        switch (currentScreen) {
            case GAMESTATE_MENU: UpdateMenuScreen(&currentScreen, virtualMousePosition); break;
            case GAMESTATE_INTRO: UpdateIntroScreen(&currentScreen, &introScreenFramesCounter); break;
            case GAMESTATE_PLAYER_MODE_MENU:
                UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying, virtualMousePosition);
                break;
            case GAMESTATE_CHARACTER_CREATION:
                UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying);
                break;
            case GAMESTATE_PLAYING:
                // Passa o ponteiro para gameplayMusicVolume (que será multiplicado por masterVolume em Sound.c)
                UpdatePlayingScreen(&currentScreen, players, currentActivePlayers,
                                     gamePlaylist, &currentPlaylistIndex,
                                     &gameplayMusicVolume, 
                                     &isMusicPlaying,
                                     &musicPlayTimer, &currentTrackDuration,
                                     &g_currentMapX, &g_currentMapY, &gameCamera, currentActiveWorldSection);
                break;
            case GAMESTATE_PAUSE:
                UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, isMusicPlaying, &isMusicPlaying, virtualMousePosition);
                break;
            case GAMESTATE_INVENTORY:
                UpdateInventoryScreen(&currentScreen, players, &isMusicPlaying, gamePlaylist, &currentPlaylistIndex);
                break;
            case GAMESTATE_SAVE_LOAD_MENU:
                UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume,
                                         &isMusicPlaying, &g_currentMapX, &g_currentMapY, virtualMousePosition,
                                         &currentActiveWorldSection);
                break;
            case GAMESTATE_SETTINGS: UpdateSettingsScreen(&currentScreen); break; // Settings irá ler e modificar os volumes globais
            default: break;
        }
        if (currentScreen == GAMESTATE_PLAYING && (g_currentMapX != previousMapX || g_currentMapY != previousMapY)) {
            if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
            currentActiveWorldSection = LoadWorldSection(g_currentMapX, g_currentMapY);
            if (!currentActiveWorldSection || !currentActiveWorldSection->isLoaded) { /* Tratar erro */ }
        }
        BeginTextureMode(targetRenderTexture);
            ClearBackground(RAYWHITE);
            if (currentScreen == GAMESTATE_PLAYING) {
                BeginMode2D(gameCamera);
                    if (currentActiveWorldSection) { DrawWorldSectionBackground(currentActiveWorldSection); }
                    else { ClearBackground(DARKGRAY); DrawText("ERRO: SECAO NAO CARREGADA",10,10,20,RED); }
                    DrawPlayingScreen(players, currentActivePlayers, gameplayMusicVolume * masterVolume, currentPlaylistIndex, isMusicPlaying, g_currentMapX, g_currentMapY);
                EndMode2D();
                DrawFPS(10, virtualScreenHeight - 20); 
            } else if (currentScreen == GAMESTATE_PAUSE) {
                BeginMode2D(gameCamera);
                    if (currentActiveWorldSection) DrawWorldSectionBackground(currentActiveWorldSection); else ClearBackground(DARKGRAY);
                    DrawPlayingScreen(players, currentActivePlayers, gameplayMusicVolume * masterVolume, currentPlaylistIndex, isMusicPlaying, g_currentMapX, g_currentMapY);
                EndMode2D();
                DrawPauseMenuElements();
            } else if (currentScreen == GAMESTATE_INVENTORY) {
                BeginMode2D(gameCamera);
                     if (currentActiveWorldSection) DrawWorldSectionBackground(currentActiveWorldSection); else ClearBackground(DARKGRAY);
                    DrawInventoryScreen(players, players, gameplayMusicVolume * masterVolume, currentPlaylistIndex, isMusicPlaying, g_currentMapX, g_currentMapY);
                EndMode2D();
                DrawInventoryUIElements(players);
            } else if (currentScreen == GAMESTATE_SETTINGS) {
                DrawSettingsScreen();
            } else { 
                switch (currentScreen) {
                    case GAMESTATE_MENU: DrawMenuScreen(); break;
                    case GAMESTATE_INTRO: DrawIntroScreen(); break;
                    case GAMESTATE_PLAYER_MODE_MENU: DrawPlayerModeMenuScreen(); break;
                    case GAMESTATE_CHARACTER_CREATION: DrawCharacterCreationScreen(players); break;
                    case GAMESTATE_SAVE_LOAD_MENU: DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, isMusicPlaying, gameplayMusicVolume * masterVolume, g_currentMapX, g_currentMapY); break;
                    default: DrawText("ESTADO DESCONHECIDO!", (int)(((float)virtualScreenWidth - (float)MeasureText("ESTADO DESCONHECIDO!", 20)) / 2.0f), (int)(((float)virtualScreenHeight - 20.0f) / 2.0f - 10.0f), 20, RED); break;
                }
            }
        EndTextureMode();
        BeginDrawing();
            ClearBackground(BLACK);
            float scale = fminf((float)GetScreenWidth()/(float)virtualScreenWidth, (float)GetScreenHeight()/(float)virtualScreenHeight);
            float dW = (float)virtualScreenWidth * scale; float dH = (float)virtualScreenHeight * scale;
            float dX = ((float)GetScreenWidth() - dW) * 0.5f; float dY = ((float)GetScreenHeight() - dH) * 0.5f;
            DrawTexturePro(targetRenderTexture.texture,
                           (Rectangle){0.0f,0.0f,(float)targetRenderTexture.texture.width, (float)-targetRenderTexture.texture.height},
                           (Rectangle){dX,dY,dW,dH}, (Vector2){0.0f,0.0f}, 0.0f, WHITE);
        EndDrawing();
    } // Fim do loop principal

    if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); currentActiveWorldSection = NULL; }
    UnloadGameAudio();
    UnloadRenderTexture(targetRenderTexture);
    for (int i=0; i<MAX_PLAYERS_SUPPORTED; i++) { UnloadCharacterAnimations(&players[i]); }
    for (int i=0; i<MAX_MUSIC_PLAYLIST_SIZE; i++) { if(gamePlaylist[i].stream.buffer != NULL) UnloadMusicStream(gamePlaylist[i]); }
    if(IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
    return 0;
}