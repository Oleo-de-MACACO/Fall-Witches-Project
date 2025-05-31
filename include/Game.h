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
    GAMESTATE_PAUSE     
} GameState;

// --- Protótipos das Funções de Lógica do Jogo (implementadas em Game.c) ---

void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight, int screenWidth, int screenHeight, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift); 

// CORRIGIDO: Removido o parâmetro Texture2D *player1Tex
void InitGameResources(Player players[], Music mainPlaylist[]);

void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr);
void DrawMenuScreen(void);

void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr);

// CORRIGIDO: Removido o parâmetro Texture2D player1Tex
void DrawPlayingScreen(Player players[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY);

void UpdatePauseScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int isPlaying);

// CORRIGIDO: Removido o parâmetro Texture2D player1Tex
void DrawPauseScreen(Player players[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY);

#endif // GAME_H