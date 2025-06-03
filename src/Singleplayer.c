#include "../include/Singleplayer.h"
#include "../include/Game.h" 
#include "raylib.h"      

// Extern global variables 
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int gameSectionWidthMultiplier; 
extern const int gameSectionHeightMultiplier; 
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

// Declaração da função move_character, pois ela é chamada aqui.
// O ideal é que ela esteja em Game.h se for usada por múltiplos arquivos .c
// ou passada como ponteiro de função. (Já está em Game.h)
// extern void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift);


void SinglePlayer_HandleMovement(Player *player) {
    if (!player) return;

    // Jogador single-player (players[0]) AGORA usa WASD + Shift Esquerdo
    move_character(player, KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT);
}

bool SinglePlayer_CheckMapTransition(Player *player, int *currentMapX_ptr, int *currentMapY_ptr) {
    if (!player || !currentMapX_ptr || !currentMapY_ptr) return false;

    int tolerance = 10;
    int currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
    int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;
    BorderDirection player_border = BORDER_NONE; // BorderDirection deve estar definido em Game.h

    if (player->posx <= tolerance) player_border = BORDER_LEFT;
    else if (player->posx >= currentSectionTotalWidth - player->width - tolerance) player_border = BORDER_RIGHT;
    else if (player->posy <= tolerance) player_border = BORDER_TOP;
    else if (player->posy >= currentSectionTotalHeight - player->height - tolerance) player_border = BORDER_BOTTOM;

    if (player_border != BORDER_NONE) {
        int nextMapX = *currentMapX_ptr;
        int nextMapY = *currentMapY_ptr;

        switch (player_border) {
            case BORDER_LEFT:   nextMapX--; break;
            case BORDER_RIGHT:  nextMapX++; break;
            case BORDER_TOP:    nextMapY--; break;
            case BORDER_BOTTOM: nextMapY++; break;
            default: return false;
        }
        
        if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X &&
            nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {
            
            *currentMapX_ptr = nextMapX;
            *currentMapY_ptr = nextMapY;

            switch (player_border) {
                case BORDER_RIGHT: player->posx = player->width / 2; break;
                case BORDER_LEFT:  player->posx = currentSectionTotalWidth - player->width - (player->width / 2); break;
                case BORDER_TOP:   player->posy = currentSectionTotalHeight - player->height - (player->height / 2); break;
                case BORDER_BOTTOM:player->posy = player->height / 2; break;
                default: break;
            }
            // Clamp position
            if (player->posx < 0) player->posx = 0;
            if (player->posx > currentSectionTotalWidth - player->width) player->posx = currentSectionTotalWidth - player->width;
            if (player->posy < 0) player->posy = 0; // Corrigido para player->posy
            if (player->posy > currentSectionTotalHeight - player->height) player->posy = currentSectionTotalHeight - player->height; // Corrigido para player->posy
            
            return true; 
        }
    }
    return false; 
}