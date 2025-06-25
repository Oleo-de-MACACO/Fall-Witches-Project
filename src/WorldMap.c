#include "../include/WorldMap.h"
#include <math.h>

// --- Variáveis Globais Externas (definidas em main.c, extern em Game.h) ---
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

/**
 * @brief Verifica e processa transições de mapa com base na posição dos jogadores.
 * Esta nova versão é mais robusta, tratando os eixos X e Y de forma independente
 * e garantindo que o jogador seja reposicionado corretamente para longe da borda.
 *
 * @param players Array de jogadores.
 * @param numActivePlayers Número de jogadores ativos.
 * @param currentMapX_ptr Ponteiro para a coordenada X global do mapa.
 * @param currentMapY_ptr Ponteiro para a coordenada Y global do mapa.
 * @param gameMode O modo de jogo atual.
 * @param sectionActualWidth A largura real da seção do mapa carregada.
 * @param sectionActualHeight A altura real da seção do mapa carregada.
 * @return true se uma transição de mapa ocorreu, false caso contrário.
 */
bool WorldMap_CheckTransition(Player players[], int numActivePlayers, int *currentMapX_ptr, int *currentMapY_ptr, GameModeType gameMode, float sectionActualWidth, float sectionActualHeight) {
    if (!players || !currentMapX_ptr || !currentMapY_ptr || numActivePlayers <= 0) return false;
    if (sectionActualWidth <= 0 || sectionActualHeight <= 0) return false;

    // Para o modo de um jogador ou como base para o multijogador, verificamos o primeiro jogador.
    Player* p = &players[0]; 
    const int tolerance = 5; // Uma tolerância menor para a detecção da borda.
    bool transition_occurred = false;

    // Trata o modo de jogo single-player e a base para o two-player.
    if (gameMode == GAME_MODE_SINGLE_PLAYER || (gameMode == GAME_MODE_TWO_PLAYER && numActivePlayers >= 1)) {
        
        // Verifica a transição no eixo X
        if ((float)p->posx <= (float)tolerance && *currentMapX_ptr > WORLD_MAP_MIN_X) {
            (*currentMapX_ptr)--;
            p->posx = (int)(sectionActualWidth - (float)p->width - (float)tolerance); // Reposiciona na borda oposta
            transition_occurred = true;
        } else if ((float)p->posx >= sectionActualWidth - (float)p->width - (float)tolerance && *currentMapX_ptr < WORLD_MAP_MAX_X) {
            (*currentMapX_ptr)++;
            p->posx = (int)(tolerance); // Reposiciona na borda oposta
            transition_occurred = true;
        }

        // Verifica a transição no eixo Y
        if ((float)p->posy <= (float)tolerance && *currentMapY_ptr > WORLD_MAP_MIN_Y) {
            (*currentMapY_ptr)--;
            p->posy = (int)(sectionActualHeight - (float)p->height - (float)tolerance); // Reposiciona na borda oposta
            transition_occurred = true;
        } else if ((float)p->posy >= sectionActualHeight - (float)p->height - (float)tolerance && *currentMapY_ptr < WORLD_MAP_MAX_Y) {
            (*currentMapY_ptr)++;
            p->posy = (int)(tolerance); // Reposiciona na borda oposta
            transition_occurred = true;
        }
    }
    
    // Se a transição ocorreu e estamos em modo de dois jogadores, move o segundo jogador junto.
    if (transition_occurred && gameMode == GAME_MODE_TWO_PLAYER && numActivePlayers >= 2) {
         // Simplesmente move o jogador 2 para perto do jogador 1. Pode ser melhorado no futuro.
        players[1].posx = p->posx + p->width + 10;
        players[1].posy = p->posy;
    }

    return transition_occurred;
}