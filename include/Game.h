#ifndef GAME_H
#define GAME_H

#include "Classes.h"
#include "raylib.h"

// --- Constantes do Jogo ---
#define MAX_SIZE 4      // Tamanho máximo para arrays (ex: playlist de música)
#define MAX_PLAYERS 2   // Número máximo de jogadores
#define PLAYER_SIZE 20  // Tamanho do jogador (usado em algum lugar? verificar uso)

// --- Estados do Jogo (Cenas) ---
typedef enum {
    GAMESTATE_MENU,
    GAMESTATE_INTRO,
    GAMESTATE_PLAYER_MODE_MENU,
    GAMESTATE_CHARACTER_CREATION, // Estado para criação de personagem
    GAMESTATE_PLAYING,
    GAMESTATE_PAUSE,
    GAMESTATE_INVENTORY,
    GAMESTATE_SAVE_LOAD_MENU
} GameState;

// --- Protótipos das Funções de Lógica do Jogo (implementadas em Game.c) ---

// Declaração de move_character MODIFICADA: removidos screenWidth e screenHeight,
// pois agora usará virtualScreenWidth/Height internamente.
void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight,
                    int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift);

void InitGameResources(Player players[], Music mainPlaylist[]);

// Declaração de PrepareNewGameSession MODIFICADA: removidos screenWidth e screenHeight,
// pois agora usará virtualScreenWidth/Height internamente para posicionar jogadores.
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY);

// Desenha a tela de jogo. Assume que está sendo chamada dentro de um BeginTextureMode(targetRenderTexture).
// O layout interno (UI, etc.) usará virtualScreenWidth/Height.
void DrawPlayingScreen(Player players[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY);

// Atualiza a lógica da tela de jogo.
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players[], Music playlist[],
                         int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera); // Added Camera2D*

#endif // GAME_H