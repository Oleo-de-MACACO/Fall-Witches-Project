#ifndef SETTINGS_H
#define SETTINGS_H

#include "Game.h" // *** ADICIONADO: Necessário para GameState ***

void InitializeSettingsScreen(GameState previousScreen);
void UpdateSettingsScreen(GameState *currentScreen_ptr);
void DrawSettingsScreen(void);

#endif // SETTINGS_H