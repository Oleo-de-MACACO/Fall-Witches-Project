#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"
#include "../include/Classes.h" // Para as structs Player e InventoryItem
#include "../include/Game.h"    // Para o enum GameState e outras definições de jogo

// Enum para as diferentes abas do painel do jogador
typedef enum {
    TAB_INVENTORY = 0,      // Inventário de itens
    TAB_EQUIPMENT,          // Equipamentos vestidos
    TAB_STATUS,             // Status e atributos do jogador
    MAX_INVENTORY_TABS      // Número total de abas (usado para contagem e navegação)
} InventoryTabType;

// Protótipos de função (assinaturas externas permanecem as mesmas)
void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr);
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY);
bool AddItemToInventory(Player *player, const char *itemName, int quantity);
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity);

#endif // INVENTORY_H