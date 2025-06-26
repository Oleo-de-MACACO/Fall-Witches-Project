#include "../include/WorldMap.h"
#include <math.h>

// --- Variáveis Globais Externas (definidas em main.c, extern em Game.h) ---
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

/**
 * @brief Verifica se a posição de um jogador está em uma borda que permite transição.
 * Esta versão da função é puramente de detecção, ela não modifica nenhum dado.
 * A lógica de mudar o mapa e reposicionar o jogador foi movida para Game.c e main.c.
 *
 * @param p Ponteiro para o jogador a ser verificado.
 * @param currentMapX Coordenada X atual do mundo.
 * @param currentMapY Coordenada Y atual do mundo.
 * @param sectionActualWidth A largura da seção atual do mapa.
 * @param sectionActualHeight A altura da seção atual do mapa.
 * @return Um enum BorderDirection indicando qual borda foi tocada, ou BORDER_NONE se nenhuma transição deve ocorrer.
 */
BorderDirection WorldMap_CheckTransition(Player* p, int currentMapX, int currentMapY, float sectionActualWidth, float sectionActualHeight) {
    if (!p || sectionActualWidth <= 0 || sectionActualHeight <= 0) {
        return BORDER_NONE;
    }

    const int tolerance = 5; // Quão perto da borda o jogador precisa estar.

    // Verifica a transição para a ESQUERDA (só se não estiver na borda do mundo)
    if ((float)p->posx <= (float)tolerance && currentMapX > WORLD_MAP_MIN_X) {
        return BORDER_LEFT;
    }
    // Verifica a transição para a DIREITA
    if ((float)p->posx >= sectionActualWidth - (float)p->width - (float)tolerance && currentMapX < WORLD_MAP_MAX_X) {
        return BORDER_RIGHT;
    }
    // Verifica a transição para CIMA
    if ((float)p->posy <= (float)tolerance && currentMapY > WORLD_MAP_MIN_Y) {
        return BORDER_TOP;
    }
    // Verifica a transição para BAIXO
    if ((float)p->posy >= sectionActualHeight - (float)p->height - (float)tolerance && currentMapY < WORLD_MAP_MAX_Y) {
        return BORDER_BOTTOM;
    }

    // Se nenhuma condição for atendida, nenhuma transição ocorre.
    return BORDER_NONE;
}