#ifndef SAVELOAD_H
#define SAVELOAD_H

#include "../include/Game.h"    // *** ADICIONADO: Necessário para GameModeType e Player ***
#include <stdbool.h>

#define MAX_SAVE_SLOTS 30
#define SAVE_BASE_DIRECTORY "Saves"
#define SAVE_SUBDIR_SINGLEPLAYER "SinglePlayer"
#define SAVE_SUBDIR_TWOPLAYER    "TwoPlayer"

bool EnsureSavesDirectoryExists(GameModeType gameMode);
bool DoesSaveSlotExist(int slot_number, GameModeType gameMode);
void GetSaveFileName(char *buffer, int buffer_size, int slot_number, GameModeType gameMode);
bool SaveGame(Player players[], int num_players_to_save, int slot_number, int currentMapX, int currentMapY, GameModeType gameMode);
bool LoadGame(Player players[], int max_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY, GameModeType gameMode, int *numPlayersLoaded);

#endif // SAVELOAD_H