#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"       // Para o enum GameState e MAX_PLAYERS (se aplicável)
#include "Classes.h"    // Para a struct Player
#include <stdbool.h>    // Para bool

// Declaração externa da flag global de saída. Definida em main.c
extern bool g_request_exit;

// Ações que os botões do menu podem executar
typedef enum {
    BUTTON_ACTION_NONE,
    BUTTON_ACTION_START_2PLAYERS,
    BUTTON_ACTION_SETTINGS,
    BUTTON_ACTION_QUIT_GAME,
    BUTTON_ACTION_RESUME_GAME,
    BUTTON_ACTION_SAVE_GAME,
    BUTTON_ACTION_LOAD_GAME,
    BUTTON_ACTION_PAUSE_SETTINGS,
    BUTTON_ACTION_GOTO_MAIN_MENU
} MenuButtonAction;

// Estrutura para um botão
typedef struct {
    Rectangle rect;
    const char *text;
    Color base_color;
    Color hover_color;
    Color disabled_color;
    Color text_color;
    bool is_active;
    bool is_hovered;
    MenuButtonAction action;
} MenuButton;

// --- Protótipos para as Funções da Tela de Menu Principal ---
void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr);
void DrawMenuScreen(void);

// --- Protótipos para as Funções da Tela de Introdução ---
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);

// --- Protótipos para as Funções da Tela de Pausa ---
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr);
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY);

// --- Protótipos para as Funções da Tela de Save/Load Slots --- ADDED THESE
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int *musicIsCurrentlyPlaying_ptr);
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY);


#endif // MENU_H