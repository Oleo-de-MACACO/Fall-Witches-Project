#include "../include/Game.h"

void move_character(int *posx, int *posy, int width, int height, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
  if(IsKeyDown(keyLeft)){
    if(IsKeyDown(keyShift)){
      *posx -= 5;
    } else {
      *posx -= 3;
    }
  }
  if(IsKeyDown(keyRight) ){
    if(IsKeyDown(keyShift)){
      *posx += 5;
    } else {
      *posx += 3;
    }
  }
  if(IsKeyDown(keyUp)){
    if(IsKeyDown(keyShift)){
      *posy -= 5;
    } else {
      *posy -= 3;
    }
  }
  if(IsKeyDown(keyDown)){
    if(IsKeyDown(keyShift)){
      *posy += 5;
    } else {
      *posy += 3;
    }
  }

  if(*posx < 0) *posx = 0;
  if(*posx > width - (PLAYER_SIZE * 4)) *posx = width - (PLAYER_SIZE * 4);
  if(*posy < PLAYER_SIZE) *posy = PLAYER_SIZE;
  if(*posy > height - (PLAYER_SIZE * 4)) *posy = height - (PLAYER_SIZE * 4);
}

void menu(Texture *t, const char *file_menu){
    *t = LoadTexture(file_menu);

    ActionMenu action = ACTION_START;

    switch(action){
        case ACTION_START:
            break;
        case ACTION_LOAD:
            break;
        case ACTION_CONFIG:
            break;
        case ACTION_EXIT:
            break;
    }
}
