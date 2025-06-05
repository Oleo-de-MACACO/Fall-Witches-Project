#ifndef CHARACTER_CREATION_H
#define CHARACTER_CREATION_H

#include "raylib.h"
#include "Game.h"
#include <stdbool.h>

void InitializeCharacterCreation(void);
void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY,
                                   Music playlist[], int currentMusicIndex, float currentVolume, bool *musicIsPlaying_ptr); // bool*
void DrawCharacterCreationScreen(Player players[]);

#endif // CHARACTER_CREATION_H