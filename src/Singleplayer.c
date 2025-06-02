#include "../include/Singleplayer.h"
#include "../include/Game.h" // Para BorderDirection, GetPlayerBorderCondition, etc. (se necessário)

// Variáveis globais externas (de main.c) se necessárias diretamente aqui
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int gameSectionWidthMultiplier; // De Game.c
extern const int gameSectionHeightMultiplier; // De Game.c

// Funções de Game.c que podem ser chamadas ou referenciadas
// extern BorderDirection GetPlayerBorderCondition(Player player_obj); // Já em Game.h indiretamente ou usado internamente
extern void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift);


void SinglePlayer_HandleMovement(Player *player) {
    if (!player) return;
    // Controles padrão para o jogador 0 podem ser usados, ou definir controles específicos aqui
    move_character(player, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT);
}

bool SinglePlayer_CheckMapTransition(Player *player, int *currentMapX_ptr, int *currentMapY_ptr) {
    if (!player) return false;

    // A função GetPlayerBorderCondition é de Game.c, mas não está em Game.h
    // Precisamos de uma forma de chamá-la ou replicar a lógica.
    // Por agora, vamos assumir que GetPlayerBorderCondition está acessível ou replicamos:
    
    // Lógica de GetPlayerBorderCondition simplificada (ou chame a original se movida para Game.h)
    int tolerance = 10;
    int currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
    int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;
    BorderDirection player_border = BORDER_NONE;

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
            default: return false; // Não deveria acontecer
        }

        // Verifica se as novas coordenadas do mapa estão dentro dos limites do mundo.
        // Estas constantes WORLD_MAP_MIN/MAX precisam ser acessíveis.
        extern const int WORLD_MAP_MIN_X, WORLD_MAP_MAX_X, WORLD_MAP_MIN_Y, WORLD_MAP_MAX_Y;
        if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X &&
            nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {
            
            *currentMapX_ptr = nextMapX;
            *currentMapY_ptr = nextMapY;

            // Reposiciona o jogador na nova seção
            switch (player_border) {
                case BORDER_RIGHT: player->posx = player->width / 2; break;
                case BORDER_LEFT:  player->posx = currentSectionTotalWidth - player->width - (player->width / 2); break;
                case BORDER_TOP:   player->posy = currentSectionTotalHeight - player->height - (player->height / 2); break;
                case BORDER_BOTTOM:player->posy = player->height / 2; break;
                default: break;
            }
            // Clamp position (já feito em move_character, mas bom garantir após transição)
            if (player->posx < 0) player->posx = 0;
            if (player->posx > currentSectionTotalWidth - player->width) player->posx = currentSectionTotalWidth - player->width;
            if (player->posy < 0) player->posy = 0;
            if (player->posy > currentSectionTotalHeight - player->height) player->posy = currentSectionTotalHeight - player->height;
            
            return true; // Transição ocorreu
        }
    }
    return false; // Nenhuma transição
}