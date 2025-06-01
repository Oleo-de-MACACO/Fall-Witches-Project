#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "Game.h"       
#include "Classes.h"    
#include <stdbool.h>    

extern bool g_request_exit; // Flag global de saída

// Ações que os botões do menu podem executar
typedef enum {
    BUTTON_ACTION_NONE,
    // BUTTON_ACTION_START_2PLAYERS, // Removido ou renomeado, não mais usado diretamente assim
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

// --- Protótipos para as Funções de Tela ---

// Menu Principal
// MODIFICADO: Removidos parâmetros de música não utilizados
void UpdateMenuScreen(GameState *currentScreen_ptr);
void DrawMenuScreen(void);

// Introdução
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);

// Menu de Modo de Jogador (seleciona Novo Jogo ou Carregar)
// Assinatura original mantida, pois pode precisar da música no futuro ou para consistência
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr);
void DrawPlayerModeMenuScreen(void);

// Tela de Criação de Personagem
void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr);
void DrawCharacterCreationScreen(Player players[], int currentPlayerFocus); 

// Menu de Pausa
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr);
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY);

// Tela de Save/Load Slots
// MODIFICADO: Adicionado currentVolume
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr);
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY);


#endif // MENU_H