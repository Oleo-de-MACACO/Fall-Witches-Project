#ifndef GAME_H 
#define GAME_H

#include "Classes.h" 
#include "raylib.h"  

// --- Constantes do Jogo ---
#define MAX_SIZE 4      
#define MAX_PLAYERS 2   
#define PLAYER_SIZE 20  // Usado no seu código original

// --- Estados do Jogo (Cenas) ---
typedef enum {
    GAMESTATE_MENU,
    GAMESTATE_INTRO,
    GAMESTATE_PLAYING,
    GAMESTATE_PAUSE,
    GAMESTATE_INVENTORY,
    GAMESTATE_SAVE_LOAD_MENU // <-- NOVO ESTADO para a tela de seleção de slot de save/load
} GameState;

// --- Protótipos das Funções de Lógica do Jogo (implementadas em Game.c) ---

void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight, int screenWidth, int screenHeight, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift); 

// CORRIGIDO: Removido o parâmetro Texture2D *player1Tex
void InitGameResources(Player players[], Music mainPlaylist[]);

// CORRIGIDO: Removido o parâmetro Texture2D player1Tex
void DrawPlayingScreen(Player players[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY);

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr);

#endif // GAME_H