#ifndef LOAD_SAVE_UI_H
#define LOAD_SAVE_UI_H

#include "raylib.h"
#include "Game.h"   // Para GameState, Player, Music
#include "WorldLoading.h" // Para WorldSection (necessário para o ponteiro de ponteiro)

// Assinatura corrigida para bool* e adicionado WorldSection**
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[],
                              int currentMusicIndex, float currentVolume, bool *musicIsCurrentlyPlaying_ptr,
                              int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos,
                              WorldSection** currentActiveWorldSection_ptr); // Ponteiro para ponteiro para atualizar a seção ativa


void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, bool musicIsPlaying,
                            float musicVolume, int mapX, int mapY);

#endif // LOAD_SAVE_UI_H