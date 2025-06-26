#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "Game.h" // *** ESSENCIAL: Para Player, GameModeType, BorderDirection ***

/**
 * @brief Verifica se a posição de um jogador está em uma borda de transição.
 * @param p O jogador a ser verificado.
 * @param currentMapX A coordenada X atual do mapa.
 * @param currentMapY A coordenada Y atual do mapa.
 * @param sectionActualWidth A largura da seção atual do mapa.
 * @param sectionActualHeight A altura da seção atual do mapa.
 * @return A direção da borda que foi tocada (BORDER_LEFT, etc.), ou BORDER_NONE.
 */
BorderDirection WorldMap_CheckTransition(Player* p, int currentMapX, int currentMapY, float sectionActualWidth, float sectionActualHeight);

#endif // WORLD_MAP_H