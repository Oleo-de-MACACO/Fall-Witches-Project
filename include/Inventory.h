#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"
#include "../include/Classes.h" // Para Player, InventoryItem
#include "../include/Game.h"    // Para GameState, Music

// Enumeração para as diferentes abas do painel de inventário/status do jogador.
typedef enum {
    TAB_INVENTORY = 0,
    TAB_EQUIPMENT,
    TAB_STATUS,
    MAX_INVENTORY_TABS
} InventoryTabType;

// --- Protótipos de Função ---

void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr);

/**
 * @brief Desenha APENAS o fundo do jogo para a tela de inventário.
 * Esta função é chamada por main.c DENTRO de BeginMode2D()/EndMode2D() para desenhar o mundo do jogo atrás da UI.
 * @param players_arr Array de jogadores para desenhar o fundo (tela de jogo).
 * @param background_players_arr (Pode ser o mesmo que players_arr) Array de jogadores para DrawPlayingScreen.
 * @param background_musicVolume Volume da música para o fundo.
 * @param background_currentMusicIndex Índice da música para o fundo.
 * @param background_musicIsPlaying Estado da música para o fundo.
 * @param background_mapX Coordenada X do mapa para o fundo.
 * @param background_mapY Coordenada Y do mapa para o fundo.
 */
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY);

/**
 * @brief Desenha os elementos da interface do usuário (UI) do inventário.
 * Inclui o filtro escuro, o título, os painéis dos jogadores e as instruções.
 * Esta função deve ser chamada DEPOIS de EndMode2D() se o fundo do jogo for desenhado com uma câmera,
 * para que os elementos da UI fiquem fixos na tela.
 * @param players_arr Array de estruturas Player contendo os dados a serem exibidos nos painéis.
 */
void DrawInventoryUIElements(Player players_arr[]);


bool AddItemToInventory(Player *player, const char *itemName, int quantity);
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity);

#endif // INVENTORY_H