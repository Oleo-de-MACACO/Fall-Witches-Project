#ifndef GAME_H
#define GAME_H

#include "Classes.h" // Contém Player, Classe, SpriteType, CharacterDirection e defines de frames
#include "raylib.h"

// Constantes do Jogo
#define MAX_MUSIC_PLAYLIST_SIZE 4 
#define MAX_PLAYERS_SUPPORTED 2   

// Estados do Jogo (Cenas)
typedef enum {
    GAMESTATE_MENU, 
    GAMESTATE_INTRO, 
    GAMESTATE_PLAYER_MODE_MENU,
    GAMESTATE_CHARACTER_CREATION, 
    GAMESTATE_PLAYING, 
    GAMESTATE_PAUSE,
    GAMESTATE_INVENTORY, 
    GAMESTATE_SAVE_LOAD_MENU,
    GAMESTATE_SETTINGS 
} GameState;

// Tipos de Modo de Jogo
typedef enum {
    GAME_MODE_UNINITIALIZED = -1, 
    GAME_MODE_SINGLE_PLAYER,
    GAME_MODE_TWO_PLAYER
} GameModeType;

// Enum para detecção de borda (usado por Game.c e Singleplayer.c)
typedef enum { 
    BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM 
} BorderDirection;

// Variáveis globais (definidas em main.c)
extern GameModeType currentGameMode;
extern int currentActivePlayers; 

// Protótipos de Funções de Game.c
void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float vWidth, float vHeight);
void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift);
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]); // mainPlaylist_arr pode ser NULL
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers); 
void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY);
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers, Music playlist_arr[], // playlist_arr pode ser NULL
                         int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera); 

#endif // GAME_H