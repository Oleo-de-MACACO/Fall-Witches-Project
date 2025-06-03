#include <stddef.h>
#include <stdbool.h>
#include "raylib.h"
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Menu.h"
#include "../include/Inventory.h"
// #include "../include/SaveLoad.h" // Not directly used by main.c if LoadSaveUI.c handles it
#include "../include/CharacterCreation.h" // Included because main.c calls its Update/Draw
#include "../include/PauseMenu.h"       // Included because main.c calls its Update/Draw
#include "../include/LoadSaveUI.h"      // Included because main.c calls its Update/Draw
#include "../include/Settings.h"       // Included because main.c calls its Update/Draw
#include "../include/WalkCycle.h"      // For UnloadCharacterAnimations
#include "../include/Sound.h"
#include <math.h> // Para fminf

// --- Constantes Globais para Resolução Virtual ---
const int virtualScreenWidth = 800;
const int virtualScreenHeight = 450;

// --- Variáveis Globais do Jogo ---
Player players[MAX_PLAYERS_SUPPORTED]; // Usa MAX_PLAYERS_SUPPORTED de Game.h
Music gamePlaylist[MAX_MUSIC_PLAYLIST_SIZE]; // Usa MAX_MUSIC_PLAYLIST_SIZE de Game.h
GameState currentScreen;
GameModeType currentGameMode = GAME_MODE_UNINITIALIZED; // De Game.h
int currentActivePlayers = MAX_PLAYERS_SUPPORTED;   // Default, ajustado dinamicamente

int introScreenFramesCounter = 0;
int currentPlaylistIndex = 0; // Definido globalmente
int musicIsPlaying = 1;
float musicVolume = 0.5f;
float sfxVolume = 0.5f; // Adicionado para configurações de som
float musicPlayTimer = 0.0f;
float currentTrackDuration = 0.0f;
int g_currentMapX = 0;
int g_currentMapY = 0;
const int WORLD_MAP_MIN_X = -10;
const int WORLD_MAP_MAX_X = 10;
const int WORLD_MAP_MIN_Y = -10;
const int WORLD_MAP_MAX_Y = 10;
bool g_request_exit = false;

RenderTexture2D targetRenderTexture; 
Camera2D gameCamera = {
    .offset = {0.0f, 0.0f},
    .target = {0.0f, 0.0f},
    .rotation = 0.0f,
    .zoom = 1.0f
};

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
    InitWindow(virtualScreenWidth, virtualScreenHeight, "Fall Witches - 0.0.11");
    if (!IsWindowReady()) {
        // Idealmente, logar ou tratar erro se a janela não puder ser criada
        return 1;
    }
    SetExitKey(KEY_NULL); 
    InitAudioDevice();
    if(!IsAudioDeviceReady()){
        TraceLog(LOG_WARNING, "Dispositivo de audio nao pode ser inicializado.");
    }

    InitGameResources(players, gamePlaylist); // Presume que players e gamePlaylist são inicializados aqui
    LoadGameAudio("assets/audio"); // Carrega todos os sons e músicas
    ApplyMusicVolume(); // Aplica volume inicial às músicas (se Sound.c o fizer)
    ApplySfxVolume();   // Aplica volume inicial aos SFX (se Sound.c o fizer)


    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    if (targetRenderTexture.id == 0) { // Checa se a render texture foi carregada
        TraceLog(LOG_ERROR, "Falha ao carregar render texture.");
        if(IsAudioDeviceReady()) CloseAudioDevice();
        CloseWindow();
        return 1;
    }
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    gameCamera.offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
    gameCamera.target = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
    gameCamera.zoom = 1.0f; // Zoom inicial resetado
    
    currentScreen = GAMESTATE_INTRO;
    introScreenFramesCounter = 0;

    // Toca música inicial do menu, se houver
    if (GetMusicTrackCount(MUSIC_CATEGORY_MAINMENU) > 0) { // Usa função de Sound.c
        PlayMusicTrack(MUSIC_CATEGORY_MAINMENU, 0, true);
        musicIsPlaying = 1; // Assume que PlayMusicTrack define isso ou precisamos de um getter
    } else {
        musicIsPlaying = 0;
    }
    
    SetTargetFPS(60);

    while (!WindowShouldClose() && !g_request_exit) {
        if(IsAudioDeviceReady()) UpdateAudioStreams(); // Atualiza streams de música do Sound.c

        Vector2 mousePosition = GetMousePosition();
        Vector2 virtualMousePosition = GetVirtualMousePosition(mousePosition);

        if (currentGameMode == GAME_MODE_SINGLE_PLAYER) {
            currentActivePlayers = 1;
        } else {
            currentActivePlayers = MAX_PLAYERS_SUPPORTED;
        }

        switch (currentScreen) {
            case GAMESTATE_MENU: UpdateMenuScreen(&currentScreen, virtualMousePosition); break;
            case GAMESTATE_INTRO: UpdateIntroScreen(&currentScreen, &introScreenFramesCounter); break;
            case GAMESTATE_PLAYER_MODE_MENU: UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying, virtualMousePosition); break;
            case GAMESTATE_CHARACTER_CREATION: UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying); break;
            case GAMESTATE_PLAYING: UpdatePlayingScreen(&currentScreen, players, currentActivePlayers, gamePlaylist, &currentPlaylistIndex, &musicVolume, &musicIsPlaying, &musicPlayTimer, &currentTrackDuration, &g_currentMapX, &g_currentMapY, &gameCamera); break;
            case GAMESTATE_PAUSE: UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, &musicIsPlaying, virtualMousePosition); break;
            case GAMESTATE_INVENTORY: UpdateInventoryScreen(&currentScreen, players, &musicIsPlaying, gamePlaylist, &currentPlaylistIndex); break;
            case GAMESTATE_SAVE_LOAD_MENU: UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying, &g_currentMapX, &g_currentMapY, virtualMousePosition); break;
            case GAMESTATE_SETTINGS: UpdateSettingsScreen(&currentScreen); break; 
            default: break;
        }
        
        // Aplica o volume da música globalmente (Sound.c pode ter um ApplyMusicVolume mais específico)
        // Esta chamada global pode ser redundante se PlayMusicTrack já define o volume.
        // if (musicIsPlaying && currentPlaylistIndex >= 0 && currentPlaylistIndex < MAX_MUSIC_PLAYLIST_SIZE && 
        //     gamePlaylist[currentPlaylistIndex].stream.buffer != NULL) { // A playlist global pode não ser mais o foco
        //      SetMusicVolume(gamePlaylist[currentPlaylistIndex], musicVolume);
        // }


        BeginTextureMode(targetRenderTexture);
            ClearBackground(RAYWHITE); // Limpa a textura de renderização para RAYWHITE
            
            if (currentScreen == GAMESTATE_PLAYING) {
                BeginMode2D(gameCamera); 
                DrawPlayingScreen(players, currentActivePlayers, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY); 
                EndMode2D();
            } else if (currentScreen == GAMESTATE_PAUSE) {
                BeginMode2D(gameCamera); 
                DrawPlayingScreen(players, currentActivePlayers, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY); 
                EndMode2D();
                DrawPauseMenuElements(); 
            } else if (currentScreen == GAMESTATE_INVENTORY) {
                BeginMode2D(gameCamera); 
                DrawInventoryScreen(players, players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY); 
                EndMode2D(); 
                DrawInventoryUIElements(players); 
            } else if (currentScreen == GAMESTATE_SETTINGS) {
                DrawSettingsScreen();
            } else { 
                switch (currentScreen) {
                    case GAMESTATE_MENU: DrawMenuScreen(); break;
                    case GAMESTATE_INTRO: DrawIntroScreen(); break; // DrawIntroScreen limpa seu próprio fundo para PRETO
                    case GAMESTATE_PLAYER_MODE_MENU: DrawPlayerModeMenuScreen(); break;
                    case GAMESTATE_CHARACTER_CREATION: DrawCharacterCreationScreen(players); break; 
                    case GAMESTATE_SAVE_LOAD_MENU: DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, musicVolume, g_currentMapX, g_currentMapY); break;
                    default: 
                        DrawText("ESTADO DESCONHECIDO!", (int)(((float)virtualScreenWidth - (float)MeasureText("ESTADO DESCONHECIDO!", 20)) / 2.0f), (int)(((float)virtualScreenHeight - 20.0f) / 2.0f - 10.0f), 20, RED); 
                        break; 
                }
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK); // Limpa a janela principal para PRETO
            float scale = fminf((float)GetScreenWidth()/(float)virtualScreenWidth, (float)GetScreenHeight()/(float)virtualScreenHeight);
            float dW = (float)virtualScreenWidth * scale; 
            float dH = (float)virtualScreenHeight * scale;
            float dX = ((float)GetScreenWidth() - dW) * 0.5f; 
            float dY = ((float)GetScreenHeight() - dH) * 0.5f;
            DrawTexturePro(targetRenderTexture.texture, 
                           (Rectangle){0.0f,0.0f,(float)targetRenderTexture.texture.width, (float)-targetRenderTexture.texture.height}, 
                           (Rectangle){dX,dY,dW,dH}, 
                           (Vector2){0.0f,0.0f}, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadGameAudio();    
    UnloadRenderTexture(targetRenderTexture);
    for (int i=0; i<MAX_PLAYERS_SUPPORTED; i++) { UnloadCharacterAnimations(&players[i]); }
    // A música é descarregada por UnloadGameAudio agora.
    // for (int i=0; i<MAX_MUSIC_PLAYLIST_SIZE; i++) { if(gamePlaylist[i].stream.buffer != NULL) UnloadMusicStream(gamePlaylist[i]); }
    if(IsAudioDeviceReady()) CloseAudioDevice(); 
    CloseWindow(); 
    return 0;
}