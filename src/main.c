#include <stddef.h>
#include <stdbool.h>     // Para o tipo bool (usado por g_request_exit)
#include "raylib.h"
#include "../include/Game.h"    // Para GameState, Player, Music, InitGameResources, etc.
#include "../include/Classes.h" // Para a struct Player
#include "../include/Menu.h"    // Para todas as funções de tela do Menu (Update/Draw Menu, Intro, Pause, SaveLoad)
#include "../include/Inventory.h" // Para UpdateInventoryScreen, DrawInventoryScreen, AddItemToInventory
#include "../include/SaveLoad.h"  // Para AddItemToInventory (se fosse usado aqui, mas é usado em Menu.c) - na verdade, SaveLoad.h é mais para Menu.c

// --- Constantes e Variáveis Globais de main.c ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

Player players[MAX_PLAYERS];
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

// Definição da flag global de saída
bool g_request_exit = false;


int main(void) {
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fall Witches - Save/Load Slots"); // Título atualizado
    InitAudioDevice();

    InitGameResources(players, gamePlaylist);

    // Exemplo: Adicionar alguns itens aos inventários dos jogadores para teste
    if (MAX_PLAYERS > 0) {
        AddItemToInventory(&players[0], "Poção de Vida", 5);
        AddItemToInventory(&players[0], "Espada Curta", 1);
    }
    if (MAX_PLAYERS > 1) {
        AddItemToInventory(&players[1], "Varinha Mágica", 1);
        AddItemToInventory(&players[1], "Poção de Mana", 3);
    }

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

    while (!WindowShouldClose() && !g_request_exit) {
        // --- Lógica de Atualização ---
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
                UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, &musicIsPlaying);
                break;
            case GAMESTATE_INVENTORY:
                UpdateInventoryScreen(&currentScreen, players, &musicIsPlaying, gamePlaylist, &currentPlaylistIndex);
                break;
            case GAMESTATE_SAVE_LOAD_MENU: // <-- NOVO CASE ADICIONADO
                // As funções UpdateSaveLoadMenuScreen e DrawSaveLoadMenuScreen estão em Menu.c
                // Elas precisam de acesso aos players e ao estado da música se LoadGame levar de volta ao jogo.
                UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, &musicIsPlaying);
                break;
            default:
                // Nenhum estado conhecido para atualizar
                break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            // --- Lógica de Desenho ---
            switch (currentScreen) {
                case GAMESTATE_MENU:
                    DrawMenuScreen();
                    break;
                case GAMESTATE_INTRO:
                    DrawIntroScreen();
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
                case GAMESTATE_SAVE_LOAD_MENU: // <-- NOVO CASE ADICIONADO
                    // Passa os parâmetros necessários para desenhar o fundo (que é a tela de pausa)
                    // e para a própria tela de Save/Load interagir com os dados do jogo.
                    DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, musicVolume, g_currentMapX, g_currentMapY);
                    break;
                default:
                    DrawText("ESTADO DESCONHECIDO!", GetScreenWidth()/2 - MeasureText("ESTADO DESCONHECIDO!", 20)/2, GetScreenHeight()/2 - 10, 20, RED);
                    break;
            }
        EndDrawing();
    }

    // --- Fase de Desinicialização ---
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].txr.id > 0) {
            UnloadTexture(players[i].txr);
            TraceLog(LOG_INFO, "Textura do Jogador %d descarregada.", i);
        }
    }
    for (int i = 0; i < MAX_SIZE; i++) {
        if (gamePlaylist[i].stream.buffer != NULL) { UnloadMusicStream(gamePlaylist[i]); }
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}