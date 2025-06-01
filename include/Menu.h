#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"       // Para GameState, Player, Music
#include "Classes.h"    // Para Player (embora já incluído por Game.h)
#include <stdbool.h>    // Para bool

// Flag global de saída (declarada em main.c)
extern bool g_request_exit;

// Ações que os botões do menu podem executar
typedef enum {
    BUTTON_ACTION_NONE,
    BUTTON_ACTION_GOTO_PLAYER_MODE_MENU,
    BUTTON_ACTION_NEW_GAME_SETUP_SLOT,
    BUTTON_ACTION_SETTINGS,
    BUTTON_ACTION_QUIT_GAME,
    BUTTON_ACTION_RESUME_GAME,
    BUTTON_ACTION_SAVE_GAME,
    BUTTON_ACTION_LOAD_GAME,
    BUTTON_ACTION_PAUSE_SETTINGS,
    BUTTON_ACTION_GOTO_MAIN_MENU,
    BUTTON_ACTION_CONFIRM_CHARACTER_CREATION
} MenuButtonAction;

// Estrutura para um botão de menu
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

// --- Protótipos para as Funções de Tela ---

// Menu Principal
// MODIFICADO: Adicionado parâmetro Vector2 virtualMousePos para interações com o mouse virtual.
void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos);
void DrawMenuScreen(void); // Desenha na tela virtual

// Introdução
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void); // Desenha na tela virtual

// Menu de Modo de Jogador (seleciona Novo Jogo ou Carregar)
// MODIFICADO: Adicionado parâmetro Vector2 virtualMousePos.
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr, Vector2 virtualMousePos);
void DrawPlayerModeMenuScreen(void); // Desenha na tela virtual

// Tela de Criação de Personagem
void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr);
void DrawCharacterCreationScreen(Player players[], int currentPlayerFocus); // Desenha na tela virtual

// Menu de Pausa
// MODIFICADO: Adicionado parâmetro Vector2 virtualMousePos.
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr, Vector2 virtualMousePos);
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY); // Desenha na tela virtual

// Tela de Save/Load Slots
// MODIFICADO: Adicionado parâmetro Vector2 virtualMousePos.
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos);
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY); // Desenha na tela virtual


#endif // MENU_H