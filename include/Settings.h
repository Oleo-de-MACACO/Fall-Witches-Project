#ifndef SETTINGS_H
#define SETTINGS_H

#include "raylib.h"
#include "Game.h" // Para GameState

// Nenhuma mudança de protótipo necessária aqui ainda,
// pois Settings.c irá modificar diretamente as variáveis globais de volume
// definidas em main.c. A UI será adicionada em Settings.c.

void InitializeSettingsScreen(GameState previousScreen);
void UpdateSettingsScreen(GameState *currentScreen_ptr); // Irá ler input para mudar os volumes globais
void DrawSettingsScreen(void);                           // Irá desenhar sliders para os novos volumes

#endif // SETTINGS_H