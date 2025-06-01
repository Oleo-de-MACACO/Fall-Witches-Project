#include <stddef.h>
#include <stdbool.h>     
#include "raylib.h"
#include "../include/Game.h"    
#include "../include/Classes.h" 
#include "../include/Menu.h"    
#include "../include/Inventory.h" 
#include "../include/SaveLoad.h"  

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

Player players[MAX_PLAYERS];    
Music gamePlaylist[MAX_SIZE]; 

GameState currentScreen;        
int introScreenFramesCounter = 0; 

int currentPlaylistIndex = 0; 
int musicIsPlaying = 1; 
float musicVolume = 0.5f; 
float musicPlayTimer = 0.0f; 
float currentTrackDuration = 0.0f; 

int g_currentMapX = 0; 
int g_currentMapY = 0; 
const int WORLD_MAP_MIN_X = -10; 
const int WORLD_MAP_MAX_X = 10;  
const int WORLD_MAP_MIN_Y = -10; 
const int WORLD_MAP_MAX_Y = 10;  

bool g_request_exit = false;

int main(void) {
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fall Witches - 0.0.6"); 
    InitAudioDevice(); 

    InitGameResources(players, gamePlaylist); 

    currentScreen = GAMESTATE_INTRO; 
    introScreenFramesCounter = 0;

    if (gamePlaylist[currentPlaylistIndex].frameCount > 0 && gamePlaylist[currentPlaylistIndex].stream.buffer != NULL) {
        PlayMusicStream(gamePlaylist[currentPlaylistIndex]);
        SetMusicVolume(gamePlaylist[currentPlaylistIndex], musicVolume);
        musicIsPlaying = 1;
        currentTrackDuration = GetMusicTimeLength(gamePlaylist[currentPlaylistIndex]);
        musicPlayTimer = 0.0f;
    } else {
        musicIsPlaying = 0;
    }

    SetTargetFPS(60); 

    while (!WindowShouldClose() && !g_request_exit) {
        switch (currentScreen) {
            case GAMESTATE_MENU:
                UpdateMenuScreen(&currentScreen); 
                break;
            case GAMESTATE_INTRO:
                UpdateIntroScreen(&currentScreen, &introScreenFramesCounter);
                break;
            case GAMESTATE_PLAYER_MODE_MENU:
                UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying);
                break;
            case GAMESTATE_CHARACTER_CREATION: 
                UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying);
                break;
            case GAMESTATE_PLAYING:
                UpdatePlayingScreen(&currentScreen, players, gamePlaylist, &currentPlaylistIndex, &musicVolume,
                                    &musicIsPlaying, &musicPlayTimer, &currentTrackDuration,
                                    &g_currentMapX, &g_currentMapY);
                break;
            case GAMESTATE_PAUSE:
                UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, &musicIsPlaying);
                break;
            case GAMESTATE_INVENTORY:
                UpdateInventoryScreen(&currentScreen, players, &musicIsPlaying, gamePlaylist, &currentPlaylistIndex);
                break;
            case GAMESTATE_SAVE_LOAD_MENU: 
                UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying, &g_currentMapX, &g_currentMapY); 
                break;
            default: 
                break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE); 
            switch (currentScreen) {
                case GAMESTATE_MENU:
                    DrawMenuScreen();
                    break;
                case GAMESTATE_INTRO:
                    DrawIntroScreen();
                    break;
                case GAMESTATE_PLAYER_MODE_MENU:
                    DrawPlayerModeMenuScreen();
                    break;
                case GAMESTATE_CHARACTER_CREATION: 
                    DrawCharacterCreationScreen(players, 0); 
                    break;
                case GAMESTATE_PLAYING:
                    DrawPlayingScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_PAUSE:
                    DrawPauseScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_INVENTORY:
                    DrawInventoryScreen(players, players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_SAVE_LOAD_MENU: 
                    DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, musicVolume, g_currentMapX, g_currentMapY);
                    break;
                default:
                    DrawText("ESTADO DESCONHECIDO!", GetScreenWidth()/2 - MeasureText("ESTADO DESCONHECIDO!", 20)/2, GetScreenHeight()/2 - 10, 20, RED);
                    break;
            }
        EndDrawing();
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].txr.id > 0) { 
            UnloadTexture(players[i].txr);
        }
    }
    for (int i = 0; i < MAX_SIZE; i++) {
        if (gamePlaylist[i].stream.buffer != NULL) { UnloadMusicStream(gamePlaylist[i]); }
    }

    CloseAudioDevice(); 
    CloseWindow();      
    return 0;
}