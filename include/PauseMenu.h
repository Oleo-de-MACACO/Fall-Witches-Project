#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "raylib.h"
#include "Game.h"   // *** ADICIONADO: Necessário para GameState, Player, Music ***

void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[],
                       int currentMusicIndex, bool isPlaying_beforePause, bool *musicIsCurrentlyPlaying_ptr,
                       Vector2 virtualMousePos);
void DrawPauseMenuElements(void);
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, bool isPlaying_when_game_paused, int currentMapX, int currentMapY);

#endif // PAUSE_MENU_H