#ifndef GAME_H
#define GAME_H

#include "Classes.h"

#define MAX_SIZE 4
#define MAX_PLAYERS 2
#define PLAYER_SIZE 20

typedef enum {
    GAMESTATE_MENU,
    GAMESTATE_INTRO,
    GAMESTATE_PLAYING,
    GAMESTATE_PAUSE
} GameState;

typedef enum {
    ACTION_START,
    ACTION_LOAD,
    ACTION_CONFIG,
    ACTION_EXIT
} ActionMenu;

void move_character(int *posx, int *posy, int width, int height, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift);
void menu(Texture *t, const char *file_menu);

#endif
