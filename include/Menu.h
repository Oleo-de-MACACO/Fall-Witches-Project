#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"       // Para GameState, Player, Music, GameModeType
#include <stdbool.h>

extern bool g_request_exit;

typedef enum {
    BUTTON_ACTION_NONE, BUTTON_ACTION_GOTO_PLAYER_MODE_MENU, BUTTON_ACTION_NEW_GAME_SETUP_SLOT,
    BUTTON_ACTION_SETTINGS, BUTTON_ACTION_QUIT_GAME, BUTTON_ACTION_RESUME_GAME,
    BUTTON_ACTION_SAVE_GAME, BUTTON_ACTION_LOAD_GAME, BUTTON_ACTION_PAUSE_SETTINGS,
    BUTTON_ACTION_GOTO_MAIN_MENU,
    BUTTON_ACTION_GOTO_SINGLE_PLAYER_SETUP, 
    BUTTON_ACTION_GOTO_TWO_PLAYER_SETUP     
} MenuButtonAction;

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

void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos);
void DrawMenuScreen(void);
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr, Vector2 virtualMousePos);
void DrawPlayerModeMenuScreen(void);

// Assinatura de Menu_RequestSaveLoadScreen CORRIGIDA para 3 argumentos.
// O GameModeType será obtido da variável global currentGameMode dentro da sua implementação em Menu.c.
void Menu_RequestSaveLoadScreen(GameState *currentScreen_ptr, bool isSaving, GameState fromScreen);
void Menu_RequestMainMenu(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int *musicIsCurrentlyPlaying_ptr);

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