#ifndef GAME_H
#define GAME_H

// Inclui headers de baixo nível dos quais este header depende
#include "raylib.h"
#include "Classes.h"
#include "WorldLoading.h"

// --- Constantes Globais ---
#define MAX_MUSIC_PLAYLIST_SIZE 4
#define MAX_PLAYERS_SUPPORTED 2

// --- Enums Globais do Jogo ---
typedef enum {
    GAMESTATE_INTRO,
    GAMESTATE_MENU,
    GAMESTATE_PLAYER_MODE_MENU,
    GAMESTATE_CHARACTER_CREATION,
    GAMESTATE_SAVE_LOAD_MENU,
    GAMESTATE_SETTINGS,
    GAMESTATE_PLAYING,
    GAMESTATE_PAUSE,
    GAMESTATE_INVENTORY,
    GAMESTATE_BATTLE
} GameState;
typedef enum { GAME_MODE_UNINITIALIZED = -1, GAME_MODE_SINGLE_PLAYER, GAME_MODE_TWO_PLAYER } GameModeType;
typedef enum { BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM } BorderDirection;

// --- Variáveis Globais Externas (Definidas em main.c) ---
extern GameModeType currentGameMode;
extern int currentActivePlayers;
extern const int virtualScreenWidth; extern const int virtualScreenHeight;
extern const int WORLD_MAP_MIN_X; extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y; extern const int WORLD_MAP_MAX_Y;
extern const int gameSectionWidthMultiplier; extern const int gameSectionHeightMultiplier;

// --- Protótipos de Funções de Game.c ---
void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float sectionActualWidth, float sectionActualHeight);
void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift, const WorldSection* activeSection);
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]);
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers, const WorldSection* worldSection);
void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, bool isPlaying, int currentMapX, int currentMapY);

// CORREÇÃO: Nova função para reposicionar jogadores após a transição.
void RepositionPlayersForTransition(Player players[], int numActivePlayers, BorderDirection direction, const WorldSection* newSection);

// CORREÇÃO: A função de update agora retorna a direção da transição para o loop principal.
BorderDirection UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers, Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, bool *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera, const WorldSection* activeSection);


#endif // GAME_H