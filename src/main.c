#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

// --- Inclusão de Todos os Headers de Módulos do Jogo ---
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
#include "../include/WorldLoading.h"
#include "../include/Event.h"
#include "../include/Dialogue.h"
#include "../include/GameProgress.h"
#include "../include/CharacterManager.h"
#include "../include/BattleSystem.h"
#include "../include/ClassSettings.h"
#include "../include/BattleUI.h"
#include "../include/EnemyLoader.h" // *** ADICIONADO ***

// ... (todas as suas variáveis globais e constantes permanecem as mesmas) ...
Player players[MAX_PLAYERS_SUPPORTED];
GameState currentScreen;
GameModeType currentGameMode = GAME_MODE_UNINITIALIZED;
int currentActivePlayers = 1;
int g_currentMapX = 0;
int g_currentMapY = 0;
bool g_request_exit = false;
static int introScreenFramesCounter = 0;
float masterVolume = 1.0f;
float mainMenuMusicVolume = 0.7f;
float gameplayMusicVolume = 0.5f;
float battleMusicVolume = 0.5f;
float cutsceneMusicVolume = 0.6f;
float ambientNatureVolume = 0.4f;
float ambientCityVolume = 0.4f;
float ambientCaveVolume = 0.45f;
float sfxVolume = 0.5f;
Music gamePlaylist[MAX_MUSIC_PLAYLIST_SIZE];
bool isMusicPlaying = true;
int currentMusicIndex = 0;
float musicPlayTimer = 0.0f;
float currentTrackDuration = 0.0f;
RenderTexture2D targetRenderTexture;
Camera2D gameCamera = { .offset = {0.0f, 0.0f}, .target = {0.0f, 0.0f}, .rotation = 0.0f, .zoom = 1.0f };
WorldSection* currentActiveWorldSection = NULL;
GameState* g_currentScreen_ptr = &currentScreen;
Player* g_players_ptr = players;
int* g_currentActivePlayers_ptr = &currentActivePlayers;
const int virtualScreenWidth = 800;
const int virtualScreenHeight = 450;
const int WORLD_MAP_MIN_X = -10;
const int WORLD_MAP_MAX_X = 10;
const int WORLD_MAP_MIN_Y = -10;
const int WORLD_MAP_MAX_Y = 10;
const int gameSectionWidthMultiplier = 3;
const int gameSectionHeightMultiplier = 3;


Vector2 GetVirtualMousePosition(Vector2 actualMousePos) {
    Vector2 virtualMouse = { 0.0f, 0.0f };
    float scale = fminf((float)GetScreenWidth() / (float)virtualScreenWidth, (float)GetScreenHeight() / (float)virtualScreenHeight);
    float letterboxX = ((float)GetScreenWidth() - ((float)virtualScreenWidth * scale)) * 0.5f;
    float letterboxY = ((float)GetScreenHeight() - ((float)virtualScreenHeight * scale)) * 0.5f;
    virtualMouse.x = (actualMousePos.x - letterboxX) / scale;
    virtualMouse.y = (actualMousePos.y - letterboxY) / scale;
    return virtualMouse;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);
    InitWindow(virtualScreenWidth, virtualScreenHeight, "Fall Witches");
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    srand(time(NULL));

    TraceLog(LOG_INFO, "Carregando todos os recursos do jogo...");
    InitGameResources(players, gamePlaylist);
    LoadGameAudio("assets/audio");
    Event_LoadAll("assets/Events/events.txt");
    Dialogue_LoadAll("assets/Dialogues/dialogues.txt");
    ClassSettings_LoadAll("assets/ClassSettings/Settings.txt");
    EnemyLoader_LoadAll("assets/Enemies.txt"); // *** ADICIONADO ***
    CharManager_Init();
    
    UpdateCurrentlyPlayingMusicVolume();
    ApplySfxVolume();

    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    gameCamera = (Camera2D){
        .offset = { (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f },
        .target = { (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f },
        .rotation = 0.0f, .zoom = 1.0f
    };
    currentScreen = GAMESTATE_INTRO;
    Progress_Reset();

    if (GetMusicTrackCount(MUSIC_CATEGORY_MAINMENU) > 0) {
        PlayMusicTrack(MUSIC_CATEGORY_MAINMENU, 0, true);
        isMusicPlaying = true;
    }
    SetTargetFPS(60);

    while (!WindowShouldClose() && !g_request_exit) {
        if(IsAudioDeviceReady()) UpdateAudioStreams();
        
        *g_currentActivePlayers_ptr = currentActivePlayers;

        Vector2 virtualMousePosition = GetVirtualMousePosition(GetMousePosition());
        int previousMapX = g_currentMapX;
        int previousMapY = g_currentMapY;

        if (currentScreen == GAMESTATE_PLAYING && Menu_IsNewGameFlow()) {
            Progress_Reset();
            g_currentMapX = 0; g_currentMapY = 0;
            if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
            currentActiveWorldSection = LoadWorldSection(g_currentMapX, g_currentMapY);
            if (!currentActiveWorldSection) {
                TraceLog(LOG_FATAL, "NÃO FOI POSSÍVEL CARREGAR O MAPA INICIAL (0,0)!");
                g_request_exit = true; 
            } else {
                PrepareNewGameSession(players, &g_currentMapX, &g_currentMapY, currentActivePlayers, currentActiveWorldSection);
                CharManager_LoadNpcsForMap();
                CharManager_CacheSpritesForMap();
            }
            Menu_SetIsNewGameFlow(false);
            previousMapX = g_currentMapX; previousMapY = g_currentMapY;
        }

        if (currentScreen == GAMESTATE_BATTLE) {
            BattleSystem_Update();
            BattleUI_Update();
        } else if (Dialogue_IsActive()) {
            Dialogue_Update();
        } else {
            switch (currentScreen) {
                case GAMESTATE_INTRO: UpdateIntroScreen(&currentScreen, &introScreenFramesCounter); break;
                case GAMESTATE_MENU: UpdateMenuScreen(&currentScreen, virtualMousePosition); break;
                case GAMESTATE_PLAYER_MODE_MENU: UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentMusicIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying, virtualMousePosition); break;
                case GAMESTATE_CHARACTER_CREATION: UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentMusicIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying); break;
                case GAMESTATE_SAVE_LOAD_MENU: UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentMusicIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying, &g_currentMapX, &g_currentMapY, virtualMousePosition, &currentActiveWorldSection); break;
                case GAMESTATE_SETTINGS: UpdateSettingsScreen(&currentScreen); break;
                case GAMESTATE_PAUSE: UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentMusicIndex, isMusicPlaying, &isMusicPlaying, virtualMousePosition); break;
                case GAMESTATE_INVENTORY: UpdateInventoryScreen(&currentScreen, players, &isMusicPlaying, gamePlaylist, &currentMusicIndex); break;
                case GAMESTATE_PLAYING:
                    UpdatePlayingScreen(&currentScreen, players, currentActivePlayers, gamePlaylist, &currentMusicIndex, &gameplayMusicVolume, &isMusicPlaying, &musicPlayTimer, &currentTrackDuration, &g_currentMapX, &g_currentMapY, &gameCamera, currentActiveWorldSection);
                    CharManager_Update(&players[0], currentActiveWorldSection);
                    CharManager_CheckInteraction(&players[0]);
                    Event_CheckAndRun(g_currentMapX, g_currentMapY);
                    break;
                default: break;
            }
        }
        
        if (g_currentMapX != previousMapX || g_currentMapY != previousMapY) {
            if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
            CharManager_UnloadAll();
            currentActiveWorldSection = LoadWorldSection(g_currentMapX, g_currentMapY);
            if(currentActiveWorldSection) {
                CharManager_LoadNpcsForMap();
                CharManager_CacheSpritesForMap();
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);
            BeginTextureMode(targetRenderTexture);
                if (currentScreen == GAMESTATE_BATTLE) {
                    BattleUI_Draw();
                } else if (currentScreen == GAMESTATE_PLAYING || currentScreen == GAMESTATE_PAUSE || currentScreen == GAMESTATE_INVENTORY) {
                    BeginMode2D(gameCamera);
                        if (currentActiveWorldSection) DrawWorldSectionBackground(currentActiveWorldSection);
                        else { ClearBackground(PURPLE); DrawText("ERRO: SECAO NAO CARREGADA", 10,10,20,WHITE); }
                        CharManager_Draw();
                        DrawPlayingScreen(players, currentActivePlayers, gameplayMusicVolume * masterVolume, currentMusicIndex, isMusicPlaying, g_currentMapX, g_currentMapY);
                        DrawWorldSectionDebug(currentActiveWorldSection);
                    EndMode2D();
                    if(currentScreen == GAMESTATE_PAUSE) DrawPauseMenuElements();
                    if(currentScreen == GAMESTATE_INVENTORY) DrawInventoryUIElements(players);
                } else {
                    switch (currentScreen) {
                        case GAMESTATE_MENU: DrawMenuScreen(); break;
                        case GAMESTATE_INTRO: DrawIntroScreen(); break;
                        case GAMESTATE_PLAYER_MODE_MENU: DrawPlayerModeMenuScreen(); break;
                        case GAMESTATE_CHARACTER_CREATION: DrawCharacterCreationScreen(players); break;
                        case GAMESTATE_SAVE_LOAD_MENU: DrawSaveLoadMenuScreen(players, gamePlaylist, currentMusicIndex, isMusicPlaying, gameplayMusicVolume * masterVolume, g_currentMapX, g_currentMapY); break;
                        case GAMESTATE_SETTINGS: DrawSettingsScreen(); break;
                        default: DrawText("ESTADO DESCONHECIDO", 190, 200, 20, LIGHTGRAY); break;
                    }
                }
            EndTextureMode();
            
            float scale = fminf((float)GetScreenWidth()/(float)virtualScreenWidth, (float)GetScreenHeight()/(float)virtualScreenHeight);
            Rectangle sourceRect = { 0.0f, 0.0f, (float)targetRenderTexture.texture.width, -(float)targetRenderTexture.texture.height };
            Rectangle destRect = { ((float)GetScreenWidth() - ((float)virtualScreenWidth * scale)) * 0.5f, ((float)GetScreenHeight() - ((float)virtualScreenHeight * scale)) * 0.5f, (float)virtualScreenWidth * scale, (float)virtualScreenHeight * scale };
            DrawTexturePro(targetRenderTexture.texture, sourceRect, destRect, (Vector2){0,0}, 0.0f, WHITE);

            Dialogue_Draw();
            DrawFPS(10, 10);
        EndDrawing();
    }

    if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
    CharManager_UnloadAll();
    Dialogue_UnloadAll();
    Event_UnloadAll();
    // ClassSettings_UnloadAll(); // Descomentar se alocar memória
    EnemyLoader_UnloadAll(); // *** ADICIONADO ***
    UnloadGameAudio();
    UnloadRenderTexture(targetRenderTexture);
    for (int i=0; i<MAX_PLAYERS_SUPPORTED; i++) { UnloadCharacterAnimations(&players[i]); }
    if(IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
    return 0;
}
