#ifndef SINGLEPLAYER_H
#define SINGLEPLAYER_H


/**
 * @brief Lida com a movimentação do jogador único.
 * @param player Ponteiro para o jogador single-player.
 */
#include "Classes.h"
void SinglePlayer_HandleMovement(Player *player);

/**
 * @brief Verifica e lida com a transição de mapa para o jogador único.
 * @param player Ponteiro para o jogador single-player.
 * @param currentMapX_ptr Ponteiro para a coordenada X atual do mapa global.
 * @param currentMapY_ptr Ponteiro para a coordenada Y atual do mapa global.
 * @return true se uma transição de mapa ocorreu, false caso contrário.
 */
bool SinglePlayer_CheckMapTransition(Player *player, int *currentMapX_ptr, int *currentMapY_ptr);

#endif // SINGLEPLAYER_H