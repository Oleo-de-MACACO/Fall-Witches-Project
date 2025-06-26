#ifndef LOAD_SAVE_UI_H
#define LOAD_SAVE_UI_H

#include "raylib.h"
#include "Game.h"   // *** ADICIONADO: Necessário para GameState, Player, Music ***
// WorldLoading.h já é incluído por Game.h

void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[],
                              int currentMusicIndex, float currentVolume, bool *musicIsCurrentlyPlaying_ptr,
                              int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos,
                              WorldSection** currentActiveWorldSection_ptr);
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, bool musicIsPlaying,
                            float musicVolume, int mapX, int mapY);

#endif // LOAD_SAVE_UI_H