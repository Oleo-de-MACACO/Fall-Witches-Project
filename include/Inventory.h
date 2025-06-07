#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"
#include "../include/Game.h"    // *** ADICIONADO: Necessário para GameState, Music, Player ***
#include "../include/Classes.h" // Classes.h não precisa mais incluir Game.h

typedef enum {
    TAB_INVENTORY = 0, TAB_EQUIPMENT, TAB_STATUS, MAX_INVENTORY_TABS
} InventoryTabType;

void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], bool *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr);
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, bool bg_music_playing, int background_mapX, int background_mapY);
void DrawInventoryUIElements(Player players_arr[]);
bool AddItemToInventory(Player *player, const char *itemName, int quantity);
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity);

#endif // INVENTORY_H