#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "raylib.h"
#include "Game.h" // *** ESSENCIAL: Para Player, GameModeType, BorderDirection ***

bool WorldMap_CheckTransition(Player players[], int numActivePlayers, int *currentMapX_ptr, int *currentMapY_ptr, GameModeType gameMode, float sectionActualWidth, float sectionActualHeight);

#endif // WORLD_MAP_H