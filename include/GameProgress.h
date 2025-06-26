#ifndef GAME_PROGRESS_H
#define GAME_PROGRESS_H

#include <stdbool.h>

// Define as dimensões da grade do mapa para rastreamento.
// Baseado em WORLD_MAP_MIN/MAX de main.c (-10 a 10 => 21 de largura/altura)
#define MAP_GRID_WIDTH 21
#define MAP_GRID_HEIGHT 21
#define MAP_GRID_OFFSET_X 10 // Offset para converter mapX (-10 a 10) para índice de array (0 a 20)
#define MAP_GRID_OFFSET_Y 10 // Offset para converter mapY (-10 a 10) para índice de array (0 a 20)

/**
 * @brief Estrutura para guardar dados de progresso do jogo que persistem entre sessões.
 */
typedef struct {
    bool visitedMaps[MAP_GRID_HEIGHT][MAP_GRID_WIDTH];
    // Adicionar outras flags de progresso aqui no futuro (ex: quests concluídas)
} GameProgress;

// --- Variável de Progresso Global ---
// Definida em GameProgress.c, acessível externamente
extern GameProgress g_gameProgress;

// --- Funções de Gerenciamento de Progresso ---

/**
 * @brief Reseta todo o progresso do jogo para um estado de novo jogo.
 */
void Progress_Reset(void);

/**
 * @brief Marca um mapa como visitado no registro de progresso.
 * @param mapX Coordenada X do mapa.
 * @param mapY Coordenada Y do mapa.
 */
void Progress_MapWasVisited(int mapX, int mapY);

/**
 * @brief Verifica se um mapa já foi visitado.
 * @param mapX Coordenada X do mapa.
 * @param mapY Coordenada Y do mapa.
 * @return true se o mapa já foi visitado, false caso contrário.
 */
bool Progress_HasVisitedMap(int mapX, int mapY);

#endif // GAME_PROGRESS_H