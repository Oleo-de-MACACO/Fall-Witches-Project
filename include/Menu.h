#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"
#include <stdbool.h>

extern bool g_request_exit; // Definido em main.c

typedef enum { /* ... (enum MenuButtonAction como antes) ... */
    BUTTON_ACTION_NONE, BUTTON_ACTION_GOTO_PLAYER_MODE_MENU, BUTTON_ACTION_NEW_GAME_SETUP_SLOT,
    BUTTON_ACTION_SETTINGS, BUTTON_ACTION_QUIT_GAME, BUTTON_ACTION_RESUME_GAME,
    BUTTON_ACTION_SAVE_GAME, BUTTON_ACTION_LOAD_GAME, BUTTON_ACTION_PAUSE_SETTINGS,
    BUTTON_ACTION_GOTO_MAIN_MENU,
    BUTTON_ACTION_GOTO_SINGLE_PLAYER_SETUP,
    BUTTON_ACTION_GOTO_TWO_PLAYER_SETUP
} MenuButtonAction;

typedef struct { /* ... (struct MenuButton como antes) ... */
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

void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos);
void DrawMenuScreen(void);
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);
// Corrigido para bool*
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, bool *isPlaying_ptr, Vector2 virtualMousePos);
void DrawPlayerModeMenuScreen(void);

void Menu_RequestSaveLoadScreen(GameState *currentScreen_ptr, bool isSaving, GameState fromScreen);
// Corrigido para bool*
void Menu_RequestMainMenu(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, bool *musicIsCurrentlyPlaying_ptr);

bool Menu_IsInSaveMode(void);
bool Menu_IsNewGameFlow(void);
int Menu_GetSaveLoadSubState(void);
int Menu_GetSelectedSlotForAction(void);
GameState Menu_GetPreviousScreenBeforeSaveLoad(void);
GameModeType Menu_GetSaveLoadContextMode(void);

void Menu_SetSaveLoadSubState(int subState);
void Menu_SetSelectedSlotForAction(int slot);
void Menu_SetIsNewGameFlow(bool isNewGameFlow);

#endif // MENU_H