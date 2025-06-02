#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"       // Para GameState, Player, Music. Game.h inclui Classes.h
// #include "Classes.h" // Removido, pois Game.h já o inclui.
#include <stdbool.h>

extern bool g_request_exit;

typedef enum {
    BUTTON_ACTION_NONE, BUTTON_ACTION_GOTO_PLAYER_MODE_MENU, BUTTON_ACTION_NEW_GAME_SETUP_SLOT,
    BUTTON_ACTION_SETTINGS, BUTTON_ACTION_QUIT_GAME, BUTTON_ACTION_RESUME_GAME,
    BUTTON_ACTION_SAVE_GAME, BUTTON_ACTION_LOAD_GAME, BUTTON_ACTION_PAUSE_SETTINGS,
    BUTTON_ACTION_GOTO_MAIN_MENU
} MenuButtonAction;

// Estrutura MenuButton CORRIGIDA para a ordem original dos membros
typedef struct {
    Rectangle rect;         // Posição e dimensões do botão
    const char *text;       // Texto exibido no botão
    Color base_color;       // Cor normal do botão
    Color hover_color;      // Cor do botão ao passar o mouse
    Color disabled_color;   // Cor do botão desabilitado
    Color text_color;       // Cor do texto do botão
    bool is_active;         // Se o botão está ativo (pode ser clicado)
    bool is_hovered;        // Se o mouse está sobre o botão
    MenuButtonAction action; // Ação a ser executada pelo botão
} MenuButton;

// --- Funções de Tela (Menu Principal, Intro, Modo de Jogador) ---
void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos);
void DrawMenuScreen(void);
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr, Vector2 virtualMousePos);
void DrawPlayerModeMenuScreen(void);

// --- Funções de Gerenciamento de Contexto para Save/Load (usadas por LoadSaveUI.c) ---
void Menu_RequestSaveLoadScreen(GameState *currentScreen_ptr, bool isSaving, GameState fromScreen);
void Menu_RequestMainMenu(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int *musicIsCurrentlyPlaying_ptr);

// Getters para o contexto do Save/Load
bool Menu_IsInSaveMode(void);
bool Menu_IsNewGameFlow(void);
int Menu_GetSaveLoadSubState(void);
int Menu_GetSelectedSlotForAction(void);
GameState Menu_GetPreviousScreenBeforeSaveLoad(void);

// Setters para o contexto do Save/Load
void Menu_SetSaveLoadSubState(int subState);
void Menu_SetSelectedSlotForAction(int slot);
void Menu_SetIsNewGameFlow(bool isNewGameFlow);

#endif // MENU_H