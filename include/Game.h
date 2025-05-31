#ifndef GAME_H
#define GAME_H

#include "Classes.h" // Sua struct Player e outras definições de classes
#include "raylib.h"

// --- Constantes do Jogo ---
#define MAX_SIZE 4      //
#define MAX_PLAYERS 2   //
#define PLAYER_SIZE 20  //

// --- Estados do Jogo (Cenas) ---
typedef enum {
    GAMESTATE_MENU,     //
    GAMESTATE_INTRO,    //
    GAMESTATE_PLAYING,  //
    GAMESTATE_PAUSE     //
} GameState;

// --- Protótipos das Funções de Lógica do Jogo (a serem implementadas em Game.c) ---

// Sua função de movimentação de personagem existente
void move_character(int *posx, int *posy, int screenWidth, int screenHeight, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift); //

// Função para inicializar/carregar recursos do jogo (jogadores, texturas, músicas)
void InitGameResources(Player players[], Texture2D *player1Tex, Music mainPlaylist[]);

// Funções para a cena de MENU
void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr);
void DrawMenuScreen(void);

// Funções para a cena de INTRODUÇÃO
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr);
void DrawIntroScreen(void);

// Funções para a cena de JOGANDO (GAMEPLAY)
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players[], Texture2D player1Tex, Music playlist[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr);
void DrawPlayingScreen(Player players[], Texture2D player1Tex, float currentVolume, int currentMusicIndex, int isPlaying);

// Funções para a cena de PAUSA
void UpdatePauseScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int isPlaying);
void DrawPauseScreen(Player players[], Texture2D player1Tex, float currentVolume, int currentMusicIndex, int isPlaying); // Para desenhar o fundo

#endif // GAME_H
