#include <stddef.h>     
#include "raylib.h"     
#include "../include/Game.h"    
#include "../include/Classes.h" 

// --- Constantes e Variáveis Globais de main.c ---
const int SCREEN_WIDTH = 800;   
const int SCREEN_HEIGHT = 450;  

Player players[MAX_PLAYERS];        // Array para armazenar os dados dos jogadores
// Texture2D player1Texture;        // REMOVIDA - Cada jogador terá sua textura em sua struct
Music gamePlaylist[MAX_SIZE];       

GameState currentScreen;            
int introScreenFramesCounter = 0;   

// Estado da Música
int currentPlaylistIndex = 0;       
int musicIsPlaying = 1;             
float musicVolume = 0.5f;           
float musicPlayTimer = 0.0f;        
float currentTrackDuration = 0.0f;  

// --- Coordenadas e Limites do Mapa ---
int g_currentMapX = 0; 
int g_currentMapY = 0; 

const int WORLD_MAP_MIN_X = -10; 
const int WORLD_MAP_MAX_X = 10; 
const int WORLD_MAP_MIN_Y = -10; 
const int WORLD_MAP_MAX_Y = 10; 


int main(void) { 
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_RESIZABLE); 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fall Witches - Sprites para Ambos"); 
    InitAudioDevice(); 

    // CORRIGIDO: Não passa mais &player1Texture
    InitGameResources(players, gamePlaylist);
    currentScreen = GAMESTATE_INTRO; 

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

    while (!WindowShouldClose()) { 
        switch (currentScreen) {
            case GAMESTATE_MENU:
                UpdateMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying);
                break;
            case GAMESTATE_INTRO:
                UpdateIntroScreen(&currentScreen, &introScreenFramesCounter);
                break;
            case GAMESTATE_PLAYING:
                UpdatePlayingScreen(&currentScreen, players, gamePlaylist, &currentPlaylistIndex, &musicVolume, 
                                    &musicIsPlaying, &musicPlayTimer, &currentTrackDuration, 
                                    &g_currentMapX, &g_currentMapY);
                break;
            case GAMESTATE_PAUSE:
                UpdatePauseScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicIsPlaying);
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
                case GAMESTATE_PLAYING:
                    // CORRIGIDO: Não passa mais player1Texture
                    DrawPlayingScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_PAUSE:
                    // CORRIGIDO: Não passa mais player1Texture
                    DrawPauseScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                default:
                    DrawText("ESTADO DESCONHECIDO!", 20, 20, 20, RED);
                    break;
            }
        EndDrawing(); 
    }

    // --- Fase de Desinicialização ---
    // Descarrega as texturas de CADA jogador
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].txr.id > 0) { // Verifica se a textura do jogador i é válida
            UnloadTexture(players[i].txr);
            TraceLog(LOG_INFO, "Textura do Jogador %d descarregada.", i);
        }
    }
    // if (player1Texture.id > 0) { UnloadTexture(player1Texture); } // Linha antiga removida

    for (int i = 0; i < MAX_SIZE; i++) { 
        if (gamePlaylist[i].stream.buffer != NULL) { UnloadMusicStream(gamePlaylist[i]); } 
    }

    CloseAudioDevice(); 
    CloseWindow();      
    return 0; 
}