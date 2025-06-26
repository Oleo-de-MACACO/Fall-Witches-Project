#include "../include/GameProgress.h"
#include <string.h> // Para memset
#include "raylib.h" // Para TraceLog

// Definição da variável global de progresso do jogo
GameProgress g_gameProgress;

/**
 * @brief Reseta todo o progresso do jogo para um estado inicial.
 * Zera todos os mapas visitados e outras futuras flags de progresso.
 */
void Progress_Reset(void) {
    memset(&g_gameProgress, 0, sizeof(GameProgress));
    TraceLog(LOG_INFO, "GameProgress: Progresso do jogo resetado para novo jogo.");
}

/**
 * @brief Converte coordenadas do mundo do mapa para índices de array.
 * @param mapX Coordenada X do mundo (-10 a 10).
 * @param mapY Coordenada Y do mundo (-10 a 10).
 * @param ix Ponteiro para armazenar o índice X do array (0 a 20).
 * @param iy Ponteiro para armazenar o índice Y do array (0 a 20).
 */
static void ConvertMapCoordsToArrayIndices(int mapX, int mapY, int* ix, int* iy) {
    *ix = mapX + MAP_GRID_OFFSET_X;
    *iy = mapY + MAP_GRID_OFFSET_Y;
}

/**
 * @brief Marca um mapa como visitado no registro de progresso global.
 */
void Progress_MapWasVisited(int mapX, int mapY) {
    int ix, iy;
    ConvertMapCoordsToArrayIndices(mapX, mapY, &ix, &iy);
    if (ix >= 0 && ix < MAP_GRID_WIDTH && iy >= 0 && iy < MAP_GRID_HEIGHT) {
        if (!g_gameProgress.visitedMaps[iy][ix]) {
            g_gameProgress.visitedMaps[iy][ix] = true;
            TraceLog(LOG_INFO, "GameProgress: Mapa (%d,%d) marcado como visitado.", mapX, mapY);
        }
    } else {
        TraceLog(LOG_WARNING, "GameProgress: Tentativa de marcar mapa fora dos limites (%d,%d) como visitado.", mapX, mapY);
    }
}

/**
 * @brief Verifica se um mapa já foi visitado no registro de progresso global.
 */
bool Progress_HasVisitedMap(int mapX, int mapY) {
    int ix, iy;
    ConvertMapCoordsToArrayIndices(mapX, mapY, &ix, &iy);
    if (ix >= 0 && ix < MAP_GRID_WIDTH && iy >= 0 && iy < MAP_GRID_HEIGHT) {
        return g_gameProgress.visitedMaps[iy][ix];
    }
    // Trata coordenadas fora dos limites como "visitadas" para não acionar eventos lá por engano.
    TraceLog(LOG_WARNING, "GameProgress: Checando mapa fora dos limites (%d,%d).", mapX, mapY);
    return true;
}