#ifndef SAVELOAD_H
#define SAVELOAD_H

#include "../include/Classes.h" 
#include "../include/Game.h"    // Para GameModeType
#include <stdbool.h>           

#define MAX_SAVE_SLOTS 30
// SAVE_DIRECTORY agora é um prefixo, subpastas serão adicionadas
#define SAVE_BASE_DIRECTORY "Saves" 
#define SAVE_SUBDIR_SINGLEPLAYER "SinglePlayer"
#define SAVE_SUBDIR_TWOPLAYER    "TwoPlayer"


bool EnsureSavesDirectoryExists(GameModeType gameMode); // Adicionado gameMode
bool DoesSaveSlotExist(int slot_number, GameModeType gameMode); // Adicionado gameMode
void GetSaveFileName(char *buffer, int buffer_size, int slot_number, GameModeType gameMode); // Adicionado gameMode

bool SaveGame(Player players[], int num_players_to_save, int slot_number, int currentMapX, int currentMapY, GameModeType gameMode); // Adicionado num_players_to_save e gameMode
bool LoadGame(Player players[], int max_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY, GameModeType gameMode, int *numPlayersLoaded); // Adicionado gameMode e numPlayersLoaded

#endif // SAVELOAD_H